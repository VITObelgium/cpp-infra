/*
 * FeedForwardModel.h
 *
 *  Created on: 2014
 *      Author: bino.maiheu@vito.be
 */

#ifndef MLP_FEEDFORWARDFORECASTMODEL_H_
#define MLP_FEEDFORWARDFORECASTMODEL_H_

#include "Model.h"

#include <string>

namespace opaq {

class Station;

/**
 * Implementation of an OPAQ multi layer perceptron feed forward neural
 * network forecast model. Usable feedforward model derive from this and
 * implement their own sample creating routine. This class is an abstract
 * base class for the actuall plugins.
 */
class MLP_FeedForwardModel : public Model
{
public:
    MLP_FeedForwardModel();

    // the configure method should also be implemented in the derived class...
    // OPAQ::Model methods --> run for this particular fcTime...
    virtual void run();

    /**
     *  Workhorse routine which acutally performs the forecast and returns the
     *  value
     */
    double fcValue(const Pollutant& pol, const Station& station,
        Aggregation::Type aggr, const chrono::date_time& baseTime,
        chrono::days fc_hor);

    // Some helper routines, make them public
    static double mean_missing(const std::vector<double>& list, double noData);
    static double max_missing(const std::vector<double>& list, double noData);
    static double min_missing(const std::vector<double>& list, double noData);

protected:
    // Pure virtual method for the model to create it's input sample, the
    // derived models are the actual plugins and they need to implement this particular
    // method...
    virtual int makeSample(double* sample, const Station& st, const Pollutant& pol,
        Aggregation::Type aggr, const chrono::date_time& baseTime,
        const chrono::date_time& fcTime, chrono::days fc_hor) = 0;

    /**
     * virtual method which returns the name of the ffnet file from a given pattern
     */
    virtual std::string getFFNetFile(const std::string& pol_name, Aggregation::Type aggr,
        const std::string& st_name, int fc_hor);

    void printPar(const std::string&, const std::vector<double>& x);

    std::string pattern;      //! needs to be set by the daughter class configure method
    unsigned int sample_size; //! needs to be set by daughter class
};

} /* namespace OPAQ */
#endif /* MLP_FEEDFORWARDMODEL_H_ */
