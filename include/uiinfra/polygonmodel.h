#pragma once

#include "uiinfra/polygonio.h"

#include <qabstractitemmodel.h>
#include <qgeopath.h>
#include <unordered_map>
#include <vector>

namespace uiinfra {

class PolygonModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        PathRole = Qt::UserRole + 1,
        NameRole,
        ColorRole,
        LineStyleRole
    };

    PolygonModel(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setGeoData(std::shared_ptr<OverlayMap> data);
    void setVisibleData(std::vector<QString> names);
    std::vector<QString> visibleData() const;

    void clear();

private:
    void updateVisibleData();

    int _rowCount;
    int _colCount;
    std::shared_ptr<OverlayMap> _data;
    std::vector<const QGeoPath*> _visibleData;
    std::vector<QString> _visibleNames;
};
}
