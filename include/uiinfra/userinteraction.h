#pragma once

#include <qmessagebox.h>

namespace inf::ui {

bool askForConfirmation(const QString& title, const QString& message, QWidget* parent = nullptr);

/* returns the chosen option 1 or 2 */
int askQuestion(const QString& title, const QString& message, const QString& option1, const QString& option2, QWidget* parent = nullptr);
QString askForString(QWidget* parent, const QString& title, const QString& name, const QString& initialValue = QString());

void displayMessage(const QString& title, const QString& message, QWidget* parent = nullptr);
void displayError(const QString& title, const QString& message);
}
