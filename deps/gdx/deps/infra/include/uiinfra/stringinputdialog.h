#pragma once

#include <memory>
#include <qdialog.h>

namespace Ui {
class StringInputDialog;
}

namespace uiinfra {

class VersionsModel;

class StringInputDialog : public QDialog
{
public:
    StringInputDialog(QWidget* parent = nullptr);
    ~StringInputDialog();

    QString value() const;

    void setLabel(const QString& label);

private:
    std::unique_ptr<Ui::StringInputDialog> _ui;
};
}
