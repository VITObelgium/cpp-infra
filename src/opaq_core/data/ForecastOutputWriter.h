/*
 * ForecastOutputWriter.h
 *
 * Bino Maiheu
 */

#ifndef OPAQ_FORECASTOUTPUTWRITER_H_
#define OPAQ_FORECASTOUTPUTWRITER_H_

#include <string>

#include <Component.h>
#include "ForecastBuffer.h"
#include "../AQNetworkProvider.h"
#include "../DateTime.h"


namespace OPAQ {

  class ForecastOutputWriter : public OPAQ::Component {

  public:
    ForecastOutputWriter();
    virtual  ~ForecastOutputWriter();

    virtual void write( Pollutant *pol, const DateTime &baseTime ) = 0;

    // some setters
    void setAQNetworkProvider( AQNetworkProvider *n ) { _net = n; }
    void setBuffer( ForecastBuffer *buf ) { _buf = buf; }
    void setForecastHorizon( OPAQ::TimeInterval *fc ) { _fcHor = fc; } // TODO : should not be a pointer... clear this up !!
    void setModelNames( std::vector<std::string> *list ) { _modelNames = list; } // TODO : should not be a pointer... clearn this up !!


  protected:
    ForecastBuffer*           getBuffer( void ) { return _buf; }
    OPAQ::AQNetworkProvider*  getAQNetworkProvider( void ) { return _net; }
    OPAQ::TimeInterval*       getForecastHorizon( void ) { return _fcHor; }
    std::vector<std::string>* getModelNames( void ) { return _modelNames; }


  private:
    OPAQ::AQNetworkProvider  *_net;
    OPAQ::ForecastBuffer     *_buf;
    OPAQ::TimeInterval       *_fcHor;
    std::vector<std::string> *_modelNames;
  };

} // namespace OPAQ

#endif /* OPAQ_FORECASTOUTPUTWRITER_H_ */
