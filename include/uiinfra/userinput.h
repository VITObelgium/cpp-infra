#pragma once

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
}
