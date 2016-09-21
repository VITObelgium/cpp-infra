#include "OpaqRun.h"
#include "../PollutantManager.h"

namespace OPAQ
{
namespace Config
{

OpaqRun::OpaqRun()
: logger("OPAQ::Config::OpaqRun")
, pollutantSet(false)
, aggregation(OPAQ::Aggregation::None)
, networkProvider(nullptr)
, gridProvider(nullptr)
{
}

}
}
