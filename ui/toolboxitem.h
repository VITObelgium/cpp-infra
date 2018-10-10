#pragma once

#include "ui_toolboxitem.h"

namespace uiinfra {

class ToolboxItem : public QWidget
{
    Q_OBJECT

public:
    explicit ToolboxItem(const QString& label, const QIcon& icon, QWidget* parent = nullptr);
    ~ToolboxItem() override;

    void setText(const QString& text);

signals:
    void clicked();

private:
    Ui::ToolboxItem _ui;
};
}
