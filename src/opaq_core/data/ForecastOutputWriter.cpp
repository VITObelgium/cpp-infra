#include "ForecastOutputWriter.h"

namespace OPAQ {

ForecastOutputWriter::ForecastOutputWriter(){
	_net        = NULL;
	_fcHor      = NULL;
	_modelNames = NULL;
	_buf        = NULL;
}

ForecastOutputWriter::~ForecastOutputWriter(){
}

} // namespace OPAQ
