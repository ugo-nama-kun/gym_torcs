// -*- Mode: c++ -*-
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
// $Id: policy.h,v 1.4 2005/08/05 09:02:58 berniw Exp $


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef POLICY_H
#define POLICY_H

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <learning/ANN.h>
#include <learning/Distribution.h>

/*! 
  \class DiscretePolicy
  \brief Discrete policies with reinforcement learning.

  This class implements a discrete policy using the Sarsa(\f$\lambda\f$)
  or Q(\f$\lambda\f$) Reinforcement Learning algorithm. After creating an
  instance of the algorithm were the number of actions and states are
  specified, you can call the SelectAction() method at every time step
  to dynamically select an action according to the given state. At the
  same time you must provide a reinforcement, which should be large
  (or positive) for when the algorithm is doing well, and small (or
  negative) when it is not. The algorithm will try and maximise the
  total reward received.

  Parameters:
  
  n_states: The number of discrete states, or situations, that the
  agent is in. You should create states that are relevant to the task.
  
  n_actions: The number of different things the agent can do at every
  state. Currently we assume that all actions are usable at all
  states. However if an action a_i is not usable in some state s, then
  you can make the client side of the code for state s map state a_i
  to some usable state a_j, meaning that when the agent selects action
  a_i in state s, the result would be that of taking one of the usable
  actions. This will make the two actions equivalent. Alternatively
  when the agent selects an unusable action at a particular state, you
  could have a random outcome of actions. The algorithm can take care
  of that, since it assumes no relationship between the same action at
  different states.


  alpha: Learning rate. Controls how fast the evaluation is changed at
  every time step. Good values are between 0.01 and 0.1. Lower than
  0.01 makes learning too slow, higher than 0.1 makes learning
  unstable, particularly for high gamma and lambda.

  gamma: The algorithm will maximise the exponentially
  decaying sum of all future rewards, were the base of the exponent is
  gamma. If gamma=0, then the algorithm will always favour short-term
  rewards over long-term ones. Setting gamma close to 1 will make
  long-term rewards more important.

  lambda: This controls how much the expected value of reinforcement
  is close to the observed value of reinforcement. Another view is
  that of it controlling the eligibility traces e. Eligibility traces
  perform temporal credit assignment to previous actions/states. High
  values of lambda (near 1) can speed up learning. With lambda=0, only
  the currently selected action/state pair evaluation is updated. With
  lambda>0 all state/action pair evaluations taken in the past are
  updated, with most recent pairs updated more than pairs further in
  the past. Ultimately the optimal value of lambda will depend on the
  task.

  softmax, randomness: If this is false, then the algorithm selects
  the best possible action all the time, but with probability
  'randomness' selects a completely random action at each timestep. If
  this is true, then the algorithm selects actions stochastically,
  with probability of selecting each action proportional to how better
  it seems to be than the others. A high randomness (>10.0) will create more
  or less equal probabilities for actions, while a low randomness
  (<0.1) will make the system almost deterministic.
  
  init_eval: This is the initial evaluation for actions. It pays to
  set this to optimistic values (i.e. higher than the reinforcement to
  be given), so that the algorithm becomes always 'disappointed' and
  tries to explore as much of the possible combinations as it can.
  
  Member functions: 

  setQLearning(): Use the Q(\f$\lambda\f$) algorithm

  setELearning(): Use the E(\f$\lambda\f$) algorithm

  setSarsa(): Use the Sarsa(\f$\lambda\f$) algorithm

  All the above algorithms attempt to approximate the optimal value
  function by using an update of the form
  \f[
  Q_{t+1}(s,a) = Q_{t}(s,a) + \alpha (r_{t+1} + \gamma E\{Q(s'|\pi)\} - Q_{t}(s,a)).
  \f]

  The difference lies in how the expectation operator is
  approximated. In the case of SARSA, it is done by directly sampling
  the current policy, i.e. the expected value of Q for the next state
  is the current evaluation for the action actually taken in the next
  state. In Q-learning the expected value is that of the greedy action
  in the next state (with some minor complications to accommodate
  eligibility traces). E-learning is the most general case. The
  simplest implementation works by just replacing the single sample
  from Q with \f$\sum_{b} Q(s',b) P(a'=b|s')\f$. This lies somewhere
  in between SARSA and Q-learning.

  setPursuit (bool pursuit): If true, use pursuit methods to determine
  the best possible action. This enforces maximum exploration
  initially, and maximum exploitation of estimates later. I am not
  sure of the convergence properties of this method, however, when
  used in conjuction with Sarsa or Q-learning.

  useConfidenceEstimates(bool confidence, float zeta): Use confidence
  estimates for the estimated parameters. This allows automatic
  exploration-exploitation tradeoffs. The zeta parameter controls how
  smooth the estimates of the confidence are, lower values for
  smoother.  (defaults to 0.01). Now, given estimates Q_1 and Q_2 for
  actions 1,2 respectively we assume a Laplacian distribution centered
  around Q_1 and Q_2, with variance equal to v_1 and v_2. With Gibbs
  sampling, the probability of selecting action Q_1 is then
  \f[
  P[1]=exp(Q_1/\sqrt{v_1})/(exp(Q_1/\sqrt{v_1})+exp(Q_2/\sqrt{v2}))
  \f].

  However it is possible to perform direct sampling.

  setForcedLearning(bool forced);
*/ 

