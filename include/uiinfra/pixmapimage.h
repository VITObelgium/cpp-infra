#pragma once

#include <qpixmap.h>
#include <qquickpainteditem.h>

QT_FORWARD_DECLARE_CLASS(QPainter)

namespace inf::ui {

class PixmapImage : public QQuickPaintedItem
{
    Q_OBJECT
public:
    PixmapImage(QQuickItem* parent = nullptr);
    Q_INVOKABLE const QPixmap image() const;
    Q_INVOKABLE void setImage(QPixmap pixmapContainer);
    Q_INVOKABLE void clear();

protected:
    virtual void paint(QPainter* painter) override;

private:
    QPixmap _pixmap;
};

}
