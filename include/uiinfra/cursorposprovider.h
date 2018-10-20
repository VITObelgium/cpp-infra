#pragma once
#include <QCursor>
#include <QPointF>
#include <qobject.h>

namespace uiinfra {

class CursorPosProvider : public QObject
{
    Q_OBJECT
public:
    explicit CursorPosProvider(QObject* parent = nullptr);

    Q_INVOKABLE QPointF cursorPos() const noexcept;
};
}
