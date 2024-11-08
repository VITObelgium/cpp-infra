#pragma once

#include <sqlpp11/transaction.h>

namespace inf::db {

template <typename DbConnectionType>
class Transaction
{
public:
    Transaction(Transaction&&)                 = default;
    Transaction(const Transaction&)            = delete;
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

}
