#pragma once

#include <memory>
#include <qdialog.h>

namespace Ui {
class AboutDialog;
}

namespace inf::ui {

struct OpenSourceUsageInfo
{
    QString name;
    QString website;
    QString license;
    QString licenseLink;
};

QString createOpenSourceUsageMessage(const std::vector<OpenSourceUsageInfo>& usages);

class AboutDialog : public QDialog
{
public:
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

    void setTitle(QString title);
    void setTitleFontSize(int size);
    void setVersion(QString versionString);

    void setLogo(QString resourceId);
    void setCopyrightInfo(QString copyright);

    void addOpenSourceUsage(OpenSourceUsageInfo info);
    void createOpensourceMessage();

private:
    std::unique_ptr<Ui::AboutDialog> _ui;
    std::vector<OpenSourceUsageInfo> _openSource;
};
}
