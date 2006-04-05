/*
 * Bayes++ the Bayesian Filtering Library
 * Copyright (c) 2004 Michael Stevens
 * See accompanying Bayes++.html for terms and conditions of use.
 *
 * $Id$
 */
 
/*
 * Implement bayesFlt.hpp :
 *  constructor/destructors
 *  error handlers
 *  default virtual and member functions
 */
#include "bayesFlt.hpp"
#include <boost/limits.hpp>
#include <vector>		// Only for unique_samples


/* Filter namespace */
namespace Bayesian_filter
{


/* Minimum allowable reciprocal condition number for PD Matrix factorisations
 * Initialised default gives 5 decimal digits of headroom */
const Bayes_base::Float Numerical_rcond::limit_PD_init = std::numeric_limits<Bayes_base::Float>::epsilon() * Bayes_base::Float(1e5);


Bayes_base::~Bayes_base()
/*
 * Default definition required for a pure virtual distructor
 */
{}

void Bayes_base::error (const Numeric_exception& e )
/*
 * Filter error
 */
{
	throw e;
}

void Bayes_base::error (const Logic_exception& e )
/*
 * Filter error
 */
{
	throw e;
}

Gaussian_predict_model::Gaussian_predict_model (std::size_t x_size, std::size_t q_size) :
		q(q_size), G(x_size, q_size)
/*
 * Set the size of things we know about
 */
{}

Additive_predict_model::Additive_predict_model (std::size_t x_size, std::size_t q_size) :
		q(q_size), G(x_size, q_size)
/*
 * Set the size of things we know about
 */
{}

Linrz_predict_model::Linrz_predict_model (std::size_t x_size, std::size_t q_size) :
/*
 * Set the size of things we know about
 */
		Additive_predict_model(x_size, q_size),
		Fx(x_size,x_size)
{}

Linear_predict_model::Linear_predict_model (std::size_t x_size, std::size_t q_size) :
/*
 * Set the size of things we know about
 */
		Linrz_predict_model(x_size, q_size),
		xp(x_size)
{}

Linear_invertible_predict_model::Linear_invertible_predict_model (std::size_t x_size, std::size_t q_size) :
/*
 * Set the size of things we know about
 */
		Linear_predict_model(x_size, q_size),
		inv(x_size)
{}

Linear_invertible_predict_model::inverse_model::inverse_model (std::size_t x_size) :
		Fx(x_size,x_size)
{}


Expected_state::Expected_state (std::size_t x_size) :
	x(x_size)
/*
 * Initialise filter and set the size of things we know about
 */
{
	if (x_size < 1)
		error (Logic_exception("Zero state filter constructed"));
}


Kalman_state::Kalman_state (std::size_t x_size) :
/*
 * Initialise state size
 */
		Expected_state(x_size), X(x_size,x_size)
{}

void Kalman_state::init_kalman (const FM::Vec& x, const FM::SymMatrix& X)
/*
 * Initialise from a state and state covariance
 *  Parameters that reference the instance's x and X members are valid
 */
{
	Kalman_state::x = x;
	Kalman_state::X = X;
	init ();
}


Bayes_base::Float
 Extended_kalman_filter::observe (Linrz_correlated_observe_model& h, const FM::Vec& z, FM::Vec& innov)
/*
 * Extended linrz correlated observe, compute innovation for observe_innovation
 */
{
	update ();
	const FM::Vec& zp = h.h(x);		// Observation model, zp is predicted observation

	innov = z;
	h.normalise(innov, zp);
	FM::noalias(innov) -= zp;
	return observe_innovation (h, innov);
}

Bayes_base::Float
 Extended_kalman_filter::observe (Linrz_uncorrelated_observe_model& h, const FM::Vec& z, FM::Vec& innov)
/*
 * Extended kalman uncorrelated observe, compute innovation for observe_innovation
 */
{
	update ();
	const FM::Vec& zp = h.h(x);		// Observation model, zp is predicted observation

	innov = z;
	h.normalise(innov, zp);
	FM::noalias(innov) -= zp;
	return observe_innovation (h, innov);
}


Information_state::Information_state (std::size_t x_size) :
/*
 * Initialise state size
 */
		y(x_size), Y(x_size,x_size)
{}

void Information_state::init_information (const FM::Vec& y, const FM::SymMatrix& Y)
/*
 * Initialise from a information state and information
 *  Parameters that reference the instance's y and Y members are valid
 */
{
	Information_state::y = y;
	Information_state::Y = Y;
	init_yY ();
}


Sample_state::Sample_state (std::size_t x_size, std::size_t s_size) :
		S(x_size,s_size)

/*
 * Initialise state size
 * Postcond: s_size >= 1
 */
{
	if (s_size < 1)
		error (Logic_exception("Zero sample filter constructed"));
}

void Sample_state::init_sample (const FM::ColMatrix& S)
/*
 * Initialise from a sampling
 */
{
	Sample_state::S = S;
	init_S();
}

namespace {
	// Column proxy so S can be sorted indirectly
	struct ColProxy
	{
		const FM::ColMatrix* cm;
		std::size_t col;
		const ColProxy& operator=(const ColProxy& a)
		{
			col = a.col;
			return a;
		}
		// Provide a ordering on columns
		static bool less(const ColProxy& a, const ColProxy& b)
		{
			FM::ColMatrix::const_iterator1 sai = a.cm->find1(1, 0,a.col);
			FM::ColMatrix::const_iterator1 sai_end = a.cm->find1(1, a.cm->size1(),a.col); 
			FM::ColMatrix::const_iterator1 sbi = b.cm->find1(1,0, b.col);
			while (sai != sai_end)
			{
				if (*sai < *sbi)
					return true;
				else if (*sai > *sbi)
					return false;

				++sai; ++sbi;
			} ;
			return false;		// Equal
		}
	};
}//namespace

std::size_t Sample_state::unique_samples () const
/*
 * Count number of unique (unequal value) samples in S
 * Implementation requires std::sort on sample column references
 */
{
						// Temporary container to Reference each element in S
	typedef std::vector<ColProxy> SRContainer;
	SRContainer sortR(S.size2());
	std::size_t col_index = 0;
	for (SRContainer::iterator si = sortR.begin(); si != sortR.end(); ++si) {
		(*si).cm = &S; (*si).col = col_index++;
	}
						// Sort the column proxies
	std::sort (sortR.begin(), sortR.end(), ColProxy::less);

						// Count element changes, precond: sortS not empty
	std::size_t u = 1;
	SRContainer::const_iterator ssi = sortR.begin();
	SRContainer::const_iterator ssp = ssi;
	++ssi;
	while (ssi < sortR.end())
	{
		if (ColProxy::less(*ssp, *ssi))
			++u;
		ssp = ssi;
		++ssi;
	}
	return u;
}


Sample_filter::Sample_filter (std::size_t x_size, std::size_t s_size) :
		Sample_state(x_size,s_size)

/*
 * Initialise filter and set the size of things we know about
 * Postcond: s_size >= 1
 */
{
}

void Sample_filter::predict (Functional_predict_model& f)
/*
 * Predict samples forward
 *		Pre : S represent the prior distribution
 *		Post: S represent the predicted distribution
 */
{
						// Predict particles S using supplied predict model
	const std::size_t nSamples = S.size2();
	for (std::size_t i = 0; i != nSamples; ++i) {
		FM::Vec Si = FM::ColMatrix::Column (S,i);
		FM::noalias(Si) = f.fx(Si);
	}
}


}//namespace
