#include "uiinfra/toolboxview.h"
#include "toolboxitem.h"

#include <cassert>
#include <qtoolbutton.h>

namespace inf::ui {

static const char* s_idProperty = "itemid";

ToolboxView::ToolboxView(QWidget* parent)
: QToolBox(parent)
{
}

int ToolboxView::addSection(const QString& sectionName)
{
    auto* sectionFrame = new QFrame(this);
    auto* layout       = new QVBoxLayout(sectionFrame);
    layout->setAlignment(Qt::AlignTop);
    sectionFrame->setObjectName(sectionName);
    sectionFrame->setLayout(layout);
    sectionFrame->setBackgroundRole(QPalette::Light);
    return addItem(sectionFrame, sectionName);
}

void ToolboxView::addItemToSection(int sectionIndex, int itemId, const QString& itemName, QIcon icon)
{
    auto* sectionFrame = qobject_cast<QFrame*>(widget(sectionIndex));
    if (!sectionFrame) {
        return;
    }

    auto* item = new ToolboxItem(itemName, icon, this);
    item->setProperty(s_idProperty, itemId);
    connect(item, &ToolboxItem::clicked, this, &ToolboxView::onItemSelected);
    sectionFrame->layout()->addWidget(item);
}

void ToolboxView::setItemsVisible(bool visible)
{
    for (int i = 0; i < count(); ++i) {
        auto* w = qobject_cast<QFrame*>(widget(i));
        w->setVisible(visible);
    }
}

void ToolboxView::setSectionText(int sectionIndex, const QString& text)
{
    setItemText(sectionIndex, text);
}

void ToolboxView::setToolboxItemText(int itemId, const QString& text)
{
    for (auto* child : findChildren<ToolboxItem*>()) {
        if (child->property(s_idProperty).toInt() == itemId) {
            child->setText(text);
            return;
        }
    }
}

void ToolboxView::onItemSelected()
{
    auto* item = qobject_cast<ToolboxItem*>(sender());
    assert(item);
    emit itemClicked(item->property("itemid").toInt());
}
}
