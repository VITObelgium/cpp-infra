#pragma once

namespace inf::gdal {

void set_log_handler();

}

#ifdef INFRA_LOG_ENABLED
#include "infra/log.h"

namespace inf::gdal {
void set_log_handler(Log::Level levelToUse);
}

#endif
