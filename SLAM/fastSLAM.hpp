/*
 * Bayes++ the Bayesian Filtering Library
 * Copyright (c) 2002 Michael Stevens, Australian Centre for Field Robotics
 * See Bayes++.htm for copyright license details
 *
 * $Header$
 */

/*
 * SLAM : Simultaneous Locatization and Mapping
 *  FastSLAM augmented particle algorithm see Ref[1]
 *
 * Reference
 *  [1] "FastSLAM: Factored Solution to the Simultaneous Localization and Mapping Problem"
 *   Montemerlo, M. and Thrun, S. and Koller, D. and Wegbreit, B. Proceedings of the AAAI National Conference on Artificial Intelligence  2002
 */


namespace SLAM_filter
{


class Fast_SLAM : public SLAM
/*
 * FastSLAM Filter
 *  Implements mechanisation of FastSLAM Algorithm:
 *  Restricted to single state features represented by mean and covariance
 *  Requires a SIR_filter (contructor parameter) to represent the location part of the state and provied resampling
 */
{
public:
	Fast_SLAM( Bayesian_filter::SIR_scheme& L_filter );
	// Construct Fast_SLAM filter using referenced filter for resampling

									// Single Feature observations (single element vectors)
	void observe( unsigned feature, const Feature_observe& fom, const FM::Vec& z);
	void observe_new( unsigned feature, const Feature_observe_inverse& fom, const FM::Vec& z);
	void observe_new( unsigned feature, const FM::Vec& t, const FM::Vec& T);

	void forget( unsigned feature, bool must_exist = true);

	virtual Float update_resample( const Bayesian_filter::Importance_resampler& resampler);
	/* Resampling Update: resample particles using weights and then roughen
	 *	Returns lcond, Smallest normalised likelihood weight, represents conditioning of resampling solution
	 *          lcond == 1. if no resampling performed
	 *			This should by multipled by the number of samples to get the Likelihood function conditioning
	 */

	virtual void update()
	// Default update, standard resample
	{
		(void)update_resample (Bayesian_filter::Standard_resampler());
	}

	unsigned feature_unique_samples( unsigned feature );

protected:
	struct Feature_1
	{	// Single feature represetation : mean and variance
		Float x, X;
	};

	struct FeatureCondMap : public std::vector<Feature_1>
	// Particle Conditional map for a feature
	{	explicit FeatureCondMap(unsigned nParticles) : std::vector<Feature_1>(nParticles)
		{}
	};
	typedef std::map<unsigned, FeatureCondMap> AllFeature;	// Particle Maps for all features: associative container keys by feature number

	Bayesian_filter::SIR_scheme& L;		// Location part of state (particle form). Reference to filter parameter in constructor
	AllFeature M;						// Map part of state (augmentation to particles)

private:
	typedef Bayesian_filter::Importance_resampler::Resamples_t Resamples_t;
	FM::DenseVec wir;			// Likelihood weights of map augmented particles
	bool wir_update;		// weights have been updated requring a resampling on update
};


class Fast_SLAM_Kstatistics : public Fast_SLAM
/*
 * A simple encapsulation of FastSLAM with Kalman_filter statistics
 */
{
public:
	Fast_SLAM_Kstatistics( BF::SIR_kalman_scheme& L_filter);
	// Construct Fast_SLAM filter using referenced filter for resampling
	
	unsigned statistics( BF::Kalman_state_filter& kstats);
	// Compute statistics of particles. Return: Number of features in map
	//  Sample mean and covariance of particle.

protected:
	BF::SIR_kalman_scheme& L;			// Reference to filter parameter in constructor
};

}//namespace SLAM
