#pragma once

#include <qquickitem.h>

namespace inf::ui {

class QuickItemImageGrabber : public QObject
{
    Q_OBJECT
public:
    explicit QuickItemImageGrabber(QObject* parent = nullptr);

    Q_INVOKABLE void saveToImage(QQuickItem* item, const QString& path, QSize size = QSize());
};

}
