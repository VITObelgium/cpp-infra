#include "uiinfra/cursorposprovider.h"

namespace inf::ui {

CursorPosProvider::CursorPosProvider(QObject* parent)
: QObject(parent)
{
}

QPointF CursorPosProvider::cursorPos() const noexcept
{
    return QCursor::pos();
}

}
