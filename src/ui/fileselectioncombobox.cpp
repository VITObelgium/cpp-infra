#include "fileselectioncombobox.h"
#include "infra/log.h"

#include <cassert>
#include <qfiledialog.h>
#include <qsettings.h>

namespace opaq {

FileSelectionComboBox::FileSelectionComboBox(QWidget* parent)
: QWidget(parent)
, _name("FileSelectionComboBox")
, _fileSelectorWindowTitle(tr("Select file"))
, _fileSelectorFilter(tr("All files (*.*)"))
, _maxRecentItems(5)
{
    setupUi(this);

    connect(browseButton, &QPushButton::clicked, this, &FileSelectionComboBox::showFileSelector);
    connect(comboBox, QOverload<const QString&>::of(&QComboBox::activated), this, &FileSelectionComboBox::pathChanged);

    label->hide();
}

void FileSelectionComboBox::setConfigName(const QString& name)
{
    _name = name;
    loadRecentPaths();
}

void FileSelectionComboBox::setLabel(const QString& value)
{
    label->setText(value);
    label->show();
}

void FileSelectionComboBox::setFileSelectorWindowTitle(const QString& value)
{
    _fileSelectorWindowTitle = value;
}

void FileSelectionComboBox::setFileSelectorInitialDir(const QString& value)
{
    _fileSelectorInitialDir = value;
}

void FileSelectionComboBox::setFileSelectorFilter(const QString& value)
{
    _fileSelectorFilter = value;
}

QString FileSelectionComboBox::currentPath() const
{
    return comboBox->currentText();
}

void FileSelectionComboBox::loadRecentPaths()
{
    if (_name.isEmpty()) {
        return;
    }

    QSettings settings;
    auto recentFilePaths = settings.value(_name).toStringList();
    while (recentFilePaths.size() > _maxRecentItems) {
        recentFilePaths.removeLast();
    }

    comboBox->clear();
    comboBox->addItems(recentFilePaths);
    comboBox->setCurrentIndex(-1);
}

void FileSelectionComboBox::updateRecentPath(const QString& filePath)
{
    QSettings settings;
    QStringList recentFilePaths = settings.value(_name).toStringList();
    recentFilePaths.removeAll(filePath);
    recentFilePaths.prepend(filePath);
    while (recentFilePaths.size() > _maxRecentItems) {
        recentFilePaths.removeLast();
    }

    settings.setValue(_name, recentFilePaths);

    loadRecentPaths();
}

void FileSelectionComboBox::showFileSelector()
{
    auto filename = QFileDialog::getOpenFileName(this, _fileSelectorWindowTitle, _fileSelectorInitialDir, _fileSelectorFilter);
    if (filename.isEmpty()) {
        return;
    }

    updateRecentPath(filename);
    comboBox->setCurrentIndex(0);
    emit pathChanged(comboBox->currentText());
}

}
