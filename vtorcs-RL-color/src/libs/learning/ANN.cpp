/* -*- Mode: C++; -*- */
/* VER: $Id: ANN.cpp,v 1.4.2.1 2008/05/30 22:18:32 berniw Exp $ */
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <learning/ANN.h>
#include <learning/string_utils.h>
#include <learning/Distribution.h>


#undef ANN_DBUG

//==========================================================
// NewANN
//----------------------------------------------------------
/// Create a new ANN
ANN *NewANN(int n_inputs, int n_outputs)
{
	ANN *ann = NULL;

	if (!(ann = AllocM(ANN, 1))) {
		Serror("Could not allocate ANN\n");
		return NULL;
	}
	ann->x = NULL;
	ann->y = NULL;
	ann->t = NULL;
	ann->d = NULL;
	ann->error = NULL;
	ann->c = NULL;
	ann->a = 0.1f;
	ann->lambda = 0.9f;
	ann->zeta = 0.9f;
	ann->n_inputs = n_inputs;
	ann->n_outputs = n_outputs;
	ann->batch_mode = false;


	/* outputs are not allocated */
	//logmsg ("Creating ANN with %d inputs and %d outputs\n", n_inputs, n_outputs);
	if (!(ann->error = AllocM(real, n_outputs))) {
		Serror("Could not allocate errors\n");
		DeleteANN(ann);
		return NULL;
	}

	if (!(ann->d = AllocM(real, n_outputs))) {
		Serror("Could not allocate derivatives\n");
		DeleteANN(ann);
		return NULL;
	}

	if (!(ann->c = List())) {
		Serror("Could not allocate list\n");
		DeleteANN(ann);
		return NULL;
	}
#ifdef ANN_DBUG
	message("Creating ANN with %d inputs and %d outputs", n_inputs,
		n_outputs);
#endif
	return ann;
}

//==========================================================
// DeleteANN
//----------------------------------------------------------
/// Delete a neural network
int DeleteANN(ANN * ann)
{
	if (!ann) {
		Swarning("Attempting to delete NULL ANN\n");
		return DEC_ARG_INVALID;
	}

	if (ann->error) {
		FreeM(ann->error);
	}
	
	//if (ann->x) {
	//	FreeM (ann->x);
	//}

	if (ann->d) {
		FreeM(ann->d);
	}

	/* We must clear all allocations in the list */
	if (ann->c) {
		ClearList(ann->c);
		ann->c = NULL;
	}

	FreeM(ann);
	return 0;
}


//==========================================================
// ANN_AddHiddenLayer()
//----------------------------------------------------------
/// Add a hidden layer with n_nodes
int ANN_AddHiddenLayer(ANN * ann, int n_nodes)
{
#ifdef ANN_DBUG
	message("Adding Hidden layer with %d nodes", n_nodes);
#endif

	LISTITEM *item = LastListItem(ann->c);
	if (item) {
		Layer *p = (Layer *) item->obj;
		ANN_AddLayer(ann, p->n_outputs, n_nodes, p->y);
	} else {
		ANN_AddLayer(ann, ann->n_inputs, n_nodes, ann->x);
	}
	return 0;
}

//==========================================================
// ANN_AddRBFHiddenLayer()
//----------------------------------------------------------
/// Add an RBF layer with n_nodes
int ANN_AddRBFHiddenLayer(ANN * ann, int n_nodes)
{
#ifdef ANN_DBUG
	message("Adding Hidden layer with %d nodes", n_nodes);
#endif
	LISTITEM *item = LastListItem(ann->c);
	if (item) {
		Layer *p = (Layer *) item->obj;
		ANN_AddRBFLayer(ann, p->n_outputs, n_nodes, p->y);
	} else {
		ANN_AddRBFLayer(ann, ann->n_inputs, n_nodes, ann->x);
	}
	return 0;
}



