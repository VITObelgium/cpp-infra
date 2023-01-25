#include "uiinfra/userinteraction.h"
#include "uiinfra/stringinputdialog.h"

#include <qlocale.h>
#include <qmessagebox.h>

namespace inf::ui {

bool askForConfirmation(const QString& title, const QString& message, QWidget* parent)
{
    QMessageBox mb(QMessageBox::Question, title, message, QMessageBox::Yes | QMessageBox::No, parent);

    return mb.exec() == QMessageBox::Yes;
}

int askQuestion(const QString& title, const QString& message, const QString& option1, const QString& option2, QWidget* parent)
{
    QMessageBox mb(QMessageBox::Question, title, message, QMessageBox::NoButton, parent);
    mb.addButton(option1, QMessageBox::AcceptRole);
    mb.addButton(option2, QMessageBox::RejectRole);
    return mb.exec() + 1; // exec returns the 0 based index of the clicked button
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
