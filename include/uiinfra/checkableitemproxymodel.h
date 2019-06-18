#pragma once

#include <qhash.h>
#include <qidentityproxymodel.h>

namespace inf::ui {

/*! Proxy model that make sure sure items are user checkable
 * and display a checkbox
 */
class CheckableItemProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    CheckableItemProxyModel(QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

private:
    QHash<QModelIndex, Qt::CheckState> _checkStates;
};
}
