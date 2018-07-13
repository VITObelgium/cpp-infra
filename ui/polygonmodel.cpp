#include "uiinfra/polygonmodel.h"
#include "infra/algo.h"
#include "infra/cast.h"

#include <qcolor.h>
#include <qgeolocation.h>
#include <qgeorectangle.h>

namespace uiinfra {

using namespace infra;

PolygonModel::PolygonModel(QObject* parent)
: QAbstractListModel(parent)
, _rowCount(0)
, _colCount(0)
{
}

QHash<int, QByteArray> PolygonModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PathRole] = "PathData";
    return roles;
}

int PolygonModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(_visibleData.size());
}

int PolygonModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant PolygonModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < truncate<int>(_visibleData.size())) {
        if (role == PathRole) {
            return QVariant::fromValue(*_visibleData.at(index.row()));
        }
    }

    return QVariant();
}

void PolygonModel::setGeoData(std::shared_ptr<OverlayMap> data)
{
    _data = data;
    updateVisibleData();
}

void PolygonModel::setVisibleData(std::vector<QString> names)
{
    _visibleNames = std::move(names);
    updateVisibleData();
}

std::vector<QString> PolygonModel::visibleData() const
{
    return _visibleNames;
}

void PolygonModel::clear()
{
    beginResetModel();
    _visibleData.clear();
    endResetModel();
}

void PolygonModel::updateVisibleData()
{
    beginResetModel();
    _visibleData.clear();

    if (_data) {
        for (auto& [name, value] : *_data) {
            if (infra::containerContains(_visibleNames, name)) {
                for (auto& geoPath : value) {
                    _visibleData.push_back(&geoPath);
                }
            }
        }
    }

    endResetModel();
}
}
