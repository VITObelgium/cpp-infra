#pragma once

#include <qevent.h>
#include <qspinbox.h>

namespace inf::ui {

class NoScrollSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    NoScrollSpinBox(QWidget* parent = nullptr)
    : QSpinBox(parent)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

protected:
    virtual void wheelEvent(QWheelEvent* event) override
    {
        if (!hasFocus()) {
            event->ignore();
        } else {
            QSpinBox::wheelEvent(event);
        }
    }
};

class NoScrollDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    NoScrollDoubleSpinBox(QWidget* parent = nullptr)
    : QDoubleSpinBox(parent)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

protected:
    virtual void wheelEvent(QWheelEvent* event) override
    {
        if (!hasFocus()) {
            event->ignore();
        } else {
            QDoubleSpinBox::wheelEvent(event);
        }
    }
};

}
