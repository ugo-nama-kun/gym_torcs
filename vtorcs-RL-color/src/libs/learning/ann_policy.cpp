// -*- Mode: c++ -*-
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
// $Id: ann_policy.cpp,v 1.3 2005/08/05 09:02:58 berniw Exp $


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <learning/ann_policy.h>

ANN_Policy::ANN_Policy (int n_states, int n_actions, int n_hidden, real alpha, real gamma, real lambda, bool eligibility, bool softmax, real randomness, real init_eval, bool separate_actions) : DiscretePolicy (n_states, n_actions, alpha, gamma, lambda, softmax, randomness, init_eval)
{
	this->separate_actions = separate_actions;
	this->eligibility = eligibility;
	if (eligibility) {
		message ("Using eligibility traces");
	}	
	if (separate_actions) {
		message ("Separate actions");
		J = NULL;
		Ja = new ANN* [n_actions];
		JQs = new real [n_actions];
		for (int i=0; i<n_actions; i++) {
			Ja[i] = NewANN (n_states, 1);
			if (n_hidden > 0) {
				ANN_AddHiddenLayer (Ja[i], n_hidden);
			}
			ANN_Init (Ja[i]);
			ANN_SetOutputsToLinear(Ja[i]);
			ANN_SetBatchMode(Ja[i], false);
			Ja[i]->eligibility_traces = eligibility;
			ANN_SetLambda(Ja[i],lambda*gamma);
			ANN_SetLearningRate (Ja[i], alpha);
		}
	} else {
		JQs = NULL;
		Ja = NULL;
		J = NewANN (n_states, n_actions);
		if (n_hidden > 0) {
			ANN_AddHiddenLayer (J, n_hidden);
		}
		ANN_Init (J);
		ANN_SetOutputsToLinear(J);
		ANN_SetBatchMode(J, false);
		J->eligibility_traces = eligibility;
		ANN_SetLambda(J,lambda*gamma);
		ANN_SetLearningRate (J, alpha);
	}
	ps = new real [n_states];
	delta_vector = new real [n_actions];
	J_ps_pa = 0.0;
}

ANN_Policy::~ANN_Policy()
{
	delete [] ps;
	delete [] delta_vector;
	if (separate_actions) {
		for (int i=0; i<n_actions; i++) {
			DeleteANN(Ja[i]);
		}
		delete [] Ja;
	} else {
		//ANN_ShowWeights(J);
		DeleteANN (J);
	}
}

int ANN_Policy::SelectAction (real* s, real r, int forced_a)
{
	int a; // selected action
	int amax; //maximum evaluated action
	real* Q_s; // pointer to evaluations for state s
	if (confidence) {
		if (separate_actions) {
			for (int i=0; i<n_actions; i++) {
				ANN_StochasticInput (Ja[i], s);
				JQs[i] = ANN_GetOutput(Ja[i])[0];
			}
			Q_s = JQs;
		} else {
			ANN_StochasticInput (J, s);
			Q_s = ANN_GetOutput (J);
		}
	} else {
		if (separate_actions) {
			for (int i=0; i<n_actions; i++) {
				ANN_Input (Ja[i], s);
				JQs[i] = ANN_GetOutput(Ja[i])[0];
			}
			Q_s = JQs;
		} else {
			ANN_Input (J, s);
			Q_s = ANN_GetOutput (J);
		}
	}
	int argmax = argMax (Q_s);

	if (forced_learning) {
		a = forced_a;
	} else if (confidence) {
		a = argmax;
	} else if (smax) {
		a = softMax (Q_s);
		//printf ("Q[%d][%d]=%f\n", s, a, Q[s][a]);
	} else {
		a = eGreedy (Q_s);
	}

	if (a<0 || a>=n_actions) {
		fprintf (stderr, "Action %d out of bounds\n", a);
	}

	switch (learning_method) {
		
	case Sarsa: 
		amax = a;
		break;
	case QLearning:
		amax = argmax;
		break;
	default:
		amax = a;
		fprintf (stderr, "Unknown learning method\n");
	}
	if (pa>=0) { // do not update at start of episode
		real delta = r + gamma*Q_s[amax] - J_ps_pa;
		tdError = delta;
		for (int j=0; j<n_actions; j++) {
			delta_vector[j] = 0.0;
		}
		if (separate_actions) {
			if (eligibility) {
				delta_vector[0] = 1.0;
				ANN_Delta_Train (Ja[pa], delta_vector, delta);
				// Reset other actions' traces.
				for (int i=0; i<n_actions; i++) {
					if (i!=pa) {
						ANN_Reset(Ja[i]);
					}
				}
			} else {
				delta_vector[0] = delta;
				ANN_Delta_Train (Ja[pa], delta_vector, 0.0);
			}
		} else {
			if (J->eligibility_traces) {
				delta_vector[pa] = 1.0;
				ANN_Delta_Train (J, delta_vector, delta);
			} else {
				delta_vector[pa] = delta;
				ANN_Delta_Train (J, delta_vector, 0.0);
			}
		}


	}

	//printf ("%d %d #STATE\n", min_el_state, max_el_state);
	//	printf ("Q[%d,%d]=%f r=%f e=%f ad=%f gl=%f #QV\n",
	//			ps, pa, Q[ps][pa], r, e[ps][pa], ad, gl);

	J_ps_pa = Q_s[a];
	pa = a;

	return a;
}

void ANN_Policy::Reset()
{
	if (separate_actions) {
		for (int i=0; i<n_actions; i++) {
			ANN_Reset (Ja[i]);
		}
	} else {
		ANN_Reset(J);
	}
}

/// Set to use confidence estimates for action selection, with
/// variance smoothing zeta.
bool ANN_Policy::useConfidenceEstimates (bool confidence, real zeta) {
	this->confidence = confidence;
	this->zeta = zeta;
	if (separate_actions) {
		for (int i=0; i<n_actions; i++) {
			ANN_SetZeta(Ja[i], zeta);
		}
	} else {
		ANN_SetZeta(J, zeta);
	}
	if (confidence) {
		logmsg ("#+[CONDIFENCE]\n");
	} else {
		logmsg ("#-[CONDIFENCE]\n");
	}

	return confidence;
}
