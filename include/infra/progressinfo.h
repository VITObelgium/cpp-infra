#pragma once

#include <functional>

namespace inf {

class ProgressInfo
{
public:
    ProgressInfo() = default;

    ProgressInfo(int totalTicks, std::function<bool(int, int)> cb)
    : _ticks(totalTicks)
    , _cb(cb)
    {
    }

    void set_callback(std::function<bool(int, int)> cb)
    {
        _cb = cb;
    }

    void set_total_ticks(int ticks)
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

    int _ticks                    = 0;
    std::atomic<int> _currentTick = 0;
    bool _cancel                  = false;
    std::function<bool(int, int)> _cb;
};

}
