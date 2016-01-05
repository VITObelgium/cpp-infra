/*
 * FeedForwardModel.h
 *
 *  Created on: 2014
 *      Author: bino.maiheu@vito.be
 */

#ifndef MLP_FEEDFORWARDFORECASTMODEL_H_
#define MLP_FEEDFORWARDFORECASTMODEL_H_

#include <string>
#include <opaq.h>

namespace OPAQ {
  
/**
 * Implementation of an OPAQ multi layer perceptron feed forward neural
 * network forecast model. Usable feedforward model derive from this and
 * implement their own sample creating routine. This class is an abstract
 * base class for the actuall plugins.
 */
class MLP_FeedForwardModel: virtual public OPAQ::Model {
public:
	MLP_FeedForwardModel();
    virtual ~MLP_FeedForwardModel();

    // the configure method should also be implemented in the derived class...
    // OPAQ::Model methods --> run for this particular fcTime...
    virtual void run();
    
    /**
     *  Workhorse routine which acutally performs the forecast and returns the
     *  value
     */
    double fcValue( const OPAQ::Pollutant& pol, const OPAQ::Station& station,
    			OPAQ::Aggregation::Type aggr, const OPAQ::DateTime& baseTime,
				const OPAQ::TimeInterval& fc_hor );

    // Some helper routines, make them public
    static double mean_missing( const std::vector<double> & list, double noData );
    static double max_missing( const std::vector<double> & list, double noData );
    static double min_missing( const std::vector<double> & list, double noData );

protected:

    // Pure virtual method for the model to create it's input sample, the 
    // derived models are the actual plugins and they need to implement this particular
    // method... 
    virtual int makeSample( double *sample, const OPAQ::Station& st, const OPAQ::Pollutant& pol,
    		    OPAQ::Aggregation::Type aggr, const OPAQ::DateTime &baseTime,
				const OPAQ::DateTime &fcTime, const OPAQ::TimeInterval &fc_hor ) = 0;
    
    /**
     * virtual method which returns the name of the ffnet file from a given pattern
     */
    virtual std::string getFFNetFile( const std::string &pol_name, Aggregation::Type aggr,
			const std::string &st_name, int fc_hor );
    
    virtual int getMissingValue( void ) = 0;

    void   printPar( std::string title, const std::vector<double> &x );

    std::string pattern;      //! needs to be set by the daughter class configure method
    unsigned int sample_size; //! needs to be set by daughter class

  private:    
    LOGGER_DEC();
  };
  
  
} /* namespace OPAQ */
#endif /* MLP_FEEDFORWARDMODEL_H_ */
