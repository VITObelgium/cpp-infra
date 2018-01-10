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
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

    void setHeaderImage(QString resourceId);
    void setFooterImage(QString resourceId);

    void setHeaderBackgroundColor(QColor color);
    void setFooterBackgroundColor(QColor color);

    void setLicenseFromResourceTextFile(QString resourceId);
    void setLicenseText(QString licenseString);
    void setVersion(QString versionString);

private:
    std::unique_ptr<Ui::AboutDialog> _ui;
};
}
