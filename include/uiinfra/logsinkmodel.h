#pragma once

#include <deque>
#include <mutex>
#include <qabstractitemmodel.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace inf::ui {

template <typename Mutex>
class LogSinkModel : public spdlog::sinks::base_sink<Mutex>, public QAbstractListModel
{
public:
    LogSinkModel(QObject* parent = nullptr);

protected:
    void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) override;

    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    struct LogMessage
    {
        spdlog::level::level_enum level;
        QString message;
    };

    int size() const;

    mutable Mutex _mutex;
    std::deque<LogMessage> _messages;
};

using LogSinkModelMt = LogSinkModel<std::recursive_mutex>;
using LogSinkModelSt = LogSinkModel<spdlog::details::null_mutex>;
}
