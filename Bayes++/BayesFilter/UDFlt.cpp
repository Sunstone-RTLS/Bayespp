/*
 * Bayes++ the Bayesian Filtering Library
 * Copyright (c) 2002 Michael Stevens
 * See Bayes++.htm for copyright license details
 *
 * $Header$
 * $NoKeywords: $
 */

/*
 * UdU' Factorisation of Covariance Filter.
 *
 * For efficiency UD_scheme requires to know the maximum q_size of the predict_model
 * ISSUES:
 *  observe functions: returned rcond is the minimum of each sequential update, an overall conditioning would be better
 */
#include "bayesFlt.hpp"
#include "matSup.hpp"
#include "UDFlt.hpp"
#include <boost/limits.hpp>

/* Filter namespace */
namespace Bayesian_filter
{


UD_scheme::
UD_scheme (size_t x_size, size_t q_maxsize, size_t z_initialsize) :
		Kalman_state_filter(x_size),
		q_max(q_maxsize),
		UD(x_size,x_size+q_max),
		s(FM::Empty), Sd(FM::Empty),
		d(x_size+q_max), dv(x_size+q_max), v(x_size+q_max),
		a(x_size), b(x_size),
		h1(x_size), w(x_size),
		znorm(FM::Empty),
		zpdecol(FM::Empty),
		Gz(FM::Empty),
		GIHx(FM::Empty)
/*
 * Initialise filter and set the size of things we know about
 */
{
	last_z_size = 0;	// Leave z_size dependants Empty if z_initialsize==0
	observe_size (z_initialsize);
}

UD_scheme&
 UD_scheme::operator= (const UD_scheme& a)
/* Optimise copy assignment to only copy filter state
 * Precond: matrix size conformance
 */
{
	Kalman_state_filter::operator=(a);
	q_max = a.q_max;
	UD = a.UD;
	return *this;
}


void
 UD_scheme::init ()
/*
 * Initialise from a state and state coveriance
 * Computes UD factor from initial covaiance
 * Predcond:
 *  X
 * Postcond:
 *  X
 *  UD=X, d is PSD
 */
{
					// Factorise X into left partition of UD
	size_t x_size = UD.size1();
	UD.sub_matrix(0,x_size, 0,x_size).assign( X );
	Float rcond = FM::UdUfactor (UD, x_size);
	rclimit.check_PSD(rcond, "Initial X not PSD");
}


void
 UD_scheme::update ()
/*
 * Defactor UD back into X
 * Precond:
 *  UD
 * Postcond:
 *  X=UD  PSD iff UD is PSD
 */
{
	FM::UdUrecompose (X, UD);
}


UD_scheme::Float
 UD_scheme::predict (Linrz_predict_model& f)
/*
 * Prediction using a diagonalised noise q, and its coupling G
 *  q can have order less then x and a matching G so GqG' has order of x
 * Precond:
 *	UD
 * Postcond:
 *  UD is PSD
 */
{
	x = f.f(x);			// Extended Kalman state predict is f(x) directly

						// Predict UD from model
	Float rcond = predictGq (f.Fx, f.G, f.q);
	rclimit.check_PSD(rcond, "X not PSD in predict");
	return rcond;
}


UD_scheme::Float
 UD_scheme::predictGq (const FM::Matrix& Fx, const FM::Matrix& G, const FM::Vec& q)
/*
 * MWG-S prediction from Bierman  p.132
 *  q can have order less then x and a matching G so GqG' has order of x
 * Precond:
 *  UD
 * Postcond:
 *  UD
 *
 * Return:
 *		reciprocal condition number, -1 if negative, 0 if semi-definate (including zero)
 */
{
	size_t i,j,k;
	const size_t n = x.size();
	const size_t Nq = q.size();
	const size_t N = n+Nq;
	Float e;
					// Check preallocated space for q size
	if (Nq > q_max)
		error (Logic_exception("Predict model q larger than preallocated space"));

	if (n > 0)		// Simplify reverse loop termination
	{
						// Augment d with q, UD with G
		for (i = 0; i < Nq; ++i)		// 0..Nq-1
		{
			d[i+n] = q[i];
		}
		for (j = 0; j < n; ++j)		// 0..n-1
		{
			FM::Matrix::Row UDj(UD,j);
			FM::Matrix::const_Row  Gj(G,j);
			for (i = 0; i < Nq; ++i)		// 0..Nq-1
				UDj[i+n] = Gj[i];
		}

						// U=Fx*U and diagonals retrived
		for (j = n-1; j > 0; --j)		// n-1..1
		{
						// Prepare d(0)..d(j) as temporary
			for (i = 0; i <= j; ++i)	// 0..j
				d[i] = UD(i,j);

						// Lower triangle of UD is implicity empty
			for (i = 0; i < n; ++i) 	// 0..n-1
			{
				FM::Matrix::Row UDi(UD,i);
				FM::Matrix::const_Row Fxi(Fx,i);
				UDi[j] = Fxi[j];
				for (k = 0; k < j; ++k)	// 0..j-1
					UDi[j] += Fxi[k] * d[k];
			}
		}
		d[0] = UD(0,0);

						//  Complete U = Fx*U
		for (j = 0; j < n; ++j)			// 0..n-1
		{
			UD(j,0) = Fx(j,0);
		}

						// The MWG-S algorithm on UD transpose
		j = n-1;
		do {							// n-1..0
			FM::Matrix::Row UDj(UD,j);
			e = 0.;
			for (k = 0; k < N; ++k)		// 0..N-1
			{
				v[k] = UDj[k];
				dv[k] = d[k] * v[k];
				e += v[k] * dv[k];
			}
			// Check diagonal element
			if (e > 0.)
			{
				// Positive definate
				UDj[j] = e;

				Float diaginv = 1 / e;
				for (k = 0; k < j; ++k)	// 0..j-1
				{
					FM::Matrix::Row UDk(UD,k);
					e = 0.;
					for (i = 0; i < N; ++i)	// 0..N-1
						e += UDk[i] * dv[i];
					e *= diaginv;
					UDj[k] = e;

					for (i = 0; i < N; ++i)	// 0..N-1
						UDk[i] -= e * v[i];
				}
			}//PD
			else if (e == 0.)
			{
				// Possibly Semidefinate, check not negative
				UDj[j] = e;

				// 1 / e is infinite
				for (k = 0; k < j; ++k)	// 0..j-1
				{
					FM::Matrix::Row UDk(UD,k);
					for (i = 0; i < N; ++i)	// 0..N-1
					{
						e = UDk[i] * dv[i];
						if (e != 0.)
							goto Negative;
					}
					// UD(j,k) uneffected
				}
			}//PD
			else
			{
				// Negative
				goto Negative;
			}
		} while (j-- > 0); //MWG-S loop

						// Transpose and Zero lower triangle
		for (j = 1; j < n; ++j)			// 0..n-1
		{
			FM::Matrix::Row UDj(UD,j);
			for (i = 0; i < j; ++i)
			{
				UD(i,j) = UDj[i];
				UDj[i] = 0.;			// Zeroing unnecessary as lower only used as a scratch
			}
		}

	}

	// Estimate the reciprocal condition number from upper triangular part
	return FM::UdUrcond(UD,n);

Negative:
	return -1;
}


void
 UD_scheme::observe_size (size_t z_size)
/*
 * Optimised dyamic observation sizing
 */
{
	if (z_size != last_z_size) {
		last_z_size = z_size;

		s.resize(z_size);
		Sd.resize(z_size);
		znorm.resize(z_size);
	}
}

Bayes_base::Float
 UD_scheme::observe (Linrz_uncorrelated_observe_model& h, const FM::Vec& z)
/*
 * Standard linrz observe
 *  Uncorrelated observations are applied sequentialy in the order they appear in z
 *  The sequential observation updates state x
 *  Therefore the model of each observation needs to be computed sequentially. Generally this
 *  is inefficient and observe (UD_sequential_observe_model&) should be used instead
 * Precond:
 *	UD
 *	Zv is PSD
 * Postcond:
 *  UD is PSD
 * Return: Minimum rcond of all squential observe
 */
{
	const size_t z_size = z.size();
	Float s, S;			// Innovation and covariance

								// Dynamic sizing
	observe_size (z_size);
								// Apply observations sequentialy as they are decorrelated
	Float rcondmin = std::numeric_limits<Float>::max();
	for (size_t o = 0; o < z_size; ++o)
	{
								// Observation model, extracted for a single z element
		const FM::Vec& zp = h.h(x);
		h.normalise(znorm = z, zp);
		h1 = FM::row(h.Hx,o);
								// Check Z precondition
		if (h.Zv[o] < 0.)
			error (Numeric_exception("Zv not PSD in observe"));
								// Update UD and extract gain
		Float rcond = observeUD (w, S, h1, h.Zv[o]);
		rclimit.check_PSD(rcond, "S not PD in observe");	// -1 implies S singular
		if (rcond < rcondmin) rcondmin = rcond;
								// State update using normalised non-linear innovation
		s = znorm[o] - zp[o];
		FM::noalias(x) += w * s;
								// Copy s and Sd
		UD_scheme::s[o] = s;
		UD_scheme::Sd[o] = S;
	}
	return rcondmin;
}

Bayes_base::Float
 UD_scheme::observe (Linrz_correlated_observe_model& /*h*/, const FM::Vec& /*z*/)
/* No solution for Correlated noise and Linearised model */
{
	error (Logic_exception("observe no Linrz_correlated_observe_model solution"));
	return 0.;	// never reached
}

Bayes_base::Float
 UD_scheme::observe (Linear_correlated_observe_model& h, const FM::Vec& z)
/*
 * Special Linear Hx observe for correlated Z
 *  Z must be PD and will be decorrelated
 * Applies observations sequentialy in the order they appear in z
 * Creates temporary Vec and Matrix to decorelate z,Z
 * Predcondition:
 *  UD
 *  Z is PSD
 * Postcondition:
 *  UD is PSD
 * Return: Minimum rcond of all squential observe
 */
{
	size_t i, j, k;
	const size_t x_size = x.size();
	const size_t z_size = z.size();
	Float s, S;			// Innovation and covariance

					// Dynamic sizing
	observe_size (z_size);
	if (z_size != zpdecol.size()) {
		zpdecol.resize(z_size);
		Gz.resize(z_size,z_size);
		GIHx.resize(z_size, x_size);
	}

					// Factorise process noise as GzG'
	{	Float rcond = FM::UdUfactor (Gz, h.Z);
		rclimit.check_PSD(rcond, "Z not PSD in observe");
	}

								// Observation prediction and normalised observation
	const FM::Vec& zp = h.h(x);
	h.normalise(znorm = z, zp);
	
	if (z_size > 0)
	{							// Solve G* GIHx = Hx for GIHx in-place
		GIHx = h.Hx;
		for (j = 0; j < x_size; ++j)
		{
			i = z_size-1;
			do {
				for (k = i+1; k < z_size; ++k)
				{
					GIHx(i,j) -= Gz(i,k) * GIHx(k,j);
				}
			} while (i-- > 0);
		}
					
		zpdecol = zp;			// Solve G zp~ = z, G z~ = z  for zp~,z~ in-place
		i = z_size-1;
		do {
			for (k = i+1; k < z_size; ++k)
			{
				znorm[i] -= Gz(i,k) * znorm[k];
				zpdecol[i] -= Gz(i,k) * zpdecol[k];
			}
		} while (i-- > 0);
	}//if (z_size>0)

								// Apply observations sequentialy as they are decorrelated
	Float rcondmin = std::numeric_limits<Float>::max();
	for (size_t o = 0; o < z_size; ++o)
	{
		h1 = FM::row(GIHx,o);
								// Update UD and extract gain
		Float rcond = observeUD (w, S, h1, Gz(o,o));
		rclimit.check_PSD(rcond, "S not PD in observe");	// -1 implies S singular
		if (rcond < rcondmin) rcondmin = rcond;
								// State update using linear innovation
		s = znorm[o]-zpdecol[o];
		FM::noalias(x) += w * s;
								// Copy s and Sd
		UD_scheme::s[o] = s;
		UD_scheme::Sd[o] = S;
	}
	return rcondmin;
}

Bayes_base::Float
 UD_scheme::observe (UD_sequential_observe_model& h, const FM::Vec& z)
/*
 * Special observe using observe_model_sequential for fast uncorrelated linrz operation
 * Uncorrelated observations are applied sequentialy in the order they appear in z
 * The sequential observation updates state x. Therefore the model of
 * each observation needs to be computed sequentially
 * Predcondition:
 *  UD
 *  Z is PSD
 * Postcondition:
 *  UD is PSD
 * Return: Minimum rcond of all squential observe
 */
{
	size_t o;
	const size_t z_size = z.size();
	Float s, S;			// Innovation and covariance

								// Dynamic sizing
	observe_size (z_size);
								// Apply observations sequentialy as they are decorrelated
	Float rcondmin = std::numeric_limits<Float>::max();
	for (o = 0; o < z_size; ++o)
	{
								// Observation prediction and model
		const FM::Vec& zp = h.ho(x, o);
		h.normalise(znorm = z, zp);
								// Check Z precondition
		if (h.Zv[o] < 0.)
			error (Numeric_exception("Zv not PSD in observe"));
								// Update UD and extract gain
		Float rcond = observeUD (w, S, h.Hx_o, h.Zv[o]);
		rclimit.check_PSD(rcond, "S not PD in observe");	// -1 implies S singular
		if (rcond < rcondmin) rcondmin = rcond;
								// State update using non-linear innovation
		s = znorm[o]-zp[o];
		FM::noalias(x) += w * s;
								// Copy s and Sd
		UD_scheme::s[o] = s;
		UD_scheme::Sd[o] = S;
	}
	return rcondmin;
}


UD_scheme::Float
 UD_scheme::observeUD (FM::Vec& gain, Float & alpha, const FM::Vec& h, const Float r)
/*
 * Linear UD factorisation update
 *  Bierman UdU' factorisation update. Bierman p.100
 * Input
 *  h observation coeficients
 *  r observation variance
 * Output
 *  gain  observation Kalman gain
 *  alpha observation innovation variance
 * Variables with physical significance
 *  gamma becomes covariance of innovation
 * Predcondition:
 *  UD
 *  r is PSD (not checked)
 * Postcondition:
 *  UD (see return value)
 * Return:
 *  reciprocal condition number of UD, -1 if alpha singular (negative or zero)
 */
{
	size_t i,j,k;
	const size_t n = UD.size1();
	Float gamma, alpha_jm1, lamda;
	// a(n) is U'a
	// b(n) is Unweighted Kalman gain

					// Compute b = DU'h, a = U'h
	a = h;
	for (j = n-1; j >= 1; --j)	// n-1..1
	{
		for (k = 0; k < j; ++k)	// 0..j-1
		{
			a[j] += UD(k,j) * a[k];
		}
		b[j] = UD(j,j) * a[j];
	}
	b[0] = UD(0,0) * a[0];

					// Update UD(0,0), d(0) modification
	alpha = r + b[0] * a[0];
	if (alpha <= 0) goto alphaNotPD;
	gamma = 1 / alpha;
	UD(0,0) *= r * gamma;
					// Update rest of UD and gain b
	for (j = 1; j < n; ++j)		// 1..n-1
	{
					// d modification
		alpha_jm1 = alpha;	// alpha at j-1
		alpha += b[j] * a[j];
		lamda = -a[j] * gamma;
		if (alpha <= 0.) goto alphaNotPD;
		gamma = 1 / alpha;
		UD(j,j) *= alpha_jm1 * gamma;
					// U modification
		for (i = 0; i < j; ++i)		// 0..j-1
		{
			Float UD_jm1 = UD(i,j);
			UD(i,j) = UD_jm1 + lamda * b[i];
			b[i] += b[j] * UD_jm1;
		}
	}
					// Update gain from b
	for (j = 0; j < n; ++j)		// 0..n-1
	{
		gain[j] = b[j] * gamma;
	}
 	// Estimate the reciprocal condition number from upper triangular part
	return FM::UdUrcond(UD,n);

alphaNotPD:
	return -1;
}

}//namespace
