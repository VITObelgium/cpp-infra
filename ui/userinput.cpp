#include "uiinfra/userinput.h"
#include "uiinfra/stringinputdialog.h"

#include <qmessagebox.h>

namespace uiinfra {

bool askForConfirmation(const QString& title, const QString& message)
{
    QMessageBox mb(title, message,
        QMessageBox::Question,
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No | QMessageBox::Escape,
        QMessageBox::NoButton);

    return mb.exec() == QMessageBox::Yes;
}

QString askForString(QWidget* parent, const QString& title, const QString& name)
{
    StringInputDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setLabel(name);

    if (dlg.exec() == QDialog::Accepted) {
        return dlg.value();
    }

    return QString();
}

void displayError(const QString& title, const QString& message)
{
    QMessageBox mb(title, message, QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
}
}
