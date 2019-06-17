#pragma once

#include <functional>

namespace inf {

class ProgressInfo
{
public:
    ProgressInfo() = default;

    ProgressInfo(int64_t totalTicks, std::function<bool(int64_t, int64_t)> cb)
    : _ticks(totalTicks)
    , _cb(cb)
    {
    }

    void set_callback(std::function<bool(int64_t, int64_t)> cb)
    {
        _cb = cb;
    }

    void set_total_ticks(int64_t ticks)
    {
        _ticks = ticks;
    }

    void tick()
    {
        ++_currentTick;
        signal_progress();
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

private:
    void signal_progress()
    {
        if (_cb) {
            _cancel = !_cb(_currentTick, _ticks);
        }
    }

    int64_t _ticks                    = 0;
    std::atomic<int64_t> _currentTick = 0;
    bool _cancel                      = false;
    std::function<bool(int64_t, int64_t)> _cb;
};

}
