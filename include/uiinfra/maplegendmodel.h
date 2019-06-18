#pragma once

#include <qabstractitemmodel.h>
#include <qcolor.h>

namespace inf::ui {

class LegendItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QColor color READ color CONSTANT)

public:
    LegendItem() = default;
    LegendItem(const QString& name, const QColor& color)
    : _name(name)
    , _color(color)
    {
    }

    LegendItem(const LegendItem& other)
    : _name(other.name())
    , _color(other.color())
    {
    }

    QString name() const noexcept
    {
        return _name;
    }

    QColor color() const noexcept
    {
        return _color;
    }

private:
    QString _name;
    QColor _color;
};

class MapLegendModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum LocationRoles
    {
        NameRole = Qt::UserRole + 1,
        ColorRole
    };

    MapLegendModel(QObject* parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void addLegendItem(const LegendItem& loc);
    void setLegendItems(std::vector<LegendItem> loc);

    void clear();

private:
    std::vector<LegendItem> _items;
};

}
