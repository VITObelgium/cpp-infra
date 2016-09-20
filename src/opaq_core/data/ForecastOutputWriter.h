/*
 * ForecastOutputWriter.h
 *
 * Bino Maiheu
 */

#ifndef OPAQ_FORECASTOUTPUTWRITER_H_
#define OPAQ_FORECASTOUTPUTWRITER_H_

#include <string>

#include "ForecastBuffer.h"
#include "../Component.h"
#include "../AQNetworkProvider.h"
#include "../DateTime.h"
#include "../Aggregation.h"


namespace OPAQ {

  class ForecastOutputWriter : public OPAQ::Component {

  public:
    ForecastOutputWriter();
    virtual  ~ForecastOutputWriter();

    virtual void write( OPAQ::Pollutant *pol, OPAQ::Aggregation::Type aggr, const DateTime &baseTime ) = 0;

    // some setters
    void setAQNetworkProvider( AQNetworkProvider *n ) { _net = n; }
    void setBuffer( ForecastBuffer *buf ) { _buf = buf; }
    void setForecastHorizon( const OPAQ::TimeInterval& fc ) { _fcHor = fc; }

  protected:
    ForecastBuffer*           getBuffer( void ) { return _buf; }
    OPAQ::AQNetworkProvider*  getAQNetworkProvider( void ) { return _net; }
    const OPAQ::TimeInterval& getForecastHorizon( void ) { return _fcHor; }

  private:
    OPAQ::AQNetworkProvider  *_net;
    OPAQ::ForecastBuffer     *_buf;
    OPAQ::TimeInterval        _fcHor;
  };

} // namespace OPAQ

#endif /* OPAQ_FORECASTOUTPUTWRITER_H_ */
