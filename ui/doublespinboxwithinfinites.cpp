#include "uiinfra/doublespinboxwithinfinites.h"

namespace inf {

QWidget* DoubleSpinBoxWithInfinites::ItemEditorCreator::createWidget(QWidget* parent) const
{
    return new DoubleSpinBoxWithInfinites(parent);
}

QByteArray DoubleSpinBoxWithInfinites::ItemEditorCreator::valuePropertyName() const
{
    return "value";
}

DoubleSpinBoxWithInfinites::DoubleSpinBoxWithInfinites(QWidget* parent)
: QDoubleSpinBox(parent)
{
    setMinimum(-std::numeric_limits<double>::infinity());
    setMaximum(std::numeric_limits<double>::infinity());
}

void DoubleSpinBoxWithInfinites::fixup(QString& input) const
{
    if (QStringList({"i", "in", "inf"}).contains(input)) {
        input = "inf";
    } else if (QStringList({"-i", "-in", "-inf"}).contains(input)) {
        input = "-inf";
    } else {
        return QDoubleSpinBox::fixup(input);
    }
}

QValidator::State DoubleSpinBoxWithInfinites::validate(QString& text, int& pos) const
{
    if (QStringList({"i", "in", "-i", "-in"}).contains(text)) {
        return QValidator::Intermediate;
    } else if (QStringList({"inf", "-inf"}).contains(text)) {
        return QValidator::Acceptable;
    } else {
        return QDoubleSpinBox::validate(text, pos);
    }
}

QString DoubleSpinBoxWithInfinites::textFromValue(double value) const
{
    if (value == std::numeric_limits<double>::infinity()) {
        return "inf";
    } else if (value == -std::numeric_limits<double>::infinity()) {
        return "-inf";
    } else {
        return QDoubleSpinBox::textFromValue(value);
    }
}

double DoubleSpinBoxWithInfinites::valueFromText(const QString& text) const
{
    if (text == "inf") {
        return std::numeric_limits<double>::infinity();
    } else if (text == "-inf") {
        return -std::numeric_limits<double>::infinity();
    } else {
        QString textDotComma = text;
        textDotComma         = textDotComma.replace(QLatin1Char('.'), QLocale().decimalPoint());
        textDotComma         = textDotComma.replace(QLatin1Char(','), QLocale().decimalPoint());
        return QDoubleSpinBox::valueFromText(textDotComma);
    }
}

}
