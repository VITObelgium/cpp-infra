#pragma once

#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

namespace inf {

template <typename... Args>
class Signal
{
public:
    Signal()              = default;
    Signal(const Signal&) = delete;
    Signal(Signal&& other) noexcept
    {
        // move the other subscribers using his mutex
        std::lock_guard<std::mutex> lock(other._mutex);
        _subscribers = std::move(other._subscribers);
    }

    ~Signal()
    {
        disconnect_all();
    }

    Signal& operator=(const Signal&) = delete;

    Signal& operator=(Signal&& other) noexcept
    {
        // move the other subscribers using his mutex
        std::lock_guard<std::mutex> lock(other._mutex);
        _subscribers = std::move(other._subscribers);
        return *this;
    }

    void connect(const void* receiver, const std::function<void(Args...)>& func)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscribers.push_back(Subscriber(receiver, func));
    }

    void disconnect(const void* receiver)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto iter = std::find_if(_subscribers.begin(), _subscribers.end(), [=](const auto& sub) {
            return sub.receiver == receiver;
        });

        if (iter != _subscribers.end()) {
            _subscribers.erase(iter);
        }
    }

    template <typename... CallArgs>
    void operator()(CallArgs&&... args)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& sub : _subscribers) {
            sub.callback(args...);
        }
    }

    void disconnect_all()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscribers.clear();
    }

private:
    struct Subscriber
    {
        Subscriber(const void* rcv, const std::function<void(Args...)>& cb)
        : receiver(rcv)
        , callback(cb)
        {
        }

        const void* receiver;
        std::function<void(Args...)> callback;
    };

    std::vector<Subscriber> _subscribers;
    std::mutex _mutex;
};

template <>
class Signal<>
{
public:
    Signal()              = default;
    Signal(const Signal&) = delete;
    Signal(Signal&& other) noexcept
    {
        // move the other subscribers using his mutex
        std::lock_guard<std::mutex> lock(other._mutex);
        _subscribers = std::move(other._subscribers);
    }

    ~Signal()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscribers.clear();
    }

    Signal& operator=(const Signal&) = delete;

    Signal& operator=(Signal&& other) noexcept
    {
        // move the other subscribers using his mutex
        std::lock_guard<std::mutex> lock(other._mutex);
        _subscribers = std::move(other._subscribers);
        return *this;
    }

    void connect(const void* receiver, const std::function<void()>& func)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _subscribers.push_back(Subscriber(receiver, func));
    }

    void disconnect(const void* receiver)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto iter = std::find_if(_subscribers.begin(), _subscribers.end(), [=](const auto& sub) {
            return sub.receiver == receiver;
        });

        if (iter != _subscribers.end()) {
            _subscribers.erase(iter);
        }
    }

    void operator()()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& sub : _subscribers) {
            sub.callback();
        }
    }

private:
    struct Subscriber
    {
        Subscriber(const void* rcv, const std::function<void()>& cb)
        : receiver(rcv)
        , callback(cb)
        {
        }

        const void* receiver;
        std::function<void()> callback;
    };

    std::vector<Subscriber> _subscribers;
    std::mutex _mutex;
};
}
