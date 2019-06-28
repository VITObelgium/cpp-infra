#pragma once

#include "infra/datatypes.h"
#include "infra/log.h"

#include <optional>
#include <sqlpp11/sqlpp11.h>

#ifdef INFRA_DB_SQLITE_SUPPORT
#include "infra/filesystem.h"
#include <sqlpp11/sqlite3/connection.h>
#endif

namespace inf::db {

enum class ConnectionDebug
{
    Yes,
    No,
};

enum class AccessMode
{
    ReadOnly,
    ReadWrite,
    Create
};

template <typename DbConnectionType>
class Transaction
{
public:
    Transaction(Transaction&&)      = default;
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    Transaction(DbConnectionType& db)
    : _transaction(start_transaction(db))
    {
    }

    void commit()
    {
        _transaction.commit();
    }

    void rollback()
    {
        _transaction.rollback();
    }

private:
    sqlpp::transaction_t<DbConnectionType> _transaction;
};

class AbstractDatabase
{
public:
    virtual ~AbstractDatabase()
    {
    }

    virtual std::vector<inf::ConfigParameter> get_config_parameters(std::string_view tableName) = 0;
    virtual void set_config_parameter(std::string_view tableName, const ConfigParameter& param) = 0;
};

template <typename ResultField>
auto optional_record_value(ResultField&& value)
{
    using ValueType = decltype(value.value());

    if (value.is_null()) {
        return std::optional<ValueType>();
    }

    return std::make_optional<ValueType>(value);
}

inline auto optional_field_value(const std::optional<double>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value());
    } else {
        return sqlpp::value_or_null<sqlpp::floating_point>(sqlpp::null);
    }
}

inline auto optional_field_value(const std::optional<int64_t>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value());
    } else {
        return sqlpp::value_or_null<sqlpp::integer>(sqlpp::null);
    }
}

struct SerializerContext
{
    std::ostringstream _os;

    SerializerContext() = default;
    SerializerContext(const SerializerContext& rhs);

    std::string str() const;
    void reset();

    template <typename T>
    std::ostream& operator<<(T t)
    {
        return _os << t;
    }

    static std::string escape(std::string arg);
};

template <typename Query>
void log_query(const Query& query)
{
    SerializerContext context;
    Log::info(serialize(query, context).str());
}

#ifdef INFRA_DB_SQLITE_SUPPORT
sqlpp::sqlite3::connection_config create_sqlite_connection_config(const fs::path& filename, inf::db::AccessMode access, ConnectionDebug debug);
#endif
}
