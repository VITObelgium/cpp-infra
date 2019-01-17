#include "uiinfra/pixmapimage.h"

#include <qpainter.h>

namespace inf::ui {

PixmapImage::PixmapImage(QQuickItem* parent)
: QQuickPaintedItem(parent)
{
}

void PixmapImage::setImage(QPixmap pixmap)
{
    _pixmap = pixmap;
    setSize(_pixmap.size());
    update();
}

void PixmapImage::clear()
{
    setImage(QPixmap());
}

void PixmapImage::paint(QPainter* painter)
{
    painter->drawPixmap(0, 0, width(), height(), _pixmap);
}

}
