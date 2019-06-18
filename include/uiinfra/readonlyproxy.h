#pragma once

#include "qidentityproxymodel.h"

namespace inf::ui {

/* Proxy model that makes sure the model is read-only
 * Usefull when dealing with sqltablemodels that lock the database otherwise
 */
class ReadOnlyProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ReadOnlyProxyModel)
public:
    explicit ReadOnlyProxyModel(QObject* parent = nullptr)
    : QIdentityProxyModel(parent)
    {
    }

    virtual ~ReadOnlyProxyModel() = default;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        return QIdentityProxyModel::flags(index) & (~Qt::ItemIsEditable);
    }

    virtual bool setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/ = Qt::EditRole) override
    {
        return false;
    }
};
}
