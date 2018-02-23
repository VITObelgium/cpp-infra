#pragma once

#include <deque>
#include <mutex>
#include <qabstractitemmodel.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>

namespace uiinfra {

class LogSinkModel : public spdlog::sinks::sink, public QAbstractListModel
{
public:
    LogSinkModel(QObject* parent = nullptr);

    void log(const spdlog::details::log_msg& msg) override;
    void flush() override;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    struct LogMessage
    {
        spdlog::level::level_enum level;
        QString message;
    };

    mutable std::recursive_mutex _mutex;
    std::deque<LogMessage> _messages;
};
}
