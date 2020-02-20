#include "infra/databaselog.h"

#ifdef INFRA_LOG_ENABLED
#include "infra/log.h"
#endif

#ifdef INFRA_DB_SQLITE_SUPPORT
#include <sqlite3.h>
#endif

namespace inf::db {

static void sqliteErrorLogCallback(void*, int errCode, const char* msg)
{
#ifdef INFRA_LOG_ENABLED
    Log::warn("SQLite database warning ({}) {}", errCode, msg);
#endif
}

void set_log_handler()
{
#ifdef INFRA_DB_SQLITE_SUPPORT
    sqlite3_config(SQLITE_CONFIG_LOG, sqliteErrorLogCallback, nullptr);
#endif
}

}
