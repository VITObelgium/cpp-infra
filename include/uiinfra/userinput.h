#pragma once

#include <qmessagebox.h>

namespace uiinfra {

inline bool askForConfirmation(const QString& title, const QString& message)
{
    QMessageBox mb(title, message,
                   QMessageBox::Question,
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No | QMessageBox::Escape,
                   QMessageBox::NoButton);

    return mb.exec() == QMessageBox::Yes;
}

inline bool displayError(const QString& title, const QString& message)
{
    QMessageBox mb(title, message, QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
    return mb.exec() == QMessageBox::Yes;
}
}
