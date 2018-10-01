#include "PredictionDatabase.h"

namespace opaq {

//#define DEBUG_QUERIES

template <typename ConnectionType>
void PredictionDatabase<ConnectionType>::addPredictions(chrono::date_time baseTime,
    const std::string& model,
    const std::string& stationId,
    const std::string& pollutantId,
    const std::string& aggr,
    chrono::days fcHor,
    const TimeSeries<double>& forecast)

    template <typename ConnectionType>
    double PredictionDatabase<ConnectionType>::getPrediction(chrono::date_time date,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor)

        template <typename ConnectionType>
        TimeSeries<double> PredictionDatabase<ConnectionType>::getPredictions(const std::string& model,
            const std::string& stationId,
            const std::string& pollutantId,
            const std::string& aggr,
            chrono::days fcHor)

            template <typename ConnectionType>
            std::vector<double> PredictionDatabase<ConnectionType>::getPredictionValues(chrono::date_time basetime,
                const std::string& stationId,
                const std::string& pollutantId,
                const std::string& aggr,
                chrono::days fcHor)

                template <typename ConnectionType>
                TimeSeries<double> PredictionDatabase<ConnectionType>::getPredictions(chrono::date_time startDate,
                    chrono::date_time endDate,
                    const std::string& model,
                    const std::string& stationId,
                    const std::string& pollutantId,
                    const std::string& aggr,
                    chrono::days fcHor)

}
