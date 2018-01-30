#pragma once

#include <qcombobox.h>
#include <qstandarditemmodel.h>

namespace uiinfra {

class CheckComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QString defaultText READ defaultText WRITE setDefaultText)
    Q_PROPERTY(QStringList checkedItems READ checkedItems WRITE setCheckedItems)

public:
    explicit CheckComboBox(QWidget* parent = 0);

    void hidePopup() override;

    QString defaultText() const;
    void setDefaultText(const QString& text);

    Qt::CheckState itemCheckState(int index) const;
    void setItemCheckState(int index, Qt::CheckState state);

    QStringList checkedItems() const;

public Q_SLOTS:
    void setCheckedItems(const QStringList& items);

Q_SIGNALS:
    void checkedItemsChanged(const QStringList& items);

private Q_SLOTS:
    void updateCheckedItems();
    void toggleCheckState(int index);

private:
    bool eventFilter(QObject* receiver, QEvent* event);

    QString _defaultText;
    bool _containerMousePress = false;
};

class CheckComboModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit CheckComboModel(QObject* parent = 0);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

Q_SIGNALS:
    void checkStateChanged();
};
}