//==========================================================
// ANN_AddLayer()
//----------------------------------------------------------
/// Low-level code to add a weighted sum layer
Layer *ANN_AddLayer(ANN * ann, int n_inputs, int n_outputs, real * x)
{
	Layer *l = NULL;
	if ((x == NULL) && (ann->c->n)) {
		Swarning
		    ("Layer connects to null but layer list is not empty\n");
	}


	if (!(l = AllocM(Layer, 1))) {
		Serror("Could not allocate layer structure\n");
		return NULL;
	}

	assert(n_inputs > 0);
	assert(n_outputs > 0);

	l->n_inputs = n_inputs;
	l->n_outputs = n_outputs;
	l->x = x;
	l->a = ann->a;
	l->zeta = ann->zeta;
	l->lambda = ann->lambda;
	l->forward = &ANN_CalculateLayerOutputs;
	l->backward = &ANN_Backpropagate;
	l->f = &htan;
	l->f_d = &htan_d;
	//l->f = &dtan;
	//	l->f_d = &dtan_d;
	l->batch_mode = false;
	if (!(l->y = AllocM(real, n_outputs))) {
		Serror("Could not allocate layer outputs\n");
		ANN_FreeLayer(l);
		return NULL;
	}
	int i;
	for (i=0; i<n_outputs; i++) {
		l->y[i] = 0.0;
	}

	if (!(l->z = AllocM(real, n_outputs))) {
		Serror("Could not allocate layer activations\n");
		ANN_FreeLayer(l);
		return NULL;
	}
	for (i=0; i<n_outputs; i++) {
		l->z[i] = 0.0;
	}
	if (!(l->d = AllocM(real, n_inputs + 1 /*bias */ ))) {
		Serror("Could not allocate layer outputs\n");
		ANN_FreeLayer(l);
		return NULL;
	}
	for (i=0; i<n_inputs+1; i++) {
		l->d[i] = 0.0;
	}

	if (!
	    (l->c =
	     AllocM(Connection, (n_inputs + 1 /*bias */ ) * n_outputs))) {
		Serror("Could not allocate connections\n");
		ANN_FreeLayer(l);
		return NULL;
	}

	l->rbf = NULL;

	real bound = 2.0f / sqrt((real) n_inputs);
	for (i = 0; i < n_inputs + 1 /*bias */ ; i++) {
		Connection *c = &l->c[i * n_outputs];
		for (int j = 0; j < n_outputs; j++) {
			c->w = (urandom() - 0.5f)* bound;;
			c->c = 1;
			c->e = 0.0f;
			c->dw = 0.0f;
			c->v = 1.0;
			c++;
		}
	}
	ListAppend(ann->c, (void *) l, &ANN_FreeLayer);
	return l;
}


//==========================================================
// ANN_AddRBFLayer()
//----------------------------------------------------------
/// Low-level code to add an RBF layer
Layer *ANN_AddRBFLayer(ANN * ann, int n_inputs, int n_outputs, real * x)
{
	Layer *l = NULL;
	if ((x == NULL) && (ann->c->n)) {
		Swarning
		    ("Layer connects to null and layer list not empty\n");
	}

	if (!(l = AllocM(Layer, 1))) {
		Serror("Could not allocate layer structure\n");
		return NULL;
	}

	assert(n_inputs > 0);
	assert(n_outputs > 0);

	l->n_inputs = n_inputs;
	l->n_outputs = n_outputs;
	l->x = x;
	l->a = ann->a;
	l->forward = &ANN_RBFCalculateLayerOutputs;
	l->backward = &ANN_RBFBackpropagate;
	l->f = &Exp;
	l->f_d = &Exp_d;
	l->batch_mode = false;

	if (!(l->y = AllocM(real, n_outputs))) {
		Serror("Could not allocate layer outputs\n");
		ANN_FreeLayer(l);
		return NULL;
	}

	if (!(l->z = AllocM(real, n_outputs))) {
		Serror("Could not allocate layer activations\n");
		ANN_FreeLayer(l);
		return NULL;
	}

	if (!(l->d = AllocM(real, n_inputs + 1 /*bias */ ))) {
		Serror("Could not allocate layer outputs\n");
		ANN_FreeLayer(l);
		return NULL;
	}

	if (!
	    (l->rbf =
	     AllocM(RBFConnection,
		    (n_inputs + 1 /*bias */ ) * n_outputs))) {
		Serror("Could not allocate connections\n");
		ANN_FreeLayer(l);
		return NULL;
	}

	l->c = NULL;

	real bound = 2.0f / sqrt((real) n_inputs);
	for (int i = 0; i < n_inputs + 1 /*bias */ ; i++) {
		RBFConnection *c = &l->rbf[i * n_outputs];
		for (int j = 0; j < n_outputs; j++) {
			c->w = (urandom() - 0.5f) * bound;;
			c->m = (urandom() - 0.5f) * 2.0f;
			c++;
		}
	}
	ListAppend(ann->c, (void *) l, &ANN_FreeLayer);
	return l;
}


//==========================================================
// ANN_FreeLayer()
//----------------------------------------------------------
/// Free this layer - low level
void ANN_FreeLayer(void *l)
{
	ANN_FreeLayer((Layer *) l);
}

