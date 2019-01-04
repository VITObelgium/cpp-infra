#include "uiinfra/cursorposprovider.h"

namespace uiinfra {

CursorPosProvider::CursorPosProvider(QObject* parent)
: QObject(parent)
{
}

QPointF CursorPosProvider::cursorPos() const noexcept
{
    return QCursor::pos();
}

}
