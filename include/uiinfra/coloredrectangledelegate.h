#pragma once

#include <optional>
#include <qhash.h>
#include <qstyleditemdelegate.h>

class QModelIndex;
class QWidget;
class QVariant;

namespace inf::ui {

/*! Item delegate that draws a rectangle with the configured role color */
class ColoredRectangleDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ColoredRectangleDelegate(QObject* parent = nullptr);

    void setHeight(int height);
    void setWidth(int width);

    /* set modelindex role to use to obtain the color info (default = Qt::BackgroundColorRole) */
    void setRole(int role);
    int role() const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    int _height = 20;
    std::optional<int> _width;
    int _role = Qt::BackgroundRole;
};
}
