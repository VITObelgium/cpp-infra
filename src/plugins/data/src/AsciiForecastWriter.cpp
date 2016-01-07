#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iterator>
#include <vector>
#include <string>

#include "AsciiForecastWriter.h"

namespace OPAQ {

  LOGGER_DEF(AsciiForecastWriter);

  const std::string AsciiForecastWriter::BASETIME_PLACEHOLDER    = "%basetime%";
  const std::string AsciiForecastWriter::POLLUTANT_PLACEHOLDER   = "%pol%";
  const std::string AsciiForecastWriter::AGGREGATION_PLACEHOLDER = "%aggr%";


  AsciiForecastWriter::AsciiForecastWriter() :
	  _enable_fields(false) {
  }

  AsciiForecastWriter::~AsciiForecastWriter(){
  }

  
  void AsciiForecastWriter::configure(TiXmlElement * configuration)
    throw (BadConfigurationException) {
    
    if ( ! configuration )
      throw NullPointerException("No XML config element give");
    
    // parse filename
    TiXmlElement * fileEl = configuration->FirstChildElement("filename");
    if (!fileEl)
      throw BadConfigurationException("filename element not found");
    _filename = fileEl->GetText();
    

    // -- look for list of models to output, all will be dumped if no list given...
    try {
    	std::string model_list = XmlTools::getText( configuration, "models" );
    	_models = StringTools::tokenize( model_list, ",;:|/ \t", 7 );
    } catch ( ... ) {
    	_models.clear();
    }


    // -- look for the title & header, omit if not present
    try { _title  = XmlTools::getText( configuration, "title" ); } catch ( ... ) { _title.clear();  }
    try { _header = XmlTools::getText( configuration, "header" ); } catch ( ... ) { _header.clear();  }
	// get output mode
	try {
		std::string s = XmlTools::getText( configuration, "fields");
		std::transform( s.begin(), s.end(), s.begin(), ::tolower );
		if ( !s.compare( "enable" ) || !s.compare( "true" ) || !s.compare( "yes" ) ) _enable_fields = true;
	} catch ( ... ) {
		_enable_fields = false;
	}


    return;
  }


  void AsciiForecastWriter::write( OPAQ::Pollutant *pol, OPAQ::Aggregation::Type aggr,
					     	 	 	 	 	 const OPAQ::DateTime &baseTime ) {

    std::string fname = _filename; 
    std::string head  = _header;

    if ( ! getBuffer() )            throw RunTimeException( "No databuffer set" );
    if ( ! getAQNetworkProvider() ) throw RunTimeException( "No AQ network set" );

    // -- get network & stations & maximum forecast horizon
    OPAQ::AQNetwork *net            = getAQNetworkProvider()->getAQNetwork();
    std::vector<Station *> stations = net->getStations();
    int fcHorMax                    = getForecastHorizon().getDays();

    // -- translate the filename
    StringTools::replaceAll(fname, POLLUTANT_PLACEHOLDER, pol->getName() );
    StringTools::replaceAll(fname, AGGREGATION_PLACEHOLDER, OPAQ::Aggregation::getName( aggr ) );
    StringTools::replaceAll(fname, BASETIME_PLACEHOLDER, baseTime.dateToString() );

    // -- translate the header
    StringTools::replaceAll(head, POLLUTANT_PLACEHOLDER, pol->getName() );
    StringTools::replaceAll(head, AGGREGATION_PLACEHOLDER, OPAQ::Aggregation::getName( aggr ) );
    StringTools::replaceAll(head, BASETIME_PLACEHOLDER, baseTime.dateToString() );

    // ========================================================================
    // initialization
    // ========================================================================
    logger->info( "writing output file " + fname );

    FILE *fp = fopen( fname.c_str(), "w" );
    if ( ! fp ) throw RunTimeException( "Unable to open output file " + fname );
    
    // -- print header
    if ( _title.size() != 0 ) fprintf( fp, "# %s\n", _title.c_str() );
    if ( head.size() != 0 ) fprintf( fp, "# %s\n", head.c_str() );
    if ( _enable_fields ) fprintf( fp, "BASETIME\tSTATION\tFCTIME" );

    // -- get the results for the models for this baseTime/fcTime combination
    std::vector<std::string> modelNames = getBuffer()->getModelNames( pol->getName(), aggr );

    // -- determine the indices of the requested models
    std::vector<unsigned int> idx;
    for (auto m : _models ) {
    	unsigned int i = std::find( modelNames.begin(), modelNames.end(), m ) - modelNames.begin(); // look up index of this model in the list of available models
    	if ( i < modelNames.size() ) idx.push_back(i);
    }
    // -- no index defined, dump all models
    if ( idx.size() == 0 ) {
    	for ( unsigned int i = 0 ; i < modelNames.size(); i++ ) idx.push_back(i);
    }

    // -- if we have to write the fields
    if ( _enable_fields ) {
    	for ( auto ii : idx ) fprintf( fp, "\t%s", modelNames[ii].c_str() );
    	fprintf( fp, "\n" );
    	fflush( fp );
    }

    // ========================================================================
    // loop over stations and produce the output
    // ========================================================================
    for ( Station *station : stations ) {

    	// loop over the different forecast horizons
    	for ( int fc_hor=0; fc_hor <= fcHorMax; fc_hor++ ) {

    		OPAQ::TimeInterval fcHor( fc_hor, TimeInterval::Days );
    		OPAQ::DateTime     fcTime = baseTime + fcHor;

    		fprintf( fp, "%s\t%s\t%s", baseTime.dateToString().c_str(),
    				station->getName().c_str(), fcTime.dateToString().c_str() );
	
    		try {
    			std::vector<double> modelVals = getBuffer()->getModelValues( baseTime, fcHor, station->getName(), pol->getName(), aggr );
    			if ( modelVals.size() != modelNames.size() )
    				throw RunTimeException( "data size doesn't match the number of models..." );

    			for ( auto ii : idx ) fprintf( fp, "\t%.6f", modelVals[ii] );
    			fprintf( fp, "\n" );

    		} catch ( OPAQ::NotAvailableException & err ) {
				for ( auto ii : idx ) fprintf( fp, "\t%f", getBuffer()->getNoData() );
				fprintf( fp, "\n" );
				continue;
    		}

    	} // loop over forecast horizons

    } // loop over stations


    fclose(fp);

    return;
  }


} // namespace

OPAQ_REGISTER_PLUGIN(OPAQ::AsciiForecastWriter);
