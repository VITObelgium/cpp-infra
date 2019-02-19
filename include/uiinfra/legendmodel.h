#pragma once

#include "infra/legend.h"

#include <qabstractitemmodel.h>
#include <qcolor.h>

namespace inf::ui {

class LegendModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum LegendRoles
    {
        NameRole = Qt::UserRole + 1,
        ColorRole
    };

    LegendModel(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    inf::Legend legend() const;
    void setLegend(Legend loc);

    bool isNumeric() const;
    void setLegendType(Legend::Type type);

    bool treatZeroAsNodata() const;
    void setTreatZeroAsNodata(bool enabled);

    int numberOfClasses() const;
    void setNumberOfClasses(int count);

    QString legendTitle() const;

    void clear();

private:
    Legend _legend;
};
}
