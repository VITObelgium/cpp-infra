#include "uiinfra/logsinkmodel.h"

#include <qapplication.h>
#include <qstyle.h>

namespace uiinfra {

template <typename Mutex>
LogSinkModel<Mutex>::LogSinkModel(QObject* parent)
: QAbstractListModel(parent)
{
}

template <typename Mutex>
void LogSinkModel<Mutex>::sink_it_(const spdlog::details::log_msg& msg)
{
    fmt::memory_buffer formatted;
    spdlog::sinks::sink::formatter_->format(msg, formatted);

    std::lock_guard<Mutex> lock(_mutex);
    auto rows = size();
    beginInsertRows(QModelIndex(), rows, rows);
    _messages.push_back({msg.level, QString::fromUtf8(fmt::to_string(formatted).c_str())});
    endInsertRows();
}

template <typename Mutex>
void LogSinkModel<Mutex>::flush_()
{
}

template <typename Mutex>
QVariant LogSinkModel<Mutex>::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    std::lock_guard<Mutex> lock(_mutex);
    if (index.row() >= size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _messages.at(size_t(index.row())).message;
    }

    if (role == Qt::DecorationRole) {
        switch (_messages.at(size_t(index.row())).level) {
        case spdlog::level::debug:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxQuestion));
        case spdlog::level::info:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxInformation));
        case spdlog::level::warn:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxWarning));
        case spdlog::level::err:
        case spdlog::level::critical:
            return QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MessageBoxCritical));
        default:
            break;
        }
    }

    return QVariant();
}

template <typename Mutex>
int LogSinkModel<Mutex>::rowCount(const QModelIndex& /*parent*/) const
{
    std::lock_guard<Mutex> lock(_mutex);
    return size();
}

template <typename Mutex>
int LogSinkModel<Mutex>::size() const
{
    return static_cast<int>(_messages.size());
}

template class LogSinkModel<std::recursive_mutex>;
template class LogSinkModel<spdlog::details::null_mutex>;

}