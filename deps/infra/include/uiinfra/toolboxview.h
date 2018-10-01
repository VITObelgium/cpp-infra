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

    void addSection(const QString& name);

    void addItemToSection(const QString& sectionName, int itemId, const QString& itemName, QIcon icon);
    void setItemsVisible(bool visible);

signals:
    void itemClicked(int);

private:
    QFrame* appendSection(const QString& name);
    void onItemSelected();
};
}
