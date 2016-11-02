#include "Model.h"

namespace opaq
{

Model::Model()
: aggregation(Aggregation::None)
, aqNetworkProvider(0)
, gridProvider(0)
, input(0)
, meteo(0)
, buffer(0)
, _missing_value(-9999)
{
}

}
