// -*- Mode: c++ -*-
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
// $Id: ann_policy.h,v 1.3 2005/08/05 09:02:58 berniw Exp $


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ANN_POLICY_H
#define ANN_POLICY_H

#include <learning/policy.h>
/** 
	A type of discrete action policy using a neural network for function approximation.
	
	Constructor arguments offer the additional option
	\c separate_actions. This is useful for the case of eligibility
	traces. It allows to use clearing actions traces, since it uses a
	separate approximator for each action, rather than a single
	approximator with many outputs.

	The class has essentially the same interface as DiscretePolicy. A
	major difference is the fact that you must supply a \c real \em
	vector that represents the state.

	Note that using Q-learning with eligibility traces in this class
	can result in divergence theoretically.
*/
class ANN_Policy : public DiscretePolicy
{
protected:
	ANN* J; ///< Evaluation network
	ANN** Ja; ///< Evaluation networks (for \c separate_actions case)
	real* ps; ///< Previous state vector \deprecated
	real* JQs; ///< Placeholder for evaluation vector (\c separate_actions)
	real J_ps_pa; ///< Evaluation of last action
	real* delta_vector; ///< Scratch vector for TD error
	bool eligibility; ///< eligibility option
	bool separate_actions; ///< Single/separate evaluation option
public:
	/// Make a new policy
	ANN_Policy (int n_states, int n_actions, int n_hidden = 0, real alpha=0.1, real gamma=0.8, real lambda=0.8, bool eligibility = false, bool softmax = false, real randomness=0.1, real init_eval=0.0, bool separate_actions = false);
	virtual ~ANN_Policy();
	/// Select an action, given a vector of real numbers which
	/// represents the state.
	virtual int SelectAction(real* s, real r, int forced_a=-1);
	/// Reset eligibility traces.
	virtual void Reset();
	/// Return the last action value.
	virtual real getLastActionValue () {return J_ps_pa;} 
	/// \deprecated Get the probabilities of all actions - call after SelectAction().
	virtual real* getActionProbabilities () {
		real sum = 0.0;
		int i;
		for (i=0; i<n_actions; i++) {
			sum += eval[i];
		}
		for (i=0; i<n_actions; i++) {
			eval[i] = eval[i]/sum;
		}
		return eval;
	}
	virtual bool useConfidenceEstimates(bool confidence, real zeta=0.01);
};


#endif
