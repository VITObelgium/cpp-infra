#pragma once

#include <qsortfilterproxymodel.h>

namespace inf::ui {

//! The UniqueValuesProxyModel provides a filter model remove duplicate values from a source model.
class UniqueValuesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool emptyItemsAllowed READ emptyItemsAllowed WRITE setEmptyItemsAllowed)
    //! @property(modelColumn)
    /**
	 * This property holds the column in the model that is use for filtering.
	 * @sa int modelColumn() const
	 * @sa void setModelColumn(int)
	 */
    Q_PROPERTY(int modelColumn READ modelColumn WRITE setModelColumn)
public:
    UniqueValuesProxyModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role) const override;

    bool emptyItemsAllowed() const;
    /**
	 * Returns true if the item in the row indicated by the given source_row and source_parent contains a unique value in the model; otherwise returns false.
	 * @note By default, the Qt::DisplayRole is used to determine if the row should be accepted or not.
	 */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    /**
	 * Returns model column used to determine unique value.
	 */
    int modelColumn() const;
    void setEmptyItemsAllowed(bool on);
    /**
	 * Set the model column used to determine unique value.
	 */
    void setModelColumn(int colum);
    void setSourceModel(QAbstractItemModel* sourceModel) override;

private:
    bool isDuplicate(int row) const;
    void buildMap();
    void sourceModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

    bool _emptyValues;
    int _modelColumn;
    QMap<QString, QList<int>> _valueMap;
};
}
