#ifndef OVL_MODEL_1_7CST_
#define OVL_MODEL_1_7CST_

namespace OVL {

  class model_1_7CST : public FeatureGenerator {
  public:

    model_1_7CST( OPAQ::DataProvider *met, OPAQ::DataProvider *obs ) :
      FeatureGenerator( met, obs ) {

      // set the size of the feature vector
      _size = 2;
    }

    ~model_1_7CST(){
    }

    // implementation of the make sample routine
    int makeSample( double *sample, OPAQ::Station *st, OPAQ::Pollutant *pol, 
		    const OPAQ::DateTime & baseTime, const OPAQ::ForecastHorizon & fc_hor ) {
      // available for use : _meteo, _obs
      
      

      for( int i=0; i<_size; ++i ) sample[i] = 10*i;

      return 0;
    } /* makeSample */
    
  };

}

#endif /* #ifndef OVL_MODEL_1_7CST_ */
