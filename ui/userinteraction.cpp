#include "uiinfra/userinteraction.h"
#include "uiinfra/stringinputdialog.h"

#include <qlocale.h>
#include <qmessagebox.h>

namespace inf::ui {

bool askForConfirmation(const QString& title, const QString& message)
{
    QMessageBox mb(QMessageBox::Question, title, message, QMessageBox::Yes | QMessageBox::No);

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

void displayMessage(const QString& title, const QString& message, QWidget* parent)
{
    QMessageBox mb(QMessageBox::Information, title, message, QMessageBox::Ok, parent);
    mb.exec();
}

void displayError(const QString& title, const QString& message)
{
    QMessageBox mb(QMessageBox::Critical, title, message, QMessageBox::Ok);
    mb.exec();
}
}
