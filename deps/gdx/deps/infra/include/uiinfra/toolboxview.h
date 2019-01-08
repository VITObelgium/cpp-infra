#pragma once

#include <qboxlayout.h>
#include <qstringview.h>
#include <qtoolbox.h>
#include <unordered_map>

namespace uiinfra {

class ToolboxView : public QToolBox
{
    Q_OBJECT

public:
    explicit ToolboxView(QWidget* parent = nullptr);

    int addSection(const QString& sectionName);

    void addItemToSection(int sectionIndex, int itemId, const QString& itemName, QIcon icon);
    void setItemsVisible(bool visible);

    //! Typically use on a language change event
    void setSectionText(int sectionId, const QString& text);
    void setToolboxItemText(int itemId, const QString& text);

signals:
    void itemClicked(int);

private:
    void onItemSelected();
};
}
