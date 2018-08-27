#include "infra/gdallog.h"
#include "infra/log.h"

#include <cpl_error.h>
#include <cpl_port.h>

namespace inf::gdal {

static void gdalErrorHandler(CPLErr errClass, int /*err_no*/, const char* msg)
{
    switch (errClass) {
    case CE_Debug:
        Log::debug("GDAL {}", msg);
        break;
    case CE_Warning:
        Log::warn("GDAL {}", msg);
        break;
    case CE_Failure:
        Log::error("GDAL {}", msg);
        break;
    case CE_Fatal:
        Log::critical("GDAL {}", msg);
        break;
    default:
        break;
    }
}

void setLogHandler()
{
    CPLSetErrorHandler(&gdalErrorHandler);
}
}
