#pragma once

#include "uiinfra/polygonio.h"

#include <qabstractitemmodel.h>
#include <qcolor.h>
#include <qgeopath.h>
#include <unordered_map>
#include <vector>

namespace inf::ui {

struct PolygonData
{
    QString name;
    QString displayName;
    double lineWidth = 2.0;
    QColor color     = Qt::black;
    std::vector<QGeoPath> geometry;
    std::vector<int64_t> ids;
};

class PolygonModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        PathRole = Qt::UserRole + 1,
        DisplayNameRole,
        NameRole,
        LineColorRole,
        LineWidthRole
    };

    PolygonModel(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setPolygonData(std::shared_ptr<std::vector<PolygonData>> data);
    void setVisibleData(std::vector<QString> names);
    std::vector<QString> visibleData() const;

    void clear();

private:
    void updateVisibleData();

    int _rowCount;
    int _colCount;
    std::shared_ptr<std::vector<PolygonData>> _data;
    std::vector<std::pair<const PolygonData*, const QGeoPath*>> _visibleData;
    std::vector<QString> _visibleNames;
};
}
