/*
 * Tune.h
 *
 *  Created on: 2014
 *      Author: bino.maiheu@vito.be
 */

#ifndef TUNE_H_
#define TUNE_H_

#include <string>

namespace OVL {

  /**
   * A class to store the configuration of a station tune
   */
  class Tune {
  public:
    Tune();
    ~Tune();

    void  setFeaturesName( std::string f ){ _features = f; }
    const std::string & getFeaturesName(){ return _features; }

  private:
    std::string _features;
  };


}

#endif /* #ifndef TUNE_H_ */
