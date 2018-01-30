#include "uiinfra/multicombobox.h"

#include <qapplication.h>
#include <qboxlayout.h>
#include <qdesktopwidget.h>
#include <qlistwidget.h>
#include <qstylepainter.h>

namespace uiinfra {

MultiComboBox::MultiComboBox(QWidget* widget)
: QComboBox(widget)
, _popupHeight(0)
, _screenBound(50)
, _popupFrame(new QFrame(NULL, Qt::Popup))
, _listWidget(new QListWidget)
{
    // setup the popup list
    _listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    _listWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    _listWidget->clearSelection();
    _popupFrame->setLayout(new QVBoxLayout());
    _popupFrame->layout()->addWidget(_listWidget);
    _popupFrame->layout()->setContentsMargins(0, 0, 0, 0);

    connect(_listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(scanItemSelect(QListWidgetItem*)));
}

MultiComboBox::~MultiComboBox()
{
    disconnect(_listWidget, 0, 0, 0);
}

void MultiComboBox::setDisplayText(QString text)
{
    _displayText        = text;
    const int textWidth = fontMetrics().width(text);
    setMinimumWidth(textWidth + 50);
    updateGeometry();
    repaint();
}

QStringList MultiComboBox::getSelectectedItemTexts() const
{
    QStringList result;

    for (int i = 0; i < _listWidget->count(); ++i) {
        auto* item = _listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            result.append(item->text());
        }
    }

    return result;
}

QString MultiComboBox::getDisplayText() const
{
    return _displayText;
}

void MultiComboBox::setPopupHeight(int height)
{
    _popupHeight = height;
}

void MultiComboBox::paintEvent(QPaintEvent* /*e*/)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    opt.currentText = _displayText;
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);
    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void MultiComboBox::showPopup()
{
    QRect rec = QRect(geometry());

    //QPoint p = this->mapToGlobal(QPoint(0,rec.height()));
    //QRect rec2(p , p + QPoint(rec.width(), rec.height()));

    // get the two possible list points and height
    QRect screen    = QApplication::desktop()->screenGeometry(this);
    QPoint above    = this->mapToGlobal(QPoint(0, 0));
    int aboveHeight = above.y() - screen.y();
    QPoint below    = this->mapToGlobal(QPoint(0, rec.height()));
    int belowHeight = screen.bottom() - below.y();

    int requiredWidth = 0;
    for (int i = 0; i < _listWidget->count(); ++i) {
        auto* item    = _listWidget->item(i);
        requiredWidth = std::max(requiredWidth, fontMetrics().width(item->text()));
    }

    QRect rec2;
    rec2.setTopLeft(below);
    rec2.setWidth(requiredWidth + 50);
    _popupFrame->setGeometry(rec2);

    // determine rect
    int contheight = _listWidget->count() * _listWidget->sizeHintForRow(0) + 4; // +4 - should be determined by margins?
    belowHeight    = std::min(abs(belowHeight) - _screenBound, contheight);
    aboveHeight    = std::min(abs(aboveHeight) - _screenBound, contheight);

    if (_popupHeight > 0) { // fixed
        rec2.setHeight(_popupHeight);
    } else { // dynamic
        // do we use below or above
        if (belowHeight == contheight || belowHeight > aboveHeight) {
            rec2.setTopLeft(below);
            rec2.setHeight(belowHeight);
        } else {
            rec2.setTopLeft(above - QPoint(0, aboveHeight));
            rec2.setHeight(aboveHeight);
        }
    }

    _popupFrame->setGeometry(rec2);
    _popupFrame->raise();
    _popupFrame->show();
}

void MultiComboBox::hidePopup()
{
    _popupFrame->hide();
}

void MultiComboBox::addItem(const QString& text, const QVariant& userData)
{
    QListWidgetItem* pListWidgetItem = new QListWidgetItem(text);
    pListWidgetItem->setFlags(pListWidgetItem->flags() | Qt::ItemIsUserCheckable);

    if (userData.toBool()) {
        pListWidgetItem->setCheckState(Qt::Checked);
    } else {
        pListWidgetItem->setCheckState(Qt::Unchecked);
    }

    _listWidget->addItem(pListWidgetItem);
}

void MultiComboBox::addItems(const QStringList& texts)
{
    for (auto& text : texts) {
        addItem(text);
    }
}

int MultiComboBox::count()
{
    return _listWidget->count();
}

void MultiComboBox::setCurrentIndex(int /*index*/)
{
    assert(false);
}

QString MultiComboBox::currentText()
{
    return _listWidget->currentItem()->text();
}

QString MultiComboBox::itemText(int row)
{
    return _listWidget->item(row)->text();
}

QVariant MultiComboBox::itemData(int row)
{
    QListWidgetItem* item = _listWidget->item(row);

    if (item->checkState() == Qt::Checked) {
        return QVariant(true);
    }

    return QVariant(false);
}

void MultiComboBox::scanItemSelect(QListWidgetItem* item)
{
    QList<QListWidgetItem*> list = _listWidget->selectedItems();

    for (int i = 0; i < list.count(); i++) {
        if (item->checkState() == Qt::Checked) {
            list[i]->setCheckState(Qt::Checked);
        } else {
            list[i]->setCheckState(Qt::Unchecked);
        }

        list[i]->setSelected(false);
    }

    emit itemChanged();
}

void MultiComboBox::initStyleOption(QStyleOptionComboBox* option) const
{
    //Initializes the state, direction, rect, palette, and fontMetrics member variables based on the specified widget.
    //This is a convenience function; the member variables can also be initialized manually.
    option->initFrom(this);
}
}
