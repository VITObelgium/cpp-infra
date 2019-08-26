#include "infra/databaselog.h"

#include "infra/log.h"

#ifdef INFRA_DB_SQLITE_SUPPORT
#include <sqlite3.h>
#endif

namespace inf::db {

static void sqliteErrorLogCallback(void*, int errCode, const char* msg)
{
    Log::warn("SQLite database warning ({}) {}", errCode, msg);
}

void set_log_handler()
{
#ifdef INFRA_DB_SQLITE_SUPPORT
    sqlite3_config(SQLITE_CONFIG_LOG, sqliteErrorLogCallback, nullptr);
#endif
}

}
