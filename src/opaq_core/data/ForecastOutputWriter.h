/*
 * ForecastOutputWriter.h
 *
 * Bino Maiheu
 */

#ifndef OPAQ_FORECASTOUTPUTWRITER_H_
#define OPAQ_FORECASTOUTPUTWRITER_H_

#include <string>

#include <opaq/pfw.h>
#include "DataBuffer.h"
#include "../AQNetworkProvider.h"
#include "../DateTime.h"
#include "../ForecastHorizon.h"


namespace OPAQ {

  class ForecastOutputWriter : public OPAQ::Component {

  public:
    ForecastOutputWriter();
    virtual  ~ForecastOutputWriter();

    virtual void write( Pollutant *pol, const DateTime &baseTime ) = 0;

    // some setters
    void setAQNetworkProvider( AQNetworkProvider *n ) { _net = n; }
    void setBuffer( DataBuffer *buf ) { _buf = buf; }
    void setForecastHorizon( OPAQ::ForecastHorizon *fc ) { _fcHor = fc; }
    void setModelNames( std::vector<std::string> *list ) { _modelNames = list; }


  protected:
    DataBuffer*               getBuffer( void ) { return _buf; }
    OPAQ::AQNetworkProvider*  getAQNetworkProvider( void ) { return _net; }
    OPAQ::ForecastHorizon*    getForecastHorizon( void ) { return _fcHor; }
    std::vector<std::string>* getModelNames( void ) { return _modelNames; }


  private:
    OPAQ::AQNetworkProvider  *_net;
    OPAQ::DataBuffer         *_buf;
    OPAQ::ForecastHorizon    *_fcHor;
    std::vector<std::string> *_modelNames;
  };

} // namespace OPAQ

#endif /* OPAQ_FORECASTOUTPUTWRITER_H_ */
