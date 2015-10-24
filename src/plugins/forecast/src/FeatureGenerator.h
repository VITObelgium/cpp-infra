#ifndef FEATUREGENERATOR_H
#define FEATUREGENERATOR_H

namespace OVL {

  /**
   * Abstract base class for generating feature vectors corresponding to different
   * OVL models
   */
  class FeatureGenerator {

  public:
    FeatureGenerator( OPAQ::DataProvider *met, OPAQ::DataProvider *obs ) : _meteo(met), _obs(obs) {
    }

    virtual ~FeatureGenerator(){
    }

    /**
     * Pure virtual function which returns the feature sampel
     * The routine does not check the size ofthe sample, so it is up to the
     * user the be sure that the input size is large enough...
     */ 
    virtual int makeSample( double *sample, OPAQ::Station *st, OPAQ::Pollutant *pol, 
			    const OPAQ::DateTime & baseTime, const OPAQ::ForecastHorizon & fc_hor ) = 0;

    int size(){ return _size; }

    /** Set the meteo provider */
    void setMeteo( OPAQ::DataProvider *m ) { _meteo = m; }

    /** Set the observation provider */
    void setObs( OPAQ::DataProvider *o ) { _obs = o; }

  protected:
    OPAQ::DataProvider *_meteo;
    OPAQ::DataProvider *_obs;

    int _size;
  };

}
#endif /* #ifndef FEATUREGENERATOR_H */
