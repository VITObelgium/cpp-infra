#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iterator>
#include <vector>
#include <string>

#include "SimpleAscForecastOutputWriter.h"

namespace OPAQ {

  LOGGER_DEF(SimpleAscForecastOutputWriter);


  const std::string SimpleAscForecastOutputWriter::BASETIME_PLACEHOLDER = "%basetime%";
  const std::string SimpleAscForecastOutputWriter::POLLUTANT_PLACEHOLDER = "%pol%";
  const std::string SimpleAscForecastOutputWriter::AGGREGATION_PLACEHOLDER = "%aggr%";


  SimpleAscForecastOutputWriter::SimpleAscForecastOutputWriter(){
  }

  SimpleAscForecastOutputWriter::~SimpleAscForecastOutputWriter(){
  }

  
  void SimpleAscForecastOutputWriter::configure(TiXmlElement * configuration)
    throw (BadConfigurationException) {
    
    if ( ! configuration )
      throw NullPointerException("No XML config element give");
    
    // parse filename
    TiXmlElement * fileEl = configuration->FirstChildElement("filename");
    if (!fileEl)
      throw BadConfigurationException("filename element not found");
    _filename = fileEl->GetText();
    
    return;
  }


  void SimpleAscForecastOutputWriter::write( OPAQ::Pollutant *pol,
		  	  	  	  	  	  	  	  	  	 OPAQ::Aggregation::Type aggr,
					     	 	 	 	 	 const OPAQ::DateTime &baseTime ) {

    std::string fname = _filename; 

    if ( ! getBuffer() )            throw RunTimeException( "No databuffer set" );
    if ( ! getAQNetworkProvider() ) throw RunTimeException( "No AQ network set" );

    // translate the filename
    StringTools::replaceAll(fname, POLLUTANT_PLACEHOLDER, pol->getName() );
    StringTools::replaceAll(fname, AGGREGATION_PLACEHOLDER, OPAQ::Aggregation::getName( aggr ) );
    StringTools::replaceAll(fname, BASETIME_PLACEHOLDER, baseTime.dateToString() );

    logger->info( "writing output file " + fname );
    FILE *fp = fopen( fname.c_str(), "w" );
    if ( ! fp ) throw RunTimeException( "Unable to open output file " + fname );
    
    // print header
    fprintf( fp, "# OPAQ Forecast\n" );
    fprintf( fp, "# Pollutant: %s\n", pol->getName().c_str() );
    fprintf( fp, "# BASETIME STATION FCTIME" );

    // get the results for the models for this baseTime/fcTime combination
    std::vector<std::string> modelNames = getBuffer()->getModelNames( pol->getName(), aggr );

    auto modelIt = modelNames.begin();
    while ( modelIt != modelNames.end() ) {
      std::string modelName = *modelIt++;
      fprintf( fp, " %s", modelName.c_str() );
    }
    fprintf( fp, "\n" );
    fflush( fp );

    OPAQ::AQNetwork *net = getAQNetworkProvider()->getAQNetwork();
    std::vector<Station *> stations = net->getStations();

    // loop over stations
    int fcHorMax = getForecastHorizon().getDays();
    std::vector<Station *>::iterator stationIt = stations.begin();
    while( stationIt != stations.end() ) {
    	Station *station = *stationIt++;

    	// loop over the different forecast horizons
    	for ( int fc_hor=0; fc_hor <= fcHorMax; fc_hor++ ) {
    		OPAQ::TimeInterval fcHor( fc_hor, TimeInterval::Days );
    		OPAQ::DateTime fcTime = baseTime + fcHor;

    		fprintf( fp, "%s\t%s\t%s", baseTime.dateToString().c_str(),
    				station->getName().c_str(), fcTime.dateToString().c_str() );
	
    		try {
    			std::vector<double> modelVals = getBuffer()->getModelValues( baseTime, fcHor, station->getName(), pol->getName(), aggr );
    			if ( modelVals.size() != modelNames.size() )
    				throw RunTimeException( "data size doesn't match the number of models..." );

    			for( auto it=modelVals.begin(); it != modelVals.end(); ++it ) fprintf( fp, "\t%.2f", *it );
    			fprintf( fp, "\n" );

    		} catch ( OPAQ::NotAvailableException & err ) {
				for ( unsigned int i = 0; i<modelNames.size(); i++ ) fprintf( fp, "\t%f", getBuffer()->getNoData() );
				fprintf( fp, "\n" );
				continue;
    		}
    	} // loop over forecast horizons
    } // loop over stations


    fclose(fp);

    return;
  }


} // namespace

OPAQ_REGISTER_PLUGIN(OPAQ::SimpleAscForecastOutputWriter);
