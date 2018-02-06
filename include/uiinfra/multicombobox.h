#pragma once

#include <qcombobox.h>

QT_FORWARD_DECLARE_CLASS(QListWidget)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

namespace uiinfra {

class MultiComboBox : public QComboBox
{
    Q_OBJECT

public:
    MultiComboBox(QWidget* widget = nullptr);
    virtual ~MultiComboBox();

    void setDisplayText(QString text);
    QString getDisplayText() const;

    QStringList getSelectectedItemTexts() const;

    void addItem(const QString& text, const QVariant& userData = QVariant());
    void addItems(const QStringList& texts);
    void clearItems();

    void paintEvent(QPaintEvent* e) override;

    void setPopupHeight(int height);

    /// replace standard QComboBox Popup
    void showPopup() override;
    void hidePopup() override;

    /// replace neccessary data access
    int count();
    void setCurrentIndex(int index);
    QString currentText();
    QString itemText(int row);
    QVariant itemData(int row);

signals:
    void itemChanged();

public slots:
    void scanItemSelect(QListWidgetItem* item);
    void initStyleOption(QStyleOptionComboBox* option) const;

protected:
    int _popupHeight;

    /// lower/upper screen bound
    int _screenBound;

    QString _displayText;
    QFrame* _popupFrame;
    QListWidget* _listWidget;
};
}
