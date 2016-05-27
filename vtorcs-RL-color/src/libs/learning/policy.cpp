// -*- Mode: c++ -*-
// $Id: policy.cpp,v 1.7 2005/09/19 12:48:03 berniw Exp $

// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstring>
#include <learning/learn_debug.h>
#include <learning/policy.h>
#include <learning/MathFunctions.h>
#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif // WIN32

#undef POLICY_LOG

#ifndef POLICY_LOG
#undef logmsg
#define logmsg empty_log
#endif

void empty_log(char* s, ...)
{
}
/// \brief Create a new discrete policy.
/// \arg n_states Number of states for the agent
/// \arg n_actions Number of actions.
/// \arg alpha Learning rate.
/// \arg gamma Discount parameter.
/// \arg lambda Eligibility trace decay.
/// \arg softmax Use softmax if true (can be overridden later)
/// \arg randomness Amount of randomness.
/// \arg init_eval Initial evaluation of actions.
DiscretePolicy::DiscretePolicy (int n_states, int n_actions, real alpha,
				real gamma, real lambda, bool softmax,
				real randomness, real init_eval)
{
	if (lambda<0.0f) lambda = 0.0f;
	if (lambda>0.99f) lambda = 0.99f;

	if (gamma<0.0f) gamma = 0.0f;
	if (gamma>0.99f) gamma = 0.99f;

	if (alpha<0.0f) alpha = 0.0f;
	if (alpha>1.0f) alpha = 1.0f;

	this->n_states = n_states;
	this->n_actions = n_actions;
	this->gamma = gamma;
	this->lambda = lambda;
	this->alpha = alpha;
	smax = softmax;
	temp = randomness;
	//logmsg ("RR:%f", temp);
	if (smax) {
		if (temp<0.1f) 
			temp = 0.1f;
	} else {
		if (temp<0.0f) { 
			temp = 0.0f;
		}
		if (temp>1.0f) {
			temp = 1.0f;
		}
	}
	learning_method = Sarsa;

	logmsg ("#Making Sarsa(lambda) ");
	if (smax) {
		logmsg ("#softmax");
	} else {
		logmsg ("#e-greedy");
	}	
	logmsg (" policy with Q:[%d x %d] -> R, a:%f g:%f, l:%f, t:%f\n",
			this->n_states, this->n_actions, this->alpha, this->gamma, this->lambda, this->temp);

	P = new real* [n_states];
	Q = new real* [n_states];
	e = new real* [n_states];
	vQ = new real* [n_states];
	for (int s=0; s<n_states; s++) {
		P[s] = new real [n_actions];
		Q[s] = new real [n_actions];
		e[s] = new real [n_actions];
		vQ[s] = new real [n_actions];
		for (int a=0; a<n_actions; a++) {
			P[s][a] = 1.0/((float)  n_actions);
			Q[s][a] = init_eval;
			e[s][a] = 0.0;
			vQ[s][a] = 1.0;
		}
	}
	pQ = 0.0;
	ps = -1;
	pa = -1;
	min_el_state = 0;
	max_el_state = n_states -1;
	eval = new real[n_actions];
	sample = new real[n_actions];
	for (int a=0; a<n_actions; a++) {
		eval[a] = 0.0;
		sample[a] = 0.0;
	}
	forced_learning = false;
	confidence = false;
	confidence_uses_gibbs = true;
	confidence_distribution = SINGULAR;
	zeta = 0.01f;
	tdError = 0.0f;
	expected_r = 0.0f;
	expected_V = 0.0f;
	n_samples = 0;
	replacing_traces = false;
}

/// \brief Save the current evaluations in text format to a file.
/// The values are saved as triplets (\c Q, \c P, \c
/// vQ). The columns are ordered by actions and the rows by state
/// number.
void DiscretePolicy::saveState(FILE* f) 
{
	if (!f)
		return;
	for (int s=0; s<n_states; s++) {
		
		//softMax(Q[s]);
		real sum2=0.0;
		int a;
		for (a=0; a<n_actions; a++) {
			sum2 += eval[a];
		}
		for (a=0; a<n_actions; a++) {
			fprintf (f, "%f ", Q[s][a]);
		}
		for (a=0; a<n_actions; a++) {
			fprintf (f, "%f ", P[s][a]);
		}
		for (a=0; a<n_actions; a++) {
			fprintf (f, "%f ", vQ[s][a]);
		}
	}

	fprintf (f, "\n");
}

