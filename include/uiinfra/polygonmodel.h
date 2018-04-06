#pragma once

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
    };

    PolygonModel(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void addGeoData(const std::string& name, std::vector<QGeoPath> data);
    void setGeoData(std::unordered_map<std::string, std::vector<QGeoPath>> data);
    void setVisibleData(std::vector<std::string> names);
    std::vector<std::string> visibleData() const;

    void clear();

private:
    void updateVisibleData();

    int _rowCount;
    int _colCount;
    std::unordered_map<std::string, std::vector<QGeoPath>> _data;
    std::vector<QGeoPath*> _visibleData;
    std::vector<std::string> _visibleNames;
};
}
