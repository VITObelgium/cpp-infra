#include <stdio.h>
#include <stdlib.h>

#include "SimpleAscForecastOutputWriter.h"

namespace OPAQ {

  LOGGER_DEF(SimpleAscForecastOutputWriter);


  const std::string SimpleAscForecastOutputWriter::BASETIME_PLACEHOLDER = "%BASETIME%";
  const std::string SimpleAscForecastOutputWriter::POLLUTANT_PLACEHOLDER = "%POL%";

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
					     const OPAQ::DateTime &baseTime ) {

    std::string fname = _filename; 

    if ( ! getBuffer() )            throw RunTimeException( "No databuffer set" );
    if ( ! getModelNames() )        throw RunTimeException( "No model list available" ); 
    if ( ! getForecastHorizon() )   throw RunTimeException( "Forecast horizon not set" );
    if ( ! getAQNetworkProvider() ) throw RunTimeException("No AQ network set");

    // translate the filename
    StringTools::replaceAll(fname, BASETIME_PLACEHOLDER, baseTime.dateToString() );
    StringTools::replaceAll(fname, POLLUTANT_PLACEHOLDER, pol->getName() );

    // TODO : what to do if file exists...
    logger->info( "writing output file " + fname );
    FILE *fp = fopen( fname.c_str(), "w" );
    if ( ! fp )throw RunTimeException( "Unable to open output file " + fname );
    
    // print header
    fprintf( fp, "# OPAQ Forecast\n" );
    fprintf( fp, "# Pollutant: %s\n", pol->getName().c_str() );
    fprintf( fp, "# BASETIME STATION FCTIME" );

    std::vector<std::string>::iterator modelIt = getModelNames()->begin(); 
    while ( modelIt != getModelNames()->end() ) {
      std::string modelName = *modelIt++;
      fprintf( fp, " %s", modelName.c_str() );
    }
    fprintf( fp, "\n" );
    fflush( fp );

    OPAQ::AQNetwork *net = getAQNetworkProvider()->getAQNetwork();
    std::vector<Station *> stations = net->getStations();
    
    // set the basetime in the buffer
    getBuffer()->setBaseTime( baseTime );

    // loop over stations
    int fcHorMax = getForecastHorizon()->getDays();
    std::vector<Station *>::iterator stationIt = stations.begin();
    while( stationIt != stations.end() ) {
      Station *station = *stationIt++;

      // loop over the different forecast horizons
      for ( int fc_hor=0; fc_hor <= fcHorMax; fc_hor++ ) {
	OPAQ::ForecastHorizon fcHor(fc_hor*24);
	OPAQ::DateTime fcTime = baseTime + fcHor; 

	fprintf( fp, "%s %s %s", baseTime.dateToString().c_str(), 
		 station->getName().c_str(), fcTime.dateToString().c_str() );
	
	// loop over the different models
	std::vector<std::string>::iterator modelIt = getModelNames()->begin(); 
	while ( modelIt != getModelNames()->end() ) {
	  std::string modelName = *modelIt++;
	  std::vector<double> val;

	  try {
	    val = getBuffer()->getValues( modelName, TimeInterval(0), TimeInterval(0),
							      pol->getName(), station->getName(), fcHor ); 
	  } catch ( OPAQ::NotAvailableException & err ) {
	    fprintf( fp, " %f", getBuffer()->getNoData() );
	    continue;
	  }

	  // expecting one value...
	  if ( val.size() != 1 ) {
	    throw RunTimeException( "Hmm, more than one value returned... " );
	  } else {
	    fprintf( fp, " %f", val[0] );
	  }

	} /* while ( modelIt != getModelNames()->end() ) */

	fprintf( fp, "\n" );
	
      }
      

    } 
    
    fclose(fp);

    return;
  }


} // namespace

OPAQ_REGISTER_PLUGIN(OPAQ::SimpleAscForecastOutputWriter);