//==========================================================
// ANN_FreeLayer()
//----------------------------------------------------------
/// Free this layer - low level
void ANN_FreeLayer(Layer * l)
{
	FreeM(l->y);
	if (l->z) {
		FreeM(l->z);
	}
	if (l->c) {
		FreeM(l->c);
	}
	if (l->rbf) {
		FreeM(l->rbf);
	}
	FreeM(l->d);
	FreeM(l);

}

//==========================================================
// ANN_Init()
//----------------------------------------------------------
/// Initialise neural network.

/// Call this function after you have added all the layers.
/// It adds an extra output layer.
int ANN_Init(ANN * ann)
{
	// Add output layer
	LISTITEM *item = LastListItem(ann->c);
	Layer *l = NULL;
#ifdef ANN_DBUG
	message("Initialising");
#endif
	if (item) {
		Layer *p = (Layer *) item->obj;
		l = ANN_AddLayer(ann, p->n_outputs, ann->n_outputs, p->y);
	} else {
		l = ANN_AddLayer(ann, ann->n_inputs, ann->n_outputs,
				 ann->x);
	}
	if (l == NULL) {
		Serror("Could not create final layer\n");
		DeleteANN(ann);
		return -1;
	}
	ann->y = l->y;
	l->f = &linear;
	l->f_d = &linear_d;
	//  ann->t = l->t;
	return 0;
}



//==========================================================
// ANN_Reset()
//----------------------------------------------------------
/// Resets the eligbility traces and batch updates.
void ANN_Reset(ANN * ann)
{
	LISTITEM *p = FirstListItem(ann->c);

	while (p) {
		Layer *l = (Layer *) p->obj;
		for (int i = 0; i < l->n_inputs + 1 /* bias */; i++) {
			Connection *c = &l->c[i * l->n_outputs];
			for (int j = 0; j < l->n_outputs; j++) {
				c->e = 0.0;
				c->dw = 0.0;
				c++;
			}
		}
		p = NextListItem (ann->c);
	}
}


//==========================================================
// ANN_Input()
//----------------------------------------------------------
/// Give an input vector to the neural network.

/// Calculate a new output given the input.
/// If the vector length is incorrect, you will have
/// trouble.
real ANN_Input(ANN * ann, real * x)
{
	LISTITEM *p = FirstListItem(ann->c);
	Layer *first_layer = (Layer *) p->obj;
	ann->x = x;
	first_layer->x = x;	// Setup input of first layer
	//  printf ("II: %f\n", ann->x[0]);
	while (p) {
		Layer *current_layer = (Layer *) p->obj;
		//    printf ("\tIII: %f\n", current_layer->x[0]);
		current_layer->forward(current_layer, false);
		p = NextListItem(ann->c);
	}
	return 0.0f;
}

//==========================================================
// ANN_StochasticInput()
//----------------------------------------------------------
/// Stochastically generate an output, depending on parameter
/// distributions. 
/// This is an option for people that understand what they
/// are doing.
real ANN_StochasticInput(ANN * ann, real * x)
{
	LISTITEM *p = FirstListItem(ann->c);
	Layer *first_layer = (Layer *) p->obj;
	ann->x = x;
	first_layer->x = x;	// Setup input of first layer
	//  printf ("II: %f\n", ann->x[0]);
	while (p) {
		Layer *current_layer = (Layer *) p->obj;
		//    printf ("\tIII: %f\n", current_layer->x[0]);
		current_layer->forward(current_layer, true);
		p = NextListItem(ann->c);
	}
	return 0.0f;
}

//==========================================================
// ANN_CalculateLayerOutputs()
//----------------------------------------------------------
/// Calculate layer outputs
void ANN_CalculateLayerOutputs(Layer * current_layer, bool stochastic)
{
	int i, j;
	int n_inputs = current_layer->n_inputs;
	int n_outputs = current_layer->n_outputs;
	real *x = current_layer->x;
	real *y = current_layer->y;
	real *z = current_layer->z;
	Connection *c;

	for (j = 0; j < n_outputs; j++) {
		z[j] = 0.0f;
	}
	c = current_layer->c;
	if (stochastic) {
		for (i = 0; i < n_inputs; i++) {
			for (j = 0; j < n_outputs; j++) {
				// using uniform bounded.. 
				real w = c->w + (urandom()-0.5f)*c->v ;
				z[j] += x[i] * w;
				c++;
			}
		}
		
		// bias
		for (j = 0; j < n_outputs; j++) {
			real w = c->w + (urandom()-0.5f)*c->v ;
			z[j] += w;
			c++;
		}
	} else {
		for (i = 0; i < n_inputs; i++) {
			for (j = 0; j < n_outputs; j++) {
				z[j] += x[i] * c->w;
				c++;
			}
		}
		
		// bias
		for (j = 0; j < n_outputs; j++) {
			z[j] += c->w;
			c++;
		}
	}
	
	for (j = 0; j < n_outputs; j++) {
		y[j] = current_layer->f(z[j]);
	}
}