/// Delete policy.
DiscretePolicy::~DiscretePolicy()
{
	real sum = 0.0;
	FILE* f = fopen ("/tmp/discrete","wb");

	int s;
	for (s=0; s<n_states; s++) {
		sum += Q[s][argMax(Q[s])];
		if (f) {
			//softMax(Q[s]);
			real sum2=0.0;
			int a;
			for (a=0; a<n_actions; a++) {
				sum2 += eval[a];
			}
			for (a=0; a<n_actions; a++) {
				fprintf (f, "%f ", Q[s][a]);
			}
			for (a=0; a<n_actions; a++) {
				fprintf (f, "%f ", P[s][a]);
			}
			for (a=0; a<n_actions; a++) {
				fprintf (f, "%f ", vQ[s][a]);
			}
			fprintf (f, "\n");
		}
	}

	if (f) {
		fclose (f);
	}

	logmsg ("#Expected return of greedy policy over random distribution of states: %f\n", sum/((real) n_states));

	for (s=0; s<n_states; s++) {
		delete [] P[s];
		delete [] Q[s];
		delete [] e[s];
		delete [] vQ[s];
	}
	delete [] P;
	delete [] Q;
	delete [] vQ;
	delete [] e;
	delete [] eval;
	delete [] sample;
}

/** Select an action a, given state s and reward from previous action.

   Optional argument a forces an action if setForcedLearning() has
   been called with true.

   Two algorithms are implemented, both of which converge. One of them
   calculates the value of the current policy, while the other that of
   the optimal policy. 

   Sarsa (\f$\lambda\f$) algorithmic description:

   1. Take action \f$a\f$, observe \f$r, s'\f$

   2. Choose \f$a'\f$ from \f$s'\f$ using some policy derived from \f$Q\f$

   3. \f$\delta = r + \gamma Q(s',a') - Q(s,a)\f$

   4. \f$e(s,a) = e(s,a)+ 1\f$, depending on trace settings

   5. for all \f$s,a\f$ :
   \f[
   Q_{t}(s,a) = Q_{t-1}(s,a) + \alpha \delta e_{t}(s,a),
   \f]
where \f$e_{t}(s,a) = \gamma \lambda e_{t-1}(s,a)\f$
   
	  end

   6. \f$a = a'\f$ (we will take this action at the next step)

   7. \f$s = s'\f$

   Watkins Q (l) algorithmic description:

   1. Take action \f$a\f$, observe \f$r\f$, \f$s'\f$

   2. Choose \f$a'\f$ from \f$s'\f$ using some policy derived from \f$Q\f$

   3. \f$a* = \arg \max_b Q(s',b)\f$

   3. \f$\delta = r + \gamma Q(s',a^*) - Q(s,a)\f$

   4. \f$e(s,a) = e(s,a)+ 1\f$, depending on eligibility traces

   5. for all \f$s,a\f$ :
\f[
        Q(s,a) = Q(s,a)+\alpha \delta e(s,a)
\f]
		if \f$(a'=a*)\f$ then \f$e(s,a)\f$ = \f$\gamma \lambda e(s,a)\f$
		           else \f$e(s,a) = 0\f$
	  end

   6. \f$a = a'\f$ (we will take this action at the next step)

   7. \f$s = s'\f$

   The most general algorithm is E-learning, currently under
   development, which is defined as follows:

   1. Take action \f$a\f$, observe \f$r\f$, \f$s'\f$

   2. Choose \f$a'\f$ from \f$s'\f$ using some policy derived from \f$Q\f$

   3. \f$\delta = r + \gamma E{Q(s',a^*)|\pi} - Q(s,a)\f$

   4. \f$e(s,a) = e(s,a)+ 1\f$, depending on eligibility traces

   5. for all \f$s,a\f$ :
\f[
        Q(s,a) = Q(s,a)+\alpha \delta e(s,a)
\f]
		\f$e(s,a)\f$ = \f$\gamma \lambda e(s,a) P(a|s,\pi) \f$

   6. \f$a = a'\f$ (we will take this action at the next step)

   7. \f$s = s'\f$

   Note that we also cut off the eligibility traces that have fallen below 0.1


*/
int DiscretePolicy::SelectAction (int s, real r, int forced_a)
{
	if ((s<0)||(s>=n_states)) {
		return 0;
	}

	if ((ps>=0)&&(pa>=0)) {
		expected_r += r;
		expected_V += Q[ps][pa];
		n_samples++;
		
		if (s==0) {
			real max_estimate = 0.0;
			real max_estimate_k = 0.0;
			for (int i=0; i<n_states; i++) {
				max_estimate += Q[i][argMax (Q[i])];
				max_estimate_k += 1.0;
			}

#if 0
			logmsg ("%f %f %f %f#rTVV\n",
					expected_r/((real) n_samples), 
					temp,
					expected_V/((real) n_samples),
					max_estimate/max_estimate_k);
#endif
			expected_r = 0.0;
			expected_V= 0.0;
			n_samples = 0;
		}
	}
	int a, amax;
	int argmax = argMax (Q[s]);

	P[s][argmax] += zeta*(1.0-P[s][argmax]);
	for (int j=0; j<n_actions; j++) {
		if (j!=argmax) {
			P[s][j] += zeta*(0.0-P[s][j]);
		}
	}



	if (forced_learning) {
		a = forced_a;
	} else if (pursuit) {
		real sum = 0.0;
		a = -1;
		int j;
		for (j=0; j<n_actions; j++) {
			sum += P[s][j];
		}
		real X = urandom()*sum;
		real dsum=0.0;
		for (j=0; j<n_actions; j++) {
			dsum += P[s][j];
			if (X<=dsum) {
				a = j;
				break;
			}
		}
		if (a==-1) {
			fprintf (stderr, "No action selected with pursuit!\n");
		}
	} else if (confidence) {
		if (confidence_uses_gibbs && (confidence_distribution == SINGULAR)) {
			a = confMax (Q[s],vQ[s]);
		} else {
			a = confSample (Q[s], vQ[s]);
			if (confidence_uses_gibbs) { // and not SINGULAR distribution
				a = softMax(sample); //use softmax on the sample values
			}
		}
	} else if (reliability_estimate) {
		temp = sqrt(Sum(vQ[s], n_actions)/((real) n_actions));
		//temp = 0.1;
		a = softMax(Q[s]);
		//printf ("%f\n", temp);
	} else if (smax) {
		a = softMax (Q[s]);
		//printf ("Q[%d][%d]=%f\n", s, a, Q[s][a]);
	} else {
		a = eGreedy (Q[s]);
	}

	if (a<0 || a>=n_actions) {
		fprintf (stderr, "Action %d out of bounds.. ", a);
		a = (int) floor (urandom()*((real) n_actions));
		fprintf (stderr, "mapping to %d\n", a);
	}

	real EQ_s = 0.0;
	int i;

	switch (learning_method) {
		
	case Sarsa: 
		amax = a;
		EQ_s = Q[s][amax];
		break;
	case QLearning:
		amax = argmax;
		EQ_s = Q[s][amax];
		break;
	case ELearning:
		amax = a; //? correct ? 
		Normalise(eval, eval, n_actions);
		EQ_s = 0.0;
		for (i=0; i<n_actions; i++) {
			EQ_s += eval[i] * Q[s][i];
		}
		break;
	default:
		amax = a;
		EQ_s = Q[s][amax];
		fprintf (stderr, "Unknown learning method\n");
	}
	if ((ps>=0)&&(pa>=0)) { // do not update at start of episode
		real delta = r + gamma*EQ_s - Q[ps][pa];
		tdError = delta;
		if (replacing_traces) {
			e[ps][pa] = 1.0;
		} else {
			e[ps][pa] += 1.0;
		}
		real ad = alpha*delta;
		real gl = gamma * lambda;
		real variance_threshold = 0.0001f;		
		if  (confidence_eligibility == false) {
			vQ[ps][pa] = (1.0 - zeta)*vQ[ps][pa] + zeta*(ad*ad);
			if (vQ[ps][pa]<variance_threshold) {
				vQ[ps][pa]=variance_threshold;
			}
		}
		if (ps<min_el_state) min_el_state = ps;
		if (ps>max_el_state) max_el_state = ps;
		

		for (i=0; i<n_states; i++) {
			//for (int i=min_el_state; i<=max_el_state; i++) {
			bool el=true;
			for (int j=0; j<n_actions; j++) {
				if (e[i][j]>0.01) {
					Q[i][j] += ad * e[i][j];
					if (confidence_eligibility == true) {
						real zeta_el = zeta * e[i][j];
						vQ[i][j] = (1.0 - zeta_el)*vQ[i][j] + zeta_el*(ad*ad);
						if (vQ[i][j]<variance_threshold) {
							vQ[i][j]=variance_threshold;
						}
					}
					//this is the same as setting e[ps][pa] += (1-P[ps][pa])
					// if P[][] remains unchanged between updates.
					// -- removed because it doesn't work! --
					//P[i][j] += 0.01*delta * e[i][j] * (1.-P[i][j]);
					if ((fabs (Q[i][j])>1000.0)||(isnan(Q[i][j]))) {
						printf ("u: %d %d %f %f\n", i,j,Q[i][j], ad * e[i][j]);
					}
					
					//This is only needed for Qlearning, but sarsa is not
					//affected since always amax==a;
					if (amax==a) {
						e[i][j] *= gl;
					} else {
						e[i][j] = 0.0;
					}
				} else {
					e[i][j] = 0.0;
					el = false;
				}
			}
			if (el==false) {
				if (min_el_state==i)
					min_el_state++;
			} else {
				max_el_state = i;
			}
		}
	}

	//printf ("%d %d #STATE\n", min_el_state, max_el_state);
	//	printf ("Q[%d,%d]=%f r=%f e=%f ad=%f gl=%f #QV\n",
	//			ps, pa, Q[ps][pa], r, e[ps][pa], ad, gl);
	ps = s;
	pa = a;

	return a;
}

