#pragma once

#include <qmessagebox.h>

namespace uiinfra {

bool askForConfirmation(const QString& title, const QString& message);
QString askForString(QWidget* parent, const QString& title, const QString& name);

void displayMessage(const QString& title, const QString& message, QWidget* parent = nullptr);
void displayError(const QString& title, const QString& message);
}
