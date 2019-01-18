#pragma once

#include <qcombobox.h>

#include "ui_fileselectioncombobox.h"

namespace opaq {

class FileSelectionComboBox : public QWidget, private Ui::FileSelectionComboBox
{
    Q_OBJECT

public:
    FileSelectionComboBox(QWidget* parent = nullptr);

    void setConfigName(const QString& name);

    void setLabel(const QString& value);
    void setFileSelectorWindowTitle(const QString& value);
    void setFileSelectorInitialDir(const QString& value);
    void setFileSelectorFilter(const QString& value);

    QString currentPath() const;

signals:
    void pathChanged(const QString&);

private:
    void loadRecentPaths();
    void updateRecentPath(const QString& filePath);

    void showFileSelector();

    QString _name;
    QString _fileSelectorWindowTitle;
    QString _fileSelectorInitialDir;
    QString _fileSelectorFilter;
    int _maxRecentItems;
};
}
