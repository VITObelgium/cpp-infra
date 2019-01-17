#pragma once

#include "infra/legend.h"

#include <qabstractitemmodel.h>
#include <qcolor.h>
#include <vector>

namespace opaq {

struct PointSourceModelData
{
    QString name;
    float value      = 0.f;
    double latitude  = 0.0;
    double longitude = 0.0;
};

class PointSourceModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum LocationRoles
    {
        NameRole = Qt::UserRole + 1,
        ValueRole,
        LatitudeRole,
        LongitudeRole,
        LambertX,
        LambertY,
        ColorRole,
    };

    PointSourceModel(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setLegend(const inf::Legend& legend);
    void setModelData(std::vector<PointSourceModelData> data);
    void clear();

private:
    QColor colorValue(float value) const;

    std::vector<PointSourceModelData> _data;
    inf::Legend _legend;
    float _maxValue = 0.f;
    QColor _minColor;
    QColor _maxColor;
};
}