//==========================================================
// ANN_RBFCalculateLayerOutputs()
//----------------------------------------------------------
/// Calculate layer outputs
void ANN_RBFCalculateLayerOutputs(Layer * current_layer, bool stochastic)
{
	int i, j;
	int n_inputs = current_layer->n_inputs;
	int n_outputs = current_layer->n_outputs;
	real *x = current_layer->x;
	real *y = current_layer->y;
	real *z = current_layer->z;
	RBFConnection *c;


	for (j = 0; j < n_outputs; j++) {
		z[j] = 0.0f;
	}

	c = current_layer->rbf;
	for (i = 0; i < n_inputs; i++) {
		real in = x[i];
		for (j = 0; j < n_outputs; j++, c++) {
			real o = (in - c->m) * c->w;
			z[j] += o * o;
		}
	}

	for (j = 0; j < n_outputs; j++) {
		z[j] = -0.5f * z[j];
		y[j] = current_layer->f(z[j]);
	}
}


//==========================================================
// ANN_Train()                           simple MSE training 
//----------------------------------------------------------

/// Perform mean square error training, where the aim is to minimise
/// the cost function \f$\sum_i |f(x_i)-t_i|^2\f$, where \f$x_i\f$ is
/// input data, \f$f(\cdot)\f$ is the mapping performed by the neural
/// network, \f$t_i\f$ is the desired output and \f$i\f$ denotes the example
/// index. Under mild assumptions, this is equivalent to minimising
/// \f$E\{|f(X)-T|^2\}\f$, the expected value of the squared error.
real ANN_Train(ANN * ann, real * x, real * t)
{
	LISTITEM *p = LastListItem(ann->c);
	Layer *l = (Layer *) p->obj;
	real sum = 0.0f;
	int j;

	ANN_Input(ann, x);

	for (j = 0; j < ann->n_outputs; j++) {
		real f = l->f_d(ann->y[j]);
		real e = t[j] - ann->y[j];
		ann->error[j] = e;
		ann->d[j] = e * f;
		sum += e * e;
	}

	l->backward(p, ann->d, ann->eligibility_traces, 0.0);

	return sum;
}


//==========================================================
// ANN_Delta_Train()                  Train with custom cost
//----------------------------------------------------------
/// \brief Minimise a custom cost function.

/// The argument delta is a the derivative of the cost function with
/// respect to the neural network outputs. In this case you must call
/// ANN_Input() yourself. As an example, to use MSE cost with
/// ANN_Delta_Train(), you should call ANN_Input() with your data,
/// call ANN_GetOutput() to take a look at the outputs and then call
/// ANN_DeltaTrain() with a real vector delta equal to the target
/// vector minus the ANN output vector. The argument TD is useful if
/// you want to separate the calculation of the derivative of the
/// output with respect to the parameters from the derivative the cost
/// function with respect to the output. This is done in reinforcement
/// learning with eligibility traces for example, where the cost
/// function depends upon previous outputs.
real ANN_Delta_Train(ANN * ann, real* delta, real TD)
{
	LISTITEM *p = LastListItem(ann->c);
	Layer *l = (Layer *) p->obj;
	real sum = 0.0f;
	int j;
	//ANN_Input(ann, x);
	for (j = 0; j < ann->n_outputs; j++) {
		real f = l->f_d(ann->y[j]);
		real e = delta[j];
		ann->error[j] = e;
		ann->d[j] = e * f;
		sum += e * e;
	}

	l->backward(p, ann->d, ann->eligibility_traces, TD);

	return sum;
}



