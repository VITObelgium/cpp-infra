#include "toolboxitem.h"

namespace uiinfra {

ToolboxItem::ToolboxItem(const QString& label, const QIcon& icon, QWidget* parent)
: QWidget(parent)
{
    _ui.setupUi(this);
    _ui.label->setText(label);
    _ui.pushButton->setIcon(icon);

    connect(_ui.pushButton, &QPushButton::clicked, this, &ToolboxItem::clicked);
}

ToolboxItem::~ToolboxItem() = default;
}
