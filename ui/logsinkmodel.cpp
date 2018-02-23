#include "uiinfra/logsinkmodel.h"

#include <qapplication.h>
#include <qstyle.h>

namespace uiinfra {

LogSinkModel::LogSinkModel(QObject* parent)
: QAbstractListModel(parent)
{
}

void LogSinkModel::log(const spdlog::details::log_msg& msg)
{
    std::scoped_lock lock(_mutex);
    auto rows = rowCount();
    beginInsertRows(QModelIndex(), rows, rows);
    _messages.push_back({msg.level, QString::fromLocal8Bit(msg.raw.c_str())});
    endInsertRows();
}

void LogSinkModel::flush()
{
}

QVariant LogSinkModel::data(const QModelIndex& index, int role) const
{
    std::scoped_lock lock(_mutex);
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _messages.at(size_t(index.row())).message;
    }

    if (role == Qt::DecorationRole) {
        switch (_messages.at(size_t(index.row())).level) {
        case spdlog::level::info:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxInformation));
        case spdlog::level::warn:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxWarning));
        case spdlog::level::err:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxCritical));
        default:
            break;
        }
    }

    return QVariant();
}

int LogSinkModel::rowCount(const QModelIndex& /*parent*/) const
{
    std::scoped_lock lock(_mutex);
    return static_cast<int>(_messages.size());
}
}