//==========================================================
// ANN_Backpropagate
//----------------------------------------------------------
/// d are the derivatives at the outputs.
real ANN_Backpropagate(LISTITEM * p, real * d, bool use_eligibility, real TD)
{
	int i, j;
	real f;
	real a;
	Layer *l = (Layer *) p->obj;
	LISTITEM *back = p->prev;
	Layer *back_layer = NULL;
	a = l->a;
	//  message ("backing with in: %d",l->x);
	if (back) {
		//message ("and to prev");
		back_layer = (Layer *) back->obj;
		for (i = 0; i < l->n_inputs; i++) {
			real der = 0.0f;

			Connection *c = &l->c[i * l->n_outputs];
			for (j = 0; j < l->n_outputs; j++) {
				der += c->w * d[j];
				c++;
			}
			f = back_layer->f_d(l->x[i]);
			der*=f;
			l->d[i] = der;
		}

		/* bias */
		i = l->n_inputs;
		l->d[i] = 0.0f;
		Connection *c = &l->c[i * l->n_outputs];
		for (j = 0; j < l->n_outputs; j++) {
			l->d[i] += c->w * d[j];
			c++;
		}
		f = back_layer->f_d(1.0f);
		l->d[i] = l->d[i] * f;

		back_layer->backward(back, l->d, use_eligibility, TD);
	}
	//update weights
	for (i = 0; i < l->n_inputs; i++) {
		Connection *c = &l->c[i * l->n_outputs];
		real dx = a * l->x[i];
		if (l->batch_mode) {
			for (j = 0; j < l->n_outputs; j++) {
				real delta;
				if (use_eligibility) {
					c->e = c->e * l->lambda + l->x[i]* d[j];
					delta = a * c->e * TD; //better?
					c->v += (1.0f - l->zeta)*c->v+(l->zeta)*delta*delta;
				} else {
					delta = dx * d[j];
				}
				c->dw += delta;
				c->v = (1.0f - l->zeta)*c->v + (l->zeta)*fabs(delta);
				if (c->v < 0.01f) c->v = 0.01f;
				c++;
			}
		} else {
			for (j = 0; j < l->n_outputs; j++) {
				real delta;
				if (use_eligibility) {
					c->e = c->e * l->lambda + l->x[i] * d[j];
					delta = a * c->e * TD;
				} else {
					delta = dx * d[j];
				}
				c->w += delta;
				delta /= a;
				c->v = (1.0f - l->zeta)*c->v + (l->zeta)*fabs(delta);
				//printf("%f\n", c->v);
				if (c->v < 0.01f) c->v = 0.01f;
				c++;
			}
		}
	}
	// update bias weight
	{
		Connection *c = &l->c[l->n_inputs * l->n_outputs];
		if (l->batch_mode) {
			for (j = 0; j < l->n_outputs; j++) {
				real delta;
				if (use_eligibility) {
					c->e = c->e * l->lambda + d[j];
					delta = a * c->e * TD;
				} else {
					delta = a * d[j];
				}
				c->dw += delta;
				c->v = (1.0 - l->zeta)*c->v + (l->zeta)*fabs(delta);
				if (c->v < 0.01f) c->v = 0.01f;
				c++;
			}
		} else {
			for (j = 0; j < l->n_outputs; j++) {
				real delta;
				if (use_eligibility) {
					c->e = c->e * l->lambda + d[j];
					delta = a * c->e * TD; //better?
				} else {
					delta = a * d[j];
				}
				c->w += delta;
				c->v = (1.0f - l->zeta)*c->v + (l->zeta)*fabs(delta);
				if (c->v < 0.01f) c->v = 0.01f;
				c++;
			}
		}
	}
	return 0.0f;
}

//==========================================================
// ANN_RBFBackpropagate
//----------------------------------------------------------
/// Backpropagation for an RBF layer.
/// \bug Currently RBF layers lack the functionality of sigmoid ones.
real ANN_RBFBackpropagate(LISTITEM * p, real * d, bool use_eligibility, real TD)
{
	int i, j;
	real f;
	real a;
	Layer *l = (Layer *) p->obj;
	LISTITEM *back = p->prev;
	Layer *back_layer = NULL;
	a = l->a;

	if (back) {
		back_layer = (Layer *) back->obj;
		for (i = 0; i < l->n_inputs; i++) {
			l->d[i] = 0.0f;
			RBFConnection *c = &l->rbf[i * l->n_outputs];
			for (j = 0; j < l->n_outputs; j++) {
				real dx = l->x[i] - c->m;
				real dm = d[j] * dx * c->w * c->w;
				l->d[j] -= dm;
				c++;
			}
			f = back_layer->f_d(l->x[i]);
			l->d[i] = l->d[i] * f;
		}
		back_layer->backward(back, l->d, use_eligibility, TD);
	}

	return 0.0f;
	//update weights
	for (i = 0; i < l->n_inputs; i++) {
		RBFConnection *c = &l->rbf[i * l->n_outputs];
		real dx = l->x[i] - c->m;
		for (j = 0; j < l->n_outputs; j++) {
			real dy = d[j];
			real dx2 = a * dy * dx * c->w;
			real dm = dx2 * c->w;
			real dw = dx2 * dx;
			c->m += dm;
			c->w += dw;
			c++;
		}
	}
	return 0.0f;
}