/// Use at the end of every episode, after agent has entered the
/// absorbing state.
void DiscretePolicy::Reset ()
{
	for (int s=0; s<n_states; s++) {
		for (int a=0; a<n_actions; a++) {
			e[s][a] = 0.0;
		}
	}
}

/// Load policy from a file.
void DiscretePolicy::loadFile (char* f)
{
	FILE* fh = NULL;
	fh = fopen (f, "rb");
	if (fh==NULL) {
		fprintf (stderr, "Failed to read file %s\n", f);
		return;
	}
	char rtag[256];
	char* start_tag="QSA";
	char* close_tag="END";
	int n_read_states, n_read_actions;

	fread((void *) rtag, sizeof (char), strlen (start_tag)+1, fh);
	if (strcmp (rtag, start_tag)) {
		fprintf (stderr, "Could not find starting tag\n");
		return;
	}
	fread((void *) &n_read_states, sizeof(int), 1, fh);
	fread((void *) &n_read_actions, sizeof(int), 1, fh);
	
	if ((n_read_states!=n_states)||(n_read_actions!=n_actions)) {
		fprintf (stderr, "File has %dx%d space! Aborting read.\n", n_read_states, n_read_actions);
		fclose(fh);
		return;
	}

	int i, j;
	for (i=0; i<n_states; i++) {
		fread((void *) Q[i], sizeof(real), n_actions, fh);
		for (j=0; j<n_actions; j++) {
			if ((fabs (Q[i][j])>100.0)||(isnan(Q[i][j]))) {
				printf ("l: %d %d %f\n", i,j,Q[i][j]);
				Q[i][j] = 0.0;
			}
		}
	}
	for (i=0; i<n_states; i++) {
		for (j=0; j<n_actions; j++) {
			{
				P[i][j] = 1.0/((real) n_actions);
			}
		}
		int argmax = argMax (Q[i]);
		P[i][argmax] += 0.001*(1.0-P[i][argmax]);
		for (int j=0; j<n_actions; j++) {
			if (j!=argmax) {
				P[i][j] += 0.001*(0.0-P[i][j]);
			}
		}
	}



	fread((void *) rtag, sizeof (char), strlen (close_tag)+1, fh);
	if (strcmp (rtag, close_tag)) {
		fprintf (stderr, "Could not find ending tag\n");
		fclose (fh);
		return;
	}


	fclose (fh);
}

