﻿#include "uiinfra/aboutdialog.h"
#include "ui_aboutdialog.h"

#include <qfile.h>
#include <qtextstream.h>

namespace inf::ui {

QString createOpenSourceUsageMessage(const std::vector<OpenSourceUsageInfo>& usages)
{
    QString openSourceInfo("<center><b>Open Source Libraries</b></center>");

    for (auto& info : usages) {
        openSourceInfo.append(QString("<center><p><a href=\"%1\">%2</a> (<a href=\"%3\">%4</a>)</p></center>")
                                  .arg(info.website)
                                  .arg(info.name)
                                  .arg(info.licenseLink)
                                  .arg(info.license));
    }

    return openSourceInfo;
}

AboutDialog::AboutDialog(QWidget* parent)
: QDialog(parent)
, _ui(std::make_unique<Ui::AboutDialog>())
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    _ui->setupUi(this);

    _ui->logoLabel->setScaledContents(true);
    _ui->copyrightLabel->setStyleSheet("QLabel { color : grey; }");
    _ui->acknowledgementsButton->setVisible(false);
    _ui->licenseEdit->hide();
    _ui->hideButton->hide();
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::setTitle(QString title)
{
    _ui->titleLabel->setText(title);
}

void AboutDialog::setTitleFontSize(int size)
{
    auto font = _ui->titleLabel->font();
    font.setPointSize(size);
    _ui->titleLabel->setFont(font);
}

void AboutDialog::setVersion(QString versionString)
{
    _ui->versionLabel->setText(versionString);
}

void AboutDialog::setLogo(QString resourceId)
{
    QPixmap pixmap(resourceId);
    if (!pixmap.isNull()) {
        _ui->logoLabel->setPixmap(pixmap.scaled(_ui->logoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void AboutDialog::setCopyrightInfo(QString copyright)
{
    _ui->copyrightLabel->setText(copyright);
}

void AboutDialog::addOpenSourceUsage(OpenSourceUsageInfo info)
{
    _ui->acknowledgementsButton->setVisible(true);
    _openSource.push_back(info);
}

void AboutDialog::createOpensourceMessage()
{
    _ui->licenseEdit->setHtml(createOpenSourceUsageMessage(_openSource));
}

}
