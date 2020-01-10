#pragma once

#include "infra/cast.h"

#include <atomic>
#include <functional>

namespace inf {

class ProgressInfo
{
public:
    enum class StatusResult
    {
        Continue,
        Abort,
    };

    class Status
    {
    public:
        Status(int64_t current, int64_t total) noexcept
        : _current(current)
        , _total(total)
        {
        }

        /*! returns the progress as a value between 0 and 1 */
        float progress() const noexcept
        {
            return float(_current) / _total;
        }

        /*! returns the progress as a value between 0 and 100 */
        int32_t progress_100() const noexcept
        {
            return truncate<int32_t>(_current * 100.f / _total);
        }

        int64_t current() const noexcept
        {
            return _current;
        }

        int64_t total() const noexcept
        {
            return _total;
        }

    private:
        int64_t _current = 0;
        int64_t _total   = 0;
    };

    using Callback = std::function<StatusResult(Status)>;

    ProgressInfo() = default;

    explicit ProgressInfo(Callback cb)
    : _cb(cb)
    {
    }

    ProgressInfo(int64_t totalTicks, Callback cb)
    : _ticks(totalTicks)
    , _cb(cb)
    {
    }

    void set_callback(Callback cb)
    {
        _cb = cb;
    }

    void set_total_ticks(int64_t ticks) noexcept
    {
        _ticks = ticks;
    }

    int64_t total_ticks() const noexcept
    {
        return _ticks;
    }

    int64_t current_tick() const noexcept
    {
        return _currentTick;
    }

    /*! Use in combination with set_total_ticks, progress will be calculated internally */
    void tick() noexcept
    {
        ++_currentTick;
        signal_progress();
    }

    /*! Provide the current progress explicitely without relying on a tick count
	 * /progress value between 0.0 and 1.0
	 */
    void tick(float progress) noexcept
    {
        signal_progress(progress);
    }

    bool cancel_requested() const noexcept
    {
        return _cancel;
    }

    void reset() noexcept
    {
        _ticks       = 0;
        _currentTick = 0;
        _cancel      = false;
    }

    bool is_valid() const noexcept
    {
        return static_cast<bool>(_cb);
    }

private:
    void signal_progress()
    {
        if (_cb) {
            _cancel = _cb(Status(_currentTick, _ticks)) == StatusResult::Abort;
        }
    }

    void signal_progress(float progress)
    {
        if (_cb) {
            _cancel = _cb(Status(truncate<int64_t>(progress * 100.f), 100)) == StatusResult::Abort;
        }
    }

    int64_t _ticks                    = 0;
    std::atomic<int64_t> _currentTick = 0;
    bool _cancel                      = false;
    Callback _cb;
};

}
