#pragma once

#include "infra/cast.h"
#include "infra/chrono.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/string.h"

#include <optional>
#include <sqlpp11/sqlpp11.h>

#ifdef INFRA_DB_SQLITE_SUPPORT
#include "infra/filesystem.h"
#include <sqlpp11/sqlite3/connection.h>
#endif

#ifdef INFRA_DB_POSTGRES_SUPPORT
#include <sqlpp11/postgresql/postgresql.h>
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
    Create,
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

template <typename ResultField>
bool bool_value(ResultField&& value)
{
    static_assert(std::is_same_v<int64_t, decltype(value.value())>);
    return static_cast<int64_t>(value) == 1;
}

template <typename ResultField>
auto optional_record_value(ResultField&& value)
{
    using ValueType = decltype(value.value());

    if (value.is_null()) {
        return std::optional<ValueType>();
    }

    return std::make_optional<ValueType>(value);
}

template <typename ResultType, typename ResultField>
auto optional_record_value_as(ResultField&& value)
{
    using ValueType = decltype(value.value());

    if (value.is_null()) {
        return std::optional<ResultType>();
    }

    return optional_cast<ResultType>(std::make_optional<ValueType>(value));
}

template <typename ResultField>
char character_record_value(ResultField&& value)
{
    static_assert(std::is_same_v<std::string, decltype(value.value())>, "TEXT record value required");

    if (value.is_null() || value.value().empty()) {
        return '\0';
    }

    if (value.value().size() > 1) {
        throw RuntimeError("Record value is not a character: {}", value.value());
    }

    return value.value().front();
}

inline auto tvin(int64_t val)
{
    if (val == 0) {
        return sqlpp::value_or_null<sqlpp::integer>(sqlpp::null);
    } else {
        return sqlpp::value_or_null(val);
    }
}

inline auto tvin(const std::string& val)
{
    if (val.empty()) {
        return sqlpp::value_or_null<sqlpp::text>(sqlpp::null);
    } else {
        return sqlpp::value_or_null(val);
    }
}

inline auto tvin(const std::string_view val)
{
    if (val.empty()) {
        return sqlpp::value_or_null<sqlpp::text>(sqlpp::null);
    } else {
        return sqlpp::value_or_null(std::string(val));
    }
}

inline auto optional_insert_value(const std::optional<double>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value());
    } else {
        return sqlpp::value_or_null<sqlpp::floating_point>(sqlpp::null);
    }
}

inline auto optional_insert_value(const std::optional<int64_t>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value());
    } else {
        return sqlpp::value_or_null<sqlpp::integral>(sqlpp::null);
    }
}

inline auto optional_insert_value(const std::optional<bool>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value() ? 1 : 0);
    } else {
        return sqlpp::value_or_null<sqlpp::integer>(sqlpp::null);
    }
}

inline auto optional_insert_value(const std::optional<chrono::date_point>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value());
    } else {
        return sqlpp::value_or_null<sqlpp::date>(sqlpp::null);
    }
}

inline auto optional_insert_value(const std::optional<chrono::time_point>& opt)
{
    if (opt.has_value()) {
        return sqlpp::value_or_null(opt.value());
    } else {
        return sqlpp::value_or_null<sqlpp::time_point>(sqlpp::null);
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

    // static std::string escape(std::string arg);
};

//! sqlContents should contain ; seperated list of sql statements
template <typename Connection>
void execute_sql_statements(Connection& conn, std::string_view sqlContents)
{
    auto lines = str::split_view(sqlContents, ';', str::SplitOpt::NoEmpty | str::SplitOpt::Trim);
    for (auto& line : lines) {
        if (!str::starts_with(line, "--")) {
            conn.execute(std::string(line));
        }
    }
}

template <typename Connection>
void execute_sql_file(Connection& conn, const fs::path& sqlPath)
{
    std::string fileContents = file::read_as_text(sqlPath);
    execute_sql_statements(conn, fileContents);
}

template <typename Query>
std::string query_to_string(const Query& query)
{
    SerializerContext context;
    return serialize(query, context).str();
}

#ifdef INFRA_DB_SQLITE_SUPPORT
sqlpp::sqlite3::connection_config create_sqlite_connection_config(const fs::path& filename, inf::db::AccessMode access, ConnectionDebug debug);
#endif

#ifdef INFRA_DB_POSTGRES_SUPPORT
struct PostgresOptions
{
    std::string databaseName;
    std::string user;
    std::string pass;
    std::string host;
    std::string applicationName;
    uint32_t port              = 5432;
    uint32_t connectionTimeout = 0;
};

std::shared_ptr<sqlpp::postgresql::connection_config> create_postgres_connection_config(const PostgresOptions& options, ConnectionDebug debug);
#endif

}