/// Save policy to a file.
void DiscretePolicy::saveFile (char* f) {
	FILE* fh = NULL;
	fh = fopen (f, "wb");
	if (fh==NULL) {
		fprintf (stderr, "Failed to write to file %s\n", f);
		return;
	}

	char* start_tag="QSA";
	char* close_tag="END";

	fwrite((void *) start_tag, sizeof (char), strlen (start_tag)+1, fh);
	fwrite((void *) &n_states, sizeof(int), 1, fh);
	fwrite((void *) &n_actions, sizeof(int), 1, fh);
	for (int i=0; i<n_states; i++) {
		fwrite((void *) Q[i], sizeof(real), n_actions, fh);
		for (int j=0; j<n_actions; j++) {
			if ((fabs (Q[i][j])>100.0)||(isnan(Q[i][j]))) {
				printf ("s: %d %d %f\n", i,j,Q[i][j]);
			}
		}
	}
	fwrite((void *) close_tag, sizeof (char), strlen (start_tag)+1, fh);
	fclose (fh);
}

/// \brief Set to use confidence estimates for action selection, with
/// variance smoothing zeta.
/// Variance smoothing currently uses a very simple method to estimate
/// the variance. 
bool DiscretePolicy::useConfidenceEstimates (bool confidence, real zeta, bool confidence_eligibility) {
	this->confidence = confidence;
	this->zeta = zeta;
	this->confidence_eligibility = confidence_eligibility;
	
	if (confidence_eligibility) {
		logmsg ("#+[ELIG_VAR]");
	}
	if (confidence) {
		logmsg ("#+[CONDIFENCE]");
	} else {
		logmsg ("#-[CONDIFENCE]\n");
	}
	
	return confidence;
}

