#include "infra/gdallog.h"

#ifdef INFRA_LOG_ENABLED
#include "infra/log.h"

#include <cpl_error.h>
#include <cpl_port.h>

namespace inf::gdal {

static void gdalErrorHandler(CPLErr errClass, int /*err_no*/, const char* msg)
{
    switch (errClass) {
    case CE_Debug:
        Log::debug("GDAL {}", std::string(msg));
        break;
    case CE_Warning:
        Log::debug("GDAL {}", std::string(msg));
        break;
    case CE_Failure:
        Log::error("GDAL {}", std::string(msg));
        break;
    case CE_Fatal:
        Log::critical("GDAL {}", std::string(msg));
        break;
    default:
        break;
    }
}

void set_log_handler()
{
    CPLSetErrorHandler(&gdalErrorHandler);
}
}
#else
namespace inf::gdal {
void set_log_handler()
{
}
}
#endif
