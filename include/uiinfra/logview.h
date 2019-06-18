#pragma once

#include <memory>
#include <qwidget.h>

namespace Ui {
class LogView;
}

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace inf::ui {

class LogView : public QWidget
{
    Q_OBJECT

public:
    explicit LogView(QWidget* parent = nullptr);
    ~LogView();

    void setModel(QAbstractItemModel* model);

    void resizeEvent(QResizeEvent* event) override;

private:
    void updateView();

    std::unique_ptr<Ui::LogView> _ui;
    QTimer* _timer;
};
}
