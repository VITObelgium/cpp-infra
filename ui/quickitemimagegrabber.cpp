#include "uiinfra/quickitemimagegrabber.h"

#include "infra/cast.h"

#include <qquickitemgrabresult.h>

namespace inf::ui {

using namespace inf;

QuickItemImageGrabber::QuickItemImageGrabber(QObject* parent)
: QObject(parent)
{
}

void QuickItemImageGrabber::saveToImage(QQuickItem* item, const QString& path, QSize size)
{
    if (item) {
        if (size.isEmpty()) {
            size = QSize(truncate<int>(item->size().width()), truncate<int>(item->size().height()));
        }

        auto result = item->grabToImage(size);
        if (result) {
            connect(result.get(), &QQuickItemGrabResult::ready, this, [result, path]() {
                result->saveToFile(path);
            });
        }
    }
}

}
