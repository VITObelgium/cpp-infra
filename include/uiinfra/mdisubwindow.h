#pragma once

#include <optional>
#include <qmdisubwindow.h>

namespace inf::ui {

class MdiSubWindow : public QMdiSubWindow
{
public:
    MdiSubWindow(QWidget* internalWidget, QWidget* parent = nullptr);
    virtual ~MdiSubWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void onMdiWindowStateChange(Qt::WindowStates oldState, Qt::WindowStates newState);
    void setLayoutMargin(int top, int left, int right, int bottom);

    void saveSettings();
    void restoreSettings();

    std::optional<int> _defaultMarginTop;
    std::optional<int> _defaultMarginLeft;
    std::optional<int> _defaultMarginRight;
    std::optional<int> _defaultMarginBottom;
};
}