/// Perform batch adaptation
void ANN_LayerBatchAdapt(Layer * l)
{
	int i, j;

	if (l->batch_mode == false) {
		Serror("Batch adapt yet not in batch mode!");
	}
	//update weights
	for (i = 0; i < l->n_inputs; i++) {
		Connection *c = &l->c[i * l->n_outputs];
		for (j = 0; j < l->n_outputs; j++) {
			c->w += c->dw;
			c++;
		}
	}
	// update bias weight
	{
		Connection *c = &l->c[l->n_inputs * l->n_outputs];
		for (j = 0; j < l->n_outputs; j++) {
			c->w += c->dw;
			c++;
		}
	}
}

//==========================================================
// ANN_Test()
//----------------------------------------------------------
/// Given an input and test pattern, return the MSE between the network's output and the test pattern.
real ANN_Test(ANN * ann, real * x, real * t)
{
	//LISTITEM *p = LastListItem(ann->c);
	//Layer *l = (Layer *) p->obj;
	real sum = 0.0f;
	int j;
	ANN_Input(ann, x);

	for (j = 0; j < ann->n_outputs; j++) {
		//real f = l->f_d(ann->y[j]);
		real e = t[j] - ann->y[j];
		ann->error[j] = e;
		ann->d[j] =0.0;// e * f;
		sum += e * e;
	}
	return sum;
}

//==========================================================
// ANN_GetOutput()
//----------------------------------------------------------
/// Get the output for the current input.
real *ANN_GetOutput(ANN * ann)
{
	return ann->y;
}

//==========================================================
// ANN_GetError()
//----------------------------------------------------------
/// Get the error for the current input/output pair.
real ANN_GetError(ANN * ann)
{
    real sum = 0.0;
    for (int i=0; i<ann->n_outputs; i++) {
	real e = ann->error[i];
	sum += e*e;
    }
    return (real) sqrt(sum);
}

//==========================================================
// ANN_GetErrorVector()
//----------------------------------------------------------
/// Return the error vector for pattern.
real* ANN_GetErrorVector(ANN * ann)
{
    return ann->error;
}

//==========================================================
// ANN_SetLearningRate()
//----------------------------------------------------------
/// Set the learning rate to a
void ANN_SetLearningRate(ANN * ann, real a)
{
	LISTITEM *c;

	ann->a = a;
	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		l->a = a;
		c = NextListItem(ann->c);
	}
}
//==========================================================
// ANN_SetLambda()
//----------------------------------------------------------
/// Set lambda, eligibility decay.
void ANN_SetLambda(ANN * ann, real lambda)
{
	LISTITEM *c;

	ann->lambda = lambda;
	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		l->lambda = lambda;
		c = NextListItem(ann->c);
	}
}

//==========================================================
// ANN_SetZeta()
//----------------------------------------------------------
/// Set zeta, parameter variance smoothing. Useful for
/// ANN_StochasticInput()
void ANN_SetZeta(ANN * ann, real zeta)
{
	LISTITEM *c;

	ann->zeta = zeta;
	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		l->zeta = zeta;
		c = NextListItem(ann->c);
	}
}
//==========================================================
// ANN_SetBatchMode
//----------------------------------------------------------
/// Set batch updates.
void ANN_SetBatchMode(ANN * ann, bool batch)
{
	LISTITEM *c;

	ann->batch_mode = batch;
	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		l->batch_mode = batch;
		c = NextListItem(ann->c);
	}
}

//==========================================================
// ANN_BatchAdapt
//----------------------------------------------------------
/// Adapt the parameters after a series of patterns has been seen.
void ANN_BatchAdapt(ANN * ann)
{
	LISTITEM *c;

	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		ANN_LayerBatchAdapt(l);
		c = NextListItem(ann->c);
	}
}



//==========================================================
// ANN_ShowWeights()
//----------------------------------------------------------
/// Dump the weights on stdout.
real ANN_ShowWeights(ANN * ann)
{
	LISTITEM *c;
	real sum = 0.0f;
	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		sum += ANN_LayerShowWeights(l);
		c = NextListItem(ann->c);
	}
	return sum;
}

//==========================================================
// ANN_LayerShowWeights()
//----------------------------------------------------------
/// Dump the weights of a particular layer on stdout.
real ANN_LayerShowWeights(Layer * l)
{
	int i, j;
	Connection *c = l->c;
	real sum = 0.0f;
	for (i = 0; i < l->n_inputs + 1 /*bias */ ; i++) {
		for (j = 0; j < l->n_outputs; j++) {
			sum += (c->w) * (c->w);	//printf ("%f ", c->w);
			printf("%f ", c->w);
			c++;
		}
	}
	//  printf ("%f ", sum);
	return sum;
}


