#include "uiinfra/userinteraction.h"
#include "uiinfra/stringinputdialog.h"

#include <qlocale.h>
#include <qmessagebox.h>

namespace uiinfra {

bool askForConfirmation(const QString& title, const QString& message)
{
    QMessageBox mb(title, message,
                   QMessageBox::Question,
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No | QMessageBox::Escape,
                   QMessageBox::NoButton);

    // TODO: should not be necessary ig the proper qt localization files are loaded
    if (QLocale::system().language() == QLocale(QLocale::Dutch).language()) {
        mb.setButtonText(QMessageBox::Yes, "Ja");
        mb.setButtonText(QMessageBox::No, "Nee");
    }

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

void displayMessage(const QString& title, const QString& message)
{
    QMessageBox mb(title, message, QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
}

void displayError(const QString& title, const QString& message)
{
    QMessageBox mb(title, message, QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    mb.exec();
}
}
