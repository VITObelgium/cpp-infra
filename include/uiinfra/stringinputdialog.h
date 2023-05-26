#pragma once

#include <memory>
#include <qdialog.h>

namespace Ui {
class StringInputDialog;
}

namespace inf::ui {

class VersionsModel;

class StringInputDialog : public QDialog
{
public:
    StringInputDialog(QWidget* parent = nullptr);
    ~StringInputDialog();

    QString value() const;
    void setValue(const QString& value);

    void setLabel(const QString& label);

private:
    std::unique_ptr<Ui::StringInputDialog> _ui;
};
}