//==========================================================
// ANN_ShowInputs()
//----------------------------------------------------------
/// Dump inputs to all layers on stdout.
real ANN_ShowInputs(ANN * ann)
{
	LISTITEM *c;
	real sum = 0.0f;
	c = FirstListItem(ann->c);
	while (c) {
		Layer *l = (Layer *) c->obj;
		sum += ANN_LayerShowInputs(l);
		c = NextListItem(ann->c);
	}
	return sum;
}

//==========================================================
// ANN_LayerShowInputs()
//----------------------------------------------------------
/// Dump inputs to a particular layer on stdout.
real ANN_LayerShowInputs(Layer * l)
{
	int i;
	real sum = 0.0f;
	for (i = 0; i < l->n_inputs; i++) {
		printf("#%f ", l->x[i]);
	}
	printf("-->");
	for (i = 0; i < l->n_outputs; i++) {
		printf("#(%f)%f ", l->f(l->z[i]), l->y[i]);
	}

	printf("\n");
	return sum;
}


//==========================================================
// ANN_ShowOutputs()
//----------------------------------------------------------
/// Dump outputs to stdout
void ANN_ShowOutputs(ANN * ann)
{
	int i;

	for (i = 0; i < ann->n_outputs; i++) {
		printf("%f ", ann->y[i]);
	}
	printf("\n");
}



//==========================================================
// ANN_SetOutputsToLinear()
//----------------------------------------------------------
/// Set outputs to linear
void ANN_SetOutputsToLinear(ANN * ann)
{
	LISTITEM *c;
	c = LastListItem(ann->c);
	if (c) {
		Layer *l = (Layer *) c->obj;
		l->f = &linear;
		l->f_d = &linear_d;
	} else {
		Serror("Could not set outputs to linear\n");
	}
}

//==========================================================
// ANN_SetOutputsToTanH()
//----------------------------------------------------------
/// Set outputs to hyperbolic tangent.
void ANN_SetOutputsToTanH(ANN * ann)
{
	LISTITEM *c;
	c = LastListItem(ann->c);
	if (c) {
		Layer *l = (Layer *) c->obj;
		l->f = &htan;
		l->f_d = &htan_d;
	} else {
		Serror("Could not set outputs to TanH\n");
	}
}


//==========================================================
// Exp()
//----------------------------------------------------------
/// Exponential hook
real Exp(real x)
{
	return (real) exp(x);
}

//==========================================================
// Exp_d()
//----------------------------------------------------------
/// Exponential derivative hook
real Exp_d(real x)
{
	return x;
}

//==========================================================
// htan()
//----------------------------------------------------------
/// Hyperbolic tangent hook
real htan(real x)
{
	return (real) tanh(x);
}

//==========================================================
// htan_d()
//----------------------------------------------------------
/// Hyperbolic tangent derivative hook
real htan_d(real x)
{
	real f = (real) tanh(x);
	return (real) (1.0 - f * f);
}

//==========================================================
// dtan()
//----------------------------------------------------------
/// Discrete htan hook
real dtan(real x)
{
	if (x>1.0) {
		return 1.0;
	} else if (x<1.0) {
		return -1.0;
	}
	return x;
}

//==========================================================
// dtan_d()
//----------------------------------------------------------
/// Discrete htan derivative hook
real dtan_d(real x)
{
	if (x>1.0) {
		return 0.0;
	} else if (x<-1.0) {
		return 0.0;
	}
	return 1.0;
}

//==========================================================
// linear()
//----------------------------------------------------------
/// linear hook
real linear(real x)
{
	return x;
}

//==========================================================
// linear_d()
//----------------------------------------------------------
/// linear derivative hook
real linear_d(real x)
{
	return 1.0f;
}

/// Check that tags match
static inline bool CheckMatchingToken (const char* tag, StringBuffer* buf, FILE* f)
{
	int l = 1+strlen(tag);
	buf = SetStringBufferLength (buf, l);
	if (buf==NULL) {
		return false;
	}
	fread(buf->c, sizeof(char), l, f);

	if (strcmp(tag,buf->c)) {
		fprintf (stderr, "Expected tag <%s>, found <%s>.\n", tag, buf->c);
		return false;
	}
	return true;
}

/// Write a token
static inline void WriteToken (const char* tag, FILE* f)
{
	fwrite (tag, sizeof(char), 1+strlen(tag), f);
}

