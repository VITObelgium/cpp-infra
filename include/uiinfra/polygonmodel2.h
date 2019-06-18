#pragma once

#include "uiinfra/polygonio.h"

#include <qabstractitemmodel.h>
#include <qcolor.h>
#include <qgeopath.h>
#include <qgeorectangle.h>
#include <qpoint.h>
#include <qvector.h>
#include <unordered_map>
#include <vector>

namespace inf::ui {

struct PolygonData2
{
    QString name;
    QString displayName;
    double lineWidth = 2.0;
    QGeoRectangle extent;
    QColor color = Qt::black;
    QVector<QGeoPath> geometry;
};

class PolygonModel2 : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        PathRole = Qt::UserRole + 1,
        DisplayNameRole,
        NameRole,
        LineColorRole,
        LineWidthRole,
    };

    PolygonModel2(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setPolygonData(std::shared_ptr<std::vector<PolygonData2>> data);
    void setVisibleData(std::vector<QString> names);
    std::vector<QString> visibleData() const;

    void clear();

private:
    void updateVisibleData();

    int _rowCount;
    int _colCount;
    std::shared_ptr<std::vector<PolygonData2>> _data;
    std::vector<PolygonData2*> _visibleData;
    std::vector<QString> _visibleNames;
};
}
