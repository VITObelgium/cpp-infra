#include "infra/database.h"

namespace inf::db {

#ifdef INFRA_DB_SQLITE_SUPPORT
sqlpp::sqlite3::connection_config create_sqlite_connection_config(const fs::path& filename, inf::db::AccessMode access, bool debugQueries)
{
    sqlpp::sqlite3::connection_config config;
    config.path_to_database = filename.string();

    if (access == db::AccessMode::ReadOnly) {
        config.flags = SQLITE_OPEN_READONLY;
    } else if (access == db::AccessMode::ReadWrite) {
        config.flags = SQLITE_OPEN_READWRITE;
    } else if (access == db::AccessMode::Create) {
        config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    }

    config.debug = debugQueries;
    return config;
}
#endif

}
