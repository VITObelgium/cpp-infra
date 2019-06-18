#pragma once

#include <QAbstractItemModel>

namespace inf::ui {

class ClearableAbstractItemModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ClearableAbstractItemModel(QObject* parent = nullptr)
    : QAbstractItemModel(parent)
    {
    }

    virtual ~ClearableAbstractItemModel() = default;

    virtual void clear() = 0;
};
}
