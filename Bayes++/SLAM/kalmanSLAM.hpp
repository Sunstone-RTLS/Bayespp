/*
 * Bayes++ the Bayesian Filtering Library
 * Copyright (c) 2004 Michael Stevens
 * See accompanying Bayes++.html for terms and conditions of use.
 *
 * $Header$
 */

/*
 * SLAM : Simultaneous Locatization and Mapping
 *  Kalman filter representation of SLAM
 *  A very simplistic and INEFFICIENT full correlation implementation.
 *  The feature numbers should be sequential to avoid sparseness in the full filter.
 *  The filter size grows with the feature number, but never shrinks
 *
 * Reference
 *  [1] "A Solution to the Simultaneous Localization and Map Building (SLAM) Problem"
 *   MWM Gamini Dissanayake, Paul Newman, Steven Clark, Hugh Durrant-Wyte, M Csorba, IEEE T Robotics and Automation vol.17 no.3 June 2001
 */


namespace SLAM_filter
{


class Kalman_filter_generator
{
public:
	typedef Bayesian_filter::Linrz_kalman_filter Filter_type;
	virtual Filter_type* generate( unsigned full_size ) =0;
	virtual void dispose( Filter_type* filter ) =0;
	virtual ~Kalman_filter_generator()
	{}
};

class Kalman_SLAM : public SLAM
{
public:
	Kalman_SLAM( Kalman_filter_generator& filter_generator );
	~Kalman_SLAM();
	void init_kalman( const FM::Vec& x, const FM::SymMatrix& X );
	void predict( Bayesian_filter::Linear_predict_model& m );

	void observe( unsigned feature, const Feature_observe& fom, const FM::Vec& z );
	void observe_new( unsigned feature, const Feature_observe_inverse& fom, const FM::Vec& z );
	void observe_new( unsigned feature, const FM::Float& t, const FM::Float& T );
	void forget( unsigned feature, bool must_exist = true );

	void update()
	// Compute sample mean and covariance statistics of filter
	{
		full->update();
	}

	void statistics_sparse( BF::Kalman_state_filter& kstats ) const;
	void decorrelate( Bayesian_filter::Bayes_base::Float d);

protected:
	Kalman_filter_generator& fgenerator;
	// Full Kalman representation of state
	Kalman_filter_generator::Filter_type* full;

private:
	unsigned nL;		// No of location states
	unsigned nM;		// No of map states
};


}//namespace SLAM