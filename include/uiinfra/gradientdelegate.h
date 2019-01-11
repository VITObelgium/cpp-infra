#pragma once

#include <qhash.h>
#include <qstyleditemdelegate.h>

class QModelIndex;
class QWidget;
class QVariant;

namespace inf::ui {

/*! Item delegate that draws a rectangle filled with the gradient from the configured model role */
class GradientDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    GradientDelegate(QObject* parent = nullptr);

    void setHeight(int height);
    /* set modelindex role to use to obtain the colormap name info (default = Qt::DisplayRole) */
    void setRole(int role);

    void prepareGradients(const QStringList& colormapNames);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    const QGradientStops* addGradient(const QString& cmapName) const;

    mutable QHash<QString, QGradientStops> _gradients;
    int _height = 20;
    int _role   = Qt::DisplayRole;
};
}
