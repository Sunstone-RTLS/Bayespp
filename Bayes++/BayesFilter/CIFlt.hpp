#ifndef _BAYES_FILTER_CI
#define _BAYES_FILTER_CI

/*
 * Bayes++ the Bayesian Filtering Library
 * Copyright (c) 2002 Michael Stevens, Australian Centre for Field Robotics
 * See Bayes++.htm for copyright license details
 *
 * $Header$
 * $NoKeywords: $
 */

/*
 * Covariance Intersection Filter.
 *
 * References
 *  [1] "A Non divergent Estimation Algorithm in the Presence of Unknown Correlations"
 *   Simon J Julier, Jeffrey K Uhlmann
 *
 * CI provides a generalised consistent method to combine mean and covariances
 * of two estimates. The combination can be optimised by chosing a norm of the
 * combined correlations. The norm (omega) is restrict to 0..1 inclusive an effectively
 * scales the combination
 * Here is CI with a predict and obeserve model to form a filter.
 *
 * The Omega norm chosen here is the fixed value of 0.5
 * The Omega function should be overloaded to produce more useful results
 *
 * The filter is operated by performing a
 * 	predict, observe
 * cycle derived from the Extended_filter
 */
#include "bayesFlt.hpp"

/* Filter namespace */
namespace Bayesian_filter
{

class CI_filter : public Extended_filter
{
public:
	CI_filter (size_t x_size, size_t z_initialsize = 0);
	CI_filter& operator= (const CI_filter&);
	// Optimise copy assignment to only copy filter state

	void init ();
	void update ();
	Float predict (Linrz_predict_model& f);
	Float observe_innovation (Linrz_uncorrelated_observe_model& h, const FM::Vec& s);
	Float observe_innovation (Linrz_correlated_observe_model& h, const FM::Vec& s);

	virtual Float Omega(const FM::SymMatrix& Ai, const FM::SymMatrix& Bi, const FM::SymMatrix& A)
	// Determine norm Omega 0..1 for the CI combination
	// Default norm is the fixed value 0.5
	{
		return 0.5;
	}

public:						// Exposed Numerical Results
	FM::SymMatrix S, SI;		// Innovation Covariance and Inverse

protected:					// allow fast operation if z_size remains constant
	size_t last_z_size;
	void observe_size (size_t z_size);
};


}//namespace
#endif
