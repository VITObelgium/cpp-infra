#pragma once

#include <QString>
#include <QMessageBox>

inline void displayError(QWidget* parent, QString title, QString errorMsg)
{
    QMessageBox messageBox;
    messageBox.critical(parent, title, errorMsg);
}