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
    
protected:
    // Pure virtual method for the model to create it's input sample, the 
    // derived models are the actual plugins and they need to implement this particular
    // method... 
    virtual int makeSample( double *sample, OPAQ::Station *st, OPAQ::Pollutant *pol, 
			    const OPAQ::DateTime &baseTime, 
			    const OPAQ::DateTime &fcTime, 
			    const OPAQ::TimeInterval &fc_hor ) = 0;
    
    virtual std::string getFFNetFile( const std::string &pol_name, 
				      const std::string &st_name, 
				      int fc_hor ) = 0;
    
    virtual int getMissingValue( void ) = 0;

    // Some helper routines...
    double mean_missing( const std::vector<double> & list, double noData );
    double max_missing( const std::vector<double> & list, double noData );
    double min_missing( const std::vector<double> & list, double noData );
    void   printPar( std::string title, const std::vector<double> &x );

  private:    
    LOGGER_DEC();
  };
  
  
} /* namespace OPAQ */
#endif /* MLP_FEEDFORWARDMODEL_H_ */
