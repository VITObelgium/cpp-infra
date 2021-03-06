#include "uiinfra/gradientdelegate.h"

#include "infra/cast.h"
#include "infra/colormap.h"
#include "uiinfra/colorconversion.h"

#include <qlistview.h>
#include <qpainter.h>

namespace inf::ui {

GradientDelegate::GradientDelegate(QObject* parent)
: QStyledItemDelegate(parent)

{
}

void GradientDelegate::setHeight(int height)
{
    _height = height;
}

void GradientDelegate::setRole(int role)
{
    _role = role;
}

void GradientDelegate::prepareGradients(const QStringList& colormapNames)
{
    _gradients.clear();
    for (auto& cmap : colormapNames) {
        addGradient(cmap);
    }
}

void GradientDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString cmapName = index.data(_role).toString();

    // Draw rectangle containing the legend color
    auto rect = option.rect;
    rect.adjust(1, 1, -1, -1);

    painter->save();
    painter->setPen(Qt::NoPen);
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    if (auto iter = _gradients.find(cmapName); iter != _gradients.end()) {
        gradient.setStops(*iter);
    } else if (auto* stops = addGradient(cmapName); stops != nullptr) {
        gradient.setStops(*stops);
    }

    painter->setBrush(QBrush(gradient));

    painter->drawRect(rect);
    painter->restore();
}

QSize GradientDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    return QSize(option.rect.width(), _height);
}

const QGradientStops* GradientDelegate::addGradient(const QString& cmapName) const
{
    if (auto stops = gradientStopsFromColormap(cmapName.toStdString()); !stops.empty()) {
        return &(*_gradients.insert(cmapName, std::move(stops)));
    }

    return nullptr;
}
}
