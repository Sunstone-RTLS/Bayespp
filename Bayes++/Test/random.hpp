/*
 * Bayes++ the Bayesian Filtering Library
 * Copyright (c) 2004 Michael Stevens
 * See accompanying Bayes++.html for terms and conditions of use.
 *
 * $Header$
 */

/*
 * Good random numbers from Boost
 *  Provides a common class  for all random number requirements to test Bayes++
 */

#include <boost/random.hpp>


namespace Bayesian_filter_test
{

/*
 * Random numbers from Boost Random
 */

namespace
{
	// ISSUE variate_generator cannot be used without Partial template specialistion
	template<class Engine, class Distribution>
	class simple_generator
	{
	public:
		typedef typename Distribution::result_type result_type;
		simple_generator(Engine& e, Distribution& d)
			: _eng(e), _dist(d)
		{}
		result_type operator()()
		{	return _dist(_eng); }
	private:
		Engine& _eng;
		Distribution& _dist;
	};
}//namespace

class Boost_random
{
public:
	// ISSUE mt19937 is failing on x86_64
	//	typedef boost::mt19937 Boost_gen;
	typedef boost::ranlux3 Boost_gen;	

	typedef Bayesian_filter_matrix::Float Float;
	typedef boost::uniform_01<Boost_gen, Float> UGen;
	Boost_random() : gen01(Boost_gen()), dist_normal()
	{}
	Bayesian_filter_matrix::Float normal(const Float mean, const Float sigma)
	{
		boost::normal_distribution<Float> dist(mean, sigma);
		simple_generator<UGen, boost::normal_distribution<Float> > gen(gen01, dist);
		return gen();
	}
	void normal(Bayesian_filter_matrix::DenseVec& v, const Float mean, const Float sigma)
	{
		boost::normal_distribution<Float> dist(mean, sigma);
		simple_generator<UGen, boost::normal_distribution<Float> > gen(gen01, dist);
		std::generate (v.begin(), v.end(), gen);
	}
	void normal(Bayesian_filter_matrix::DenseVec& v)
	{
		simple_generator<UGen, boost::normal_distribution<Float> > gen(gen01, dist_normal);
		std::generate (v.begin(), v.end(), gen);
	}
	void uniform_01(Bayesian_filter_matrix::DenseVec& v)
	{
		std::generate (v.begin(), v.end(), gen01);
	}
#ifdef BAYES_FILTER_GAPPY
	void normal(Bayesian_filter_matrix::Vec& v, const Float mean, const Float sigma)
	{
		boost::normal_distribution<Float> dist(mean, sigma);
		simple_generator<UGen, boost::normal_distribution<Float> > gen(gen01, dist);
		for (std::size_t i = 0, iend=v.size(); i < iend; ++i)
			v[i] = gen();
	}
	void normal(Bayesian_filter_matrix::Vec& v)
	{
		simple_generator<UGen, boost::normal_distribution<Float> > gen(gen01, dist_normal);
		for (std::size_t i = 0, iend=v.size(); i < iend; ++i)
			v[i] = gen();
	}
	void uniform_01(Bayesian_filter_matrix::Vec& v)
	{
		for (std::size_t i = 0, iend=v.size(); i < iend; ++i)
			v[i] = gen01();
	}
#endif
	void seed()
	{
		gen01.base().seed();
	}
private:
	UGen gen01;
	boost::normal_distribution<Float> dist_normal;
};


}//namespace
