#include "uiinfra/aboutdialog.h"
#include "ui_aboutdialog.h"

#include <qfile.h>
#include <qtextstream.h>

namespace uiinfra {

AboutDialog::AboutDialog(QWidget* parent)
: QDialog(parent)
, _ui(std::make_unique<Ui::AboutDialog>())
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    _ui->setupUi(this);
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::setHeaderImage(QString resourceId)
{
    _ui->headerLabel->setPixmap(QPixmap(resourceId));
}

void AboutDialog::setFooterImage(QString resourceId)
{
    _ui->footerLabel->setPixmap(QPixmap(resourceId));
}

void AboutDialog::setHeaderBackgroundColor(QColor color)
{
    QPalette palette;
    palette.setColor(QPalette::Window, color);

    _ui->headerLabel->setAutoFillBackground(true);
    _ui->headerLabel->setPalette(palette);
}

void AboutDialog::setFooterBackgroundColor(QColor color)
{
    QPalette palette;
    palette.setColor(QPalette::Window, color);

    _ui->footerLabel->setAutoFillBackground(true);
    _ui->footerLabel->setPalette(palette);
}

void AboutDialog::setLicenseFromResourceTextFile(QString resourceId)
{
    QFile myFile(resourceId);
    myFile.open(QIODevice::ReadOnly);
    QTextStream textStream(&myFile);
    setLicenseText(textStream.readAll());
}

void AboutDialog::setLicenseText(QString licenseString)
{
    _ui->licenseTextBox->setText(licenseString);
}

void AboutDialog::setVersion(QString versionString)
{
    _ui->versionLabel->setText(versionString);
}
}