/// Load an ANN from a filename.
ANN* LoadANN(char* filename)
{
	FILE* f = fopen (filename, "rb");
	if (f) {
		ANN* ann = LoadANN (f);
		fclose (f);
		return ann;
	}
	return NULL;
}
/// Save the ANN to a filename.
int SaveANN(ANN* ann, char* filename)
{
	FILE* f = fopen (filename, "wb");
	if (f) {
		int r = SaveANN (ann, f);
		fclose (f);
		return r;
	}
	return -1;
}

/// Load the ANN from a C file handle.
ANN* LoadANN(FILE* f)
{
	if (f==NULL) {
		return NULL;
	}
	StringBuffer* rtag = NewStringBuffer (256);
	CheckMatchingToken("VSOUND_ANN", rtag, f);
	int n_inputs;
	int n_outputs;
	fread(&n_inputs, sizeof(int), 1, f);
	fread(&n_outputs, sizeof(int), 1, f);
	ANN* ann = NewANN (n_inputs, n_outputs);
	CheckMatchingToken("Layer Data", rtag, f);
	int n_layers;
	fread(&n_layers, sizeof(int), 1, f);
	for (int i=0; i<n_layers-1; i++) {
		int layer_type;
		CheckMatchingToken("TYPE", rtag, f);
		fread(&layer_type, sizeof(int), 1, f);
		int nhu;
		CheckMatchingToken("UNITS", rtag, f);
		fread(&nhu, sizeof(int), 1, f);
		if (layer_type==0) {
			ANN_AddHiddenLayer(ann, nhu);
		} else {
			ANN_AddRBFHiddenLayer(ann, nhu);
		}
	}
	{
		int layer_type =0;
		ANN_Init(ann);
		CheckMatchingToken("Output Type", rtag, f);
		fread(&layer_type, sizeof(int), 1, f);
		if (layer_type==0) {
			ANN_SetOutputsToLinear(ann);	
		} else {
			ANN_SetOutputsToTanH(ann);
		}
	}

	LISTITEM* list_item = FirstListItem(ann->c);
	while (list_item) {
		Layer* l = (Layer*) list_item->obj;
		CheckMatchingToken("Connections", rtag, f);
		int size = (l->n_inputs + 1 /*bias*/) * l->n_outputs;
		fread(l->c, size, sizeof(Connection), f);
		list_item = NextListItem (ann->c);
	}
	CheckMatchingToken("END", rtag, f);

	FreeStringBuffer (&rtag);
	return ann;
}

/// Save the ANN to a C file handle.
int SaveANN(ANN* ann, FILE* f)
{
	if (f==NULL) {
		return -1;
	}
	
	StringBuffer* rtag = NewStringBuffer (256);

	WriteToken("VSOUND_ANN", f);
	fwrite(&ann->n_inputs, sizeof(int), 1, f);
	fwrite(&ann->n_outputs, sizeof(int), 1, f);
	WriteToken("Layer Data", f);
	int n_layers = 0;
	LISTITEM* list_item = FirstListItem(ann->c);
	while (list_item) {
		n_layers++;
		list_item = NextListItem (ann->c);
	}
	fwrite(&n_layers, sizeof(int), 1, f);
	list_item = FirstListItem(ann->c);
	for (int i=0; i<n_layers-1; i++) {
		Layer* l = (Layer*) list_item->obj;

		int layer_type = 0;
		WriteToken("TYPE", f);
		fwrite(&layer_type, sizeof(int), 1, f);

		int nhu = l->n_outputs;
		WriteToken("UNITS", f);
		fwrite(&nhu, sizeof(int), 1, f);
		list_item = NextListItem (ann->c);
	}
	WriteToken("Output Type", f);
	{
		int layer_type = 0;
		LISTITEM *c;
		c = LastListItem(ann->c);
		if (c) {
			Layer *l = (Layer *) c->obj;
			if (l->f==&linear) {
				layer_type = 0;
			} else {
				layer_type = 1;
			}
		}
		fwrite(&layer_type, sizeof(int), 1, f);
	}
	list_item = FirstListItem(ann->c); 
	while(list_item) {
		Layer* l = (Layer*) list_item->obj;
		WriteToken("Connections", f);
		int size = (l->n_inputs + 1 /*bias*/) * l->n_outputs;
		fwrite(l->c, size, sizeof(Connection), f);
		list_item = NextListItem(ann->c);
	}
	WriteToken("END", f);

	FreeStringBuffer (&rtag);
	return 0;
}
