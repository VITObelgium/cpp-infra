#pragma once

#include <QDoubleSpinBox>
#include <QItemEditorCreatorBase>

namespace inf {

class DoubleSpinBoxWithInfinites : public QDoubleSpinBox
{
public:
    class ItemEditorCreator : public QItemEditorCreatorBase
    {
        virtual QWidget* createWidget(QWidget* parent) const override;
        virtual QByteArray valuePropertyName() const override;
    };

    DoubleSpinBoxWithInfinites(QWidget* parent);

    virtual void fixup(QString& input) const override;
    virtual QValidator::State validate(QString& text, int& pos) const override;
    virtual QString textFromValue(double value) const override;
    virtual double valueFromText(const QString& text) const override;
};

}