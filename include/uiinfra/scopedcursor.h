#pragma once

#include <qapplication.h>

namespace uiinfra {

/* RAII class for changing the cursor shape in a scope
 */
class ScopedCursor
{
public:
    explicit ScopedCursor(const QCursor& cursor)
    {
        QApplication::setOverrideCursor(cursor);
    }

    ~ScopedCursor()
    {
        QApplication::restoreOverrideCursor();
    }
};
}
