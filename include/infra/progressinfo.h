#pragma once

#include "infra/cast.h"

#include <atomic>
#include <functional>

namespace inf {

class ProgressInfo
{
public:
    ProgressInfo() = default;

    explicit ProgressInfo(std::function<bool(int64_t, int64_t)> cb)
    : _cb(cb)
    {
    }

    ProgressInfo(int64_t totalTicks, std::function<bool(int64_t, int64_t)> cb)
    : _ticks(totalTicks)
    , _cb(cb)
    {
    }

    void set_callback(std::function<bool(int64_t, int64_t)> cb)
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
            _cancel = !_cb(_currentTick, _ticks);
        }
    }

    void signal_progress(float progress)
    {
        if (_cb) {
            _cancel = !_cb(truncate<int64_t>(progress * 100.f), 100);
        }
    }

    int64_t _ticks                    = 0;
    std::atomic<int64_t> _currentTick = 0;
    bool _cancel                      = false;
    std::function<bool(int64_t, int64_t)> _cb;
};

}
