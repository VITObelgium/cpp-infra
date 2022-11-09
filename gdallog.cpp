#include "infra/gdallog.h"

#ifdef INFRA_LOG_ENABLED
#include "infra/cast.h"
#include "infra/enumutils.h"
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
        Log::debug("GDAL {}", msg);
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

static void gdalErrorHandlerLevelOverride(CPLErr /*errClass*/, int /*err_no*/, const char* msg)
{
    // read the log level from the pointer value
    const auto logLevel = Log::Level(truncate<enum_type_t<Log::Level>>(reinterpret_cast<unsigned long long>(CPLGetErrorHandlerUserData())));
    Log::log(logLevel, "GDAL {}", msg);
}

void set_log_handler()
{
    CPLSetErrorHandler(&gdalErrorHandler);
}

void set_log_handler(Log::Level levelToUse)
{
    CPLSetErrorHandlerEx(&gdalErrorHandlerLevelOverride, reinterpret_cast<void*>(levelToUse));
}

}
#else
namespace inf::gdal {
void set_log_handler()
{
}
}
#endif
