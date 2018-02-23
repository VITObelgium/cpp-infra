#pragma once

#include <qwidget.h>

namespace Ui {
class LogView;
}

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace uiinfra {

class LogView : public QWidget
{
    Q_OBJECT

public:
    explicit LogView(QWidget* parent = nullptr);
    ~LogView();

    void setModel(QAbstractItemModel* model);

    void resizeEvent(QResizeEvent* event) override;

private:
    std::unique_ptr<Ui::LogView> _ui;
};
}