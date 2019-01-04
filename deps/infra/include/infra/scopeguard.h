#pragma once

#include <functional>

namespace inf {

class ScopeGuard
{
public:
    template <typename Callable>
    ScopeGuard(Callable&& cb)
    : _atExit(cb)
    {
    }

    ScopeGuard(ScopeGuard&&) = default;
    ScopeGuard& operator=(ScopeGuard&&) = default;

    ~ScopeGuard()
    {
        if (_atExit) {
            _atExit();
        }
    }

    void release()
    {
        _atExit = nullptr;
    }

private:
    std::function<void()> _atExit;
};

}
