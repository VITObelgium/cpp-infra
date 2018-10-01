#include "uiinfra/stringinputdialog.h"
#include "ui_stringinputdialog.h"

#include <qmessagebox.h>

namespace uiinfra {

StringInputDialog::StringInputDialog(QWidget* parent)
: QDialog(parent)
, _ui(std::make_unique<Ui::StringInputDialog>())
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    _ui->setupUi(this);
}

StringInputDialog::~StringInputDialog() = default;

QString StringInputDialog::value() const
{
    return _ui->editBox->text();
}

void StringInputDialog::setLabel(const QString& label)
{
    _ui->label->setText(label);
}
}
