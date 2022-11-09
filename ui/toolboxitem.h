#pragma once

#include "ui_toolboxitem.h"

namespace inf::ui {

class ToolboxItem : public QWidget
{
    Q_OBJECT

public:
    explicit ToolboxItem(const QString& label, const QIcon& icon, QWidget* parent = nullptr);
    ~ToolboxItem() override;

    void setText(const QString& text);

    Q_SIGNAL void clicked();

private:
    Ui::ToolboxItem _ui;
};
}
