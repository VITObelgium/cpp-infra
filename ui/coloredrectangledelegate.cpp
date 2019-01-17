#include "uiinfra/coloredrectangledelegate.h"

#include <qlistview.h>
#include <qpainter.h>

namespace inf::ui {

ColoredRectangleDelegate::ColoredRectangleDelegate(QObject* parent)
: QStyledItemDelegate(parent)
{
}

void ColoredRectangleDelegate::setHeight(int height)
{
    _height = height;
}

void ColoredRectangleDelegate::setWidth(int width)
{
    _width = width;
}

void ColoredRectangleDelegate::setRole(int role)
{
    _role = role;
}

int ColoredRectangleDelegate::role() const
{
    return _role;
}

void ColoredRectangleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QColor color = index.data(_role).toString();

    // Draw rectangle containing the legend color
    auto rect = option.rect;
    rect.adjust(1, 1, -1, -1);

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));

    painter->drawRect(rect);
    painter->restore();
}

QSize ColoredRectangleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    return QSize(_width.value_or(option.rect.width()), _height);
}

}