/// Set the algorithm to QLearning mode
void DiscretePolicy::setQLearning() {
	learning_method = QLearning;
	logmsg ("#[Q-learning]\n");
}

/// Set the algorithm to ELearning mode
void DiscretePolicy::setELearning() {
	learning_method = ELearning;
	logmsg ("#[E-learning]\n");
}

/// \brief Set the algorithm to SARSA mode.
/// A unified framework for action selection.
void DiscretePolicy::setSarsa()
{
	learning_method = Sarsa;
	logmsg ("#[Sarsa]\n");
}

/// Use Pursuit for action selection.
void DiscretePolicy::setPursuit(bool pursuit)
{
	this->pursuit = pursuit;
	if (pursuit) {
		logmsg ("#+[PURSUIT]\n");
	} else {
		logmsg ("#-[PURSUIT]\n");
	}
}

/// Use Pursuit for action selection.
void DiscretePolicy::setReplacingTraces (bool replacing)
{
	this->replacing_traces = replacing;
	if (replacing) {
		logmsg ("#[REPLACING TRACES]\n");
	} else {
		logmsg ("#[ACCUMULATING TRACES]\n");
	}
}
/// Set forced learning (force-feed actions)
void DiscretePolicy::setForcedLearning(bool forced)
{
	forced_learning = forced;
}

/// Set randomness for action selection. Does not affect confidence mode.
void DiscretePolicy::setRandomness (real epsilon)
{
	temp = epsilon;
	if (smax) {
		if (temp<0.01) {
			smax = false;
		}
	}
}

/// Set the gamma of the sum to be maximised.
void DiscretePolicy::setGamma (real gamma) 
{
	this->gamma = gamma;
}

/// Set action selection to softmax.
void DiscretePolicy::useSoftmax (bool softmax) 
{
	smax = softmax;
	if (smax) {
		logmsg ("#+[SMAX]\n");
	} else {
		logmsg ("#-[SMAX]\n");
	}
}

/// Use the reliability estimate method for action selection.
void DiscretePolicy::useReliabilityEstimate (bool ri)
{
	reliability_estimate = ri;
	if (ri) {
		logmsg("#+[RI]\n");
	} else {
		logmsg("#-[RI]\n");
	}
}

/// Set the distribution for direct action sampling.
void DiscretePolicy::setConfidenceDistribution (enum ConfidenceDistribution cd)
{
	switch (cd) {
	case SINGULAR:
		logmsg("#[SINGULAR CONFIDENCE]\n"); break;
	case BOUNDED:
		logmsg("#[BOUNDED CONFIDENCE]\n"); break;
	case GAUSSIAN:
		logmsg("#[GAUSSIAN CONFIDENCE]\n"); break;
	case LAPLACIAN:
		logmsg("#[LAPLACIAN CONFIDENCE]\n"); break;
	default:
		Serror ("Unknown type %d\n", cd);
	}
	confidence_distribution = cd;
}

