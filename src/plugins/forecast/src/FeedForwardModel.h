/*
 * FeedForwardModel.h
 *
 *  Created on: 2014
 *      Author: bino.maiheu@vito.be
 */

#ifndef FEEDFORWARDFORECASTMODEL_H_
#define FEEDFORWARDFORECASTMODEL_H_

#include <string>

#include <opaq.h>

#include "Tune.h"

namespace OPAQ {
  
  /**
   * Implementation of an OPAQ feedforaward neural network forecast model
   */
  class FeedForwardModel: public OPAQ::Model {
  public:
    FeedForwardModel();
    virtual ~FeedForwardModel();
    
    // OPAQ::Component methods
    virtual void configure (TiXmlElement * configuration) 
      throw (OPAQ::BadConfigurationException);


    // OPAQ::Model methods
    virtual void run();
    
private:
    std::string pattern;     //!< feed forward network file pattern
    int         max_fc_hor;  //!< the maximum forecast horizon in days (e.g. day+"2")
    int         mor_agg;     //!< morning aggregation hour

    std::string   _tuneSel;  //!< string indicating what tune the user wants to run
    TiXmlElement *_tuneList; //!< xml element containing the tune lists available

    // include a map for a tune configuration per station
    std::map<OPAQ::Station*,OVL::Tune> _tuneConfig; 

    double mean_missing( std::vector<double> & list, double noData );
    // double max_missing( std::vector<double> & list, double noData );
    // double min_missing( std::vector<double> & list, double noData );
    
    /**
     * Finds the tune in the list and sets the tune for the pollutant given
     * to the appropriate value
     */
    void _selectTune( const Pollutant & pol ) throw( BadConfigurationException );

    LOGGER_DEC();
    
  };


} /* namespace OPAQ */
#endif /* FEEDFORWARDMODEL_H_ */
