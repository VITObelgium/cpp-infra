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
    } else if (access == db::AccessMode::Create) {
        config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    }

    config.debug = debug == ConnectionDebug::Yes;
    return config;
}
#endif
}
