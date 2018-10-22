#include "uiinfra/polygonmodel2.h"
#include "infra/algo.h"
#include "infra/cast.h"
#include "infra/log.h"
#include "infra/rect.h"

#include <qcolor.h>
#include <qgeolocation.h>
#include <qgeorectangle.h>

namespace uiinfra {

using namespace inf;

PolygonModel2::PolygonModel2(QObject* parent)
: QAbstractListModel(parent)
, _rowCount(0)
, _colCount(0)
{
}

QHash<int, QByteArray> PolygonModel2::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PathRole]        = "PathData";
    roles[NameRole]        = "Name";
    roles[DisplayNameRole] = "DisplayName";
    roles[LineColorRole]   = "LineColor";
    roles[LineWidthRole]   = "LineWidth";
    return roles;
}

int PolygonModel2::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(_visibleData.size());
}

int PolygonModel2::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant PolygonModel2::data(const QModelIndex& index, int role) const
{
    if (index.row() < truncate<int>(_visibleData.size())) {
        auto* polygonData = _visibleData.at(index.row());

        if (role == PathRole) {
            return QVariant::fromValue(polygonData->geometry);
        } else if (role == LineColorRole) {
            return polygonData->color;
        } else if (role == LineWidthRole) {
            return polygonData->lineWidth;
        } else if (role == NameRole) {
            return polygonData->name;
        } else if (role == DisplayNameRole) {
            return polygonData->displayName;
        }
    }

    return QVariant();
}

void PolygonModel2::setPolygonData(std::shared_ptr<std::vector<PolygonData2>> data)
{
    _data = data;
    updateVisibleData();
}

void PolygonModel2::setVisibleData(std::vector<QString> names)
{
    _visibleNames = std::move(names);
    updateVisibleData();
}

std::vector<QString> PolygonModel2::visibleData() const
{
    return _visibleNames;
}

void PolygonModel2::clear()
{
    beginResetModel();
    _visibleData.clear();
    _data.reset();
    endResetModel();
}

void PolygonModel2::updateVisibleData()
{
    beginResetModel();
    _visibleData.clear();

    if (_data) {
        for (auto& polyData : *_data) {
            if (inf::container_contains(_visibleNames, polyData.name)) {
                _visibleData.emplace_back(&polyData);
            }
        }
    }

    endResetModel();
}
}
