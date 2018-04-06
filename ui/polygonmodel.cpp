#include "uiinfra/polygonmodel.h"
#include "infra/algo.h"

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
    if (role == PathRole) {
        return QVariant::fromValue(*_visibleData.at(index.row()));
    }

    return QVariant();
}

void PolygonModel::addGeoData(const std::string& name, std::vector<QGeoPath> data)
{
    _data[name] = std::move(data);
    updateVisibleData();
}

void PolygonModel::setGeoData(std::unordered_map<std::string, std::vector<QGeoPath>> data)
{
    _data = std::move(data);
    updateVisibleData();
}

void PolygonModel::setVisibleData(std::vector<std::string> names)
{
    _visibleNames = std::move(names);
    updateVisibleData();
}

std::vector<std::string> PolygonModel::visibleData() const
{
    return _visibleNames;
}

void PolygonModel::clear()
{
    beginResetModel();
    _data.clear();
    _visibleData.clear();
    endResetModel();
}

void PolygonModel::updateVisibleData()
{
    beginResetModel();
    _visibleData.clear();

    for (auto& [name, data] : _data) {
        if (infra::containerContains(_visibleNames, name)) {
            for (auto& geoPath : data) {
                _visibleData.push_back(&geoPath);
            }
        }
    }

    endResetModel();
}
}