/// Types of learning methods.
enum LearningMethod {QLearning, Sarsa, ForcedLearning, ELearning};
/// Types of confidence distributions.
enum ConfidenceDistribution {SINGULAR, BOUNDED, GAUSSIAN, LAPLACIAN};

class DiscretePolicy
{
protected:

	enum LearningMethod learning_method; ///< learning method to use;
	int n_states; ///< number of states
	int n_actions; ///< number of actions
	real** Q; ///< state-action evaluation
 	real** e; ///< eligibility trace
	real* eval; ///< evaluation of current aciton
	real* sample; ///< sampling output
	real pQ; ///< previous Q
	int ps; ///< previous state
	int pa; ///< previous action
	real r; ///< reward
	real temp; ///< scratch
	real tdError; ///< temporal difference error
 	bool smax; ///< softmax option
	bool pursuit; ///< pursuit option
	real** P; ///< pursuit action probabilities
	real gamma; ///< Future discount parameter
	real lambda; ///< Eligibility trace decay
	real alpha; ///< learning rate
	real expected_r; ///< Expected reward
	real expected_V; ///< Expected state return
	int n_samples; ///< number of samples for above expected r and V
	int min_el_state; ///< min state ID to search for eligibility \deprecated
	int max_el_state; ///< max state ID to search for eligibility \deprecated
	bool replacing_traces; ///< Replacing instead of accumulating traces
	bool forced_learning; ///< Force agent to take supplied action
	bool confidence; ///< Confidence estimates option
	bool confidence_eligibility; ///< Apply eligibility traces to confidence
	bool reliability_estimate; ///< reliability estimates option
	enum ConfidenceDistribution confidence_distribution; ///< Distribution to use for confidence sampling
	bool confidence_uses_gibbs; ///< Additional gibbs sampling for confidence
	real zeta; ///< Confidence smoothing
	real** vQ; ///< variance estimate for Q
	int confMax(real* Qs, real* vQs, real p = 1.0); ///< Confidence-based Gibbs sampling
	int confSample(real* Qs, real* vQs); ///< Directly sample from action value distribution
	int softMax(real* Qs); ///< Softmax Gibbs sampling
	int eGreedy(real* Qs); ///< e-greedy sampling
	int argMax(real* Qs); ///< Get ID of maximum action
public:
	DiscretePolicy (int n_states, int n_actions, real alpha=0.1, real gamma=0.8, real lambda=0.8, bool softmax = false, real randomness=0.1, real init_eval=0.0);
	/// Kill the agent and free everything.
	virtual ~DiscretePolicy();
	/// Set the learning rate.
	virtual void setLearningRate (real alpha) {this->alpha = alpha;}
	/// Get the temporal difference error of the \em previous action.
	virtual real getTDError () {return tdError;}
	/// Get the vale of the last action taken.
	virtual real getLastActionValue () {return Q[ps][pa];}
	virtual int SelectAction(int s, real r, int forced_a=-1);
	virtual void Reset();
	virtual void loadFile (char* f);
	virtual void saveFile (char* f);
	virtual void setQLearning();
	virtual void setELearning();
	virtual void setSarsa();
	virtual bool useConfidenceEstimates(bool confidence, real zeta=0.01, bool confidence_eligibility = false);
	virtual void setForcedLearning(bool forced);
	virtual void setRandomness (real epsilon);
	virtual void setGamma (real gamma);
	virtual void setPursuit (bool pursuit);
	virtual void setReplacingTraces (bool replacing);
	virtual void useSoftmax (bool softmax);
	virtual void setConfidenceDistribution (enum ConfidenceDistribution cd);
	virtual void useGibbsConfidence (bool gibbs);
	virtual void useReliabilityEstimate (bool ri);
	virtual void saveState (FILE* f);
};


#endif
