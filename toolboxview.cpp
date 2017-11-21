#include "uiinfra/toolboxview.h"
#include "uiinfra/toolboxitem.h"

#include <cassert>
#include <qtoolbutton.h>

namespace uiinfra {

ToolboxView::ToolboxView(QWidget* parent)
: QToolBox(parent)
{
}

void ToolboxView::addItemToSection(const QString& sectionName, int itemId, const QString& itemName, QIcon icon)
{
    auto* section = findChild<QFrame*>(sectionName);
    if (!section) {
        section = addSection(sectionName);
    }

    auto* item = new ToolboxItem(itemName, icon, this);
    item->setProperty("itemid", itemId);
    connect(item, &ToolboxItem::clicked, this, &ToolboxView::onItemSelected);
    section->layout()->addWidget(item);
}

void ToolboxView::setItemsVisible(bool visible)
{
    for (int i = 0; i < count(); ++i) {
        auto* w = qobject_cast<QFrame*>(widget(i));
        w->setVisible(visible);
    }
}

QFrame* ToolboxView::addSection(const QString& name)
{
    auto* sectionFrame = new QFrame(this);
    auto* layout       = new QVBoxLayout(sectionFrame);
    layout->setAlignment(Qt::AlignTop);
    sectionFrame->setObjectName(name);
    sectionFrame->setLayout(layout);
    sectionFrame->setBackgroundRole(QPalette::Light);
    addItem(sectionFrame, name);

    return sectionFrame;
}

void ToolboxView::onItemSelected()
{
    auto* item = qobject_cast<ToolboxItem*>(sender());
    assert(item);
    emit itemClicked(item->property("itemid").toInt());
}
}
