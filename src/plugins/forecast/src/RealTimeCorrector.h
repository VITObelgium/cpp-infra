#ifndef REALTIMECORRECTOR_H__
#define REALTIMECORRECTOR_H__

#include <tinyxml.h>

namespace OPAQ {

  class RealTimeCorrector {
  public:
    RealTimeCorrector( TiXmlElement *cnf );
    virtual ~RealTimeCorrector();

    
  };


}

#endif /* REALTIMECORRECTOR_H__ */
