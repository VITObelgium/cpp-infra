#include "infra/database.h"

namespace inf::db {

SerializerContext::SerializerContext(const SerializerContext& rhs)
{
    _os << rhs._os.str();
}

std::string SerializerContext::str() const
{
    return _os.str();
}

void SerializerContext::reset()
{
    _os.str("");
}

std::string SerializerContext::escape(std::string arg)
{
    return sqlpp::serializer_context_t::escape(arg);
}

#ifdef INFRA_DB_SQLITE_SUPPORT
sqlpp::sqlite3::connection_config create_sqlite_connection_config(const fs::path& filename, inf::db::AccessMode access, ConnectionDebug debug)
{
    sqlpp::sqlite3::connection_config config;
    config.path_to_database = filename.string();

    if (access == db::AccessMode::ReadOnly) {
        config.flags = SQLITE_OPEN_READONLY;
    } else if (access == db::AccessMode::ReadWrite) {
        config.flags = SQLITE_OPEN_READWRITE;
    } else if (access == db::AccessMode::Create || access == db::AccessMode::Recreate) {
        config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    }

    config.debug = debug == ConnectionDebug::Yes;
    return config;
}
#endif

#ifdef INFRA_DB_POSTGRES_SUPPORT
std::shared_ptr<sqlpp::postgresql::connection_config> create_postgres_connection_config(const PostgresOptions& options, ConnectionDebug debug)
{
    auto config             = std::make_shared<sqlpp::postgresql::connection_config>();
    config->host            = options.host;
    config->user            = options.user;
    config->password        = options.pass;
    config->dbname          = options.databaseName;
    config->port            = options.port;
    config->connect_timeout = options.connectionTimeout;
    config->debug           = debug == ConnectionDebug::Yes;

#ifdef DEBUG_QUERIES
    config->debug = true;
#endif

    return config;
}

#endif
}
