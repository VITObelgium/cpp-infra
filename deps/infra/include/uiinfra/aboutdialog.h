#pragma once

#include <memory>
#include <qdialog.h>

namespace Ui {
class AboutDialog;
}

namespace uiinfra {

class AboutDialog : public QDialog
{
public:
    struct OpenSourceInfo
    {
        QString name;
        QString website;
        QString license;
        QString licenseLink;
    };

    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

    void setTitle(QString title);
    void setTitleFontSize(int size);
    void setVersion(QString versionString);

    void setLogo(QString resourceId);
    void setCopyrightInfo(QString copyright);

    void addOpenSourceUsage(OpenSourceInfo info);
    void createOpensourceMessage();

private:
    std::unique_ptr<Ui::AboutDialog> _ui;
    std::vector<OpenSourceInfo> _openSource;
};
}
