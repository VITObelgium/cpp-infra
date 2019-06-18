#pragma once

#include <qapplication.h>

namespace inf::ui {

/* RAII class for changing the cursor shape in a scope
 */
class ScopedCursor
{
public:
    explicit ScopedCursor(const QCursor& cursor)
    {
        QApplication::setOverrideCursor(cursor);
        QApplication::processEvents();
    }

    ~ScopedCursor()
    {
        QApplication::restoreOverrideCursor();
        QApplication::processEvents();
    }
};
}