/// \brief Add Gibbs sampling for confidences.
/// This can be used in conjuction with any confidence distribution,
/// however it mostly makes sense for SINGULAR.
void DiscretePolicy::useGibbsConfidence (bool gibbs)
{
	if (gibbs) {
		logmsg ("#+[GIBBS CONFIDENCE]\n");
	} else {
		logmsg ("#-[GIBBS CONFIDENCE]\n");
	}
	this->confidence_uses_gibbs = gibbs;
}

// ---------- action selection helpers -------------
int DiscretePolicy::confMax(real* Qs, real* vQs, real p) {
	real sum=0.0;
	int a;
#if 0
	for (a=0; a<n_actions; a++) {
		eval[a] = exp(pow(Qs[a]/sqrt(vQs[a]), p));
		sum += eval[a];
	}
#else
	for (a=0; a<n_actions; a++) {
		real Q = Qs[a];
		real cum = 1.0;
		//real v = sqrt(vQs[a]);
		for (int j=0; j<n_actions; j++) {
			if (j!=a) {
				cum += exp ((Qs[j]-Q)/sqrt(vQs[j]));
			}
		}
		eval[a] = 1.0/(cum);//#exp(Qs[a]/sqrt(vQs[a]));
		sum += eval[a];
	}
#endif
	real X = urandom()*sum;
	real dsum = 0.0;
	for (a=0; a<n_actions; a++) {
		dsum += eval[a];
		if (X<=dsum) 
			return a;

	}
	fprintf (stderr, "ConfMax: No action selected! %f %f %f\n",X,dsum,sum);
	return -1;
}

int DiscretePolicy::confSample(real* Qs, real* vQs) {
	static NormalDistribution gaussian;
	static LaplacianDistribution laplacian;
	static UniformDistribution uniform;

	for (int a=0; a<n_actions; a++) {
		//eval[a] = Qs[a] + urandom(-1.0,1.0)*vQs[a];
		switch(confidence_distribution) {
		case SINGULAR:
			sample[a] = Qs[a];
			break;
		case BOUNDED:
			uniform.setMean(Qs[a]);
			uniform.setVariance(vQs[a]);
			sample[a] = uniform.generate();
			break;
		case GAUSSIAN:
			gaussian.setMean(Qs[a]);
			gaussian.setVariance(vQs[a]);
			sample[a] = gaussian.generate();
			break;
		case LAPLACIAN:
			laplacian.setMean(Qs[a]);
			laplacian.setVariance(vQs[a]);
			sample[a] = Qs[a] + laplacian.generate();
			break;
		default:
			Serror ("Unknown distribution ID:%d\n", confidence_distribution);
			break;
		}
	}
	return argMax(sample);
}

int DiscretePolicy::softMax(real* Qs) {
	real sum=0.0f;
	real beta = 1.0f/temp;
	int a;
	for (a=0; a<n_actions; a++) {
		eval[a] = exp(beta * Qs[a]);
		sum += eval[a];
	}
	real X = urandom()*sum;
	real dsum = 0.0;
	for (a=0; a<n_actions; a++) {
		dsum += eval[a];
		if (X<=dsum) 
			return a;

	}
	fprintf (stderr, "softMax: No action selected! %f %f %f\nT:%f\n",X,dsum,sum,temp);
	return -1;
}
int DiscretePolicy::eGreedy(real* Qs) {
	real X = urandom();
	int amax = argMax(Qs);
	real base_prob = temp/((real) n_actions);
	for (int a=0; a<n_actions; a++) {
		eval[a] = base_prob;
	}
	eval[amax] += 1.0-temp;
	if (X<temp) {
		return rand()%n_actions;
	}
	return argMax(Qs);
}

int DiscretePolicy::argMax(real* Qs) {
	real max = Qs[0];
	int arg_max = 0;
	for (int a=1; a<n_actions; a++) {
		if (max<Qs[a]) {
			max = Qs[a];
			arg_max = a;
		}
	}
	return arg_max;
}


