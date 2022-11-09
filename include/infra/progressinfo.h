#pragma once

#include "infra/cast.h"
#include "infra/exception.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <type_traits>

namespace inf {

namespace detail {

template <typename T>
class PayloadBase
{
protected:
    PayloadBase() = default;
    PayloadBase(T payload)
    : _payload(payload)
    {
    }

    void set_payload_impl(T payload)
    {
        _payload = std::move(payload);
    }

    T payload_impl() const noexcept
    {
        return _payload;
    }

private:
    template <typename U>
    friend class ProgressTracker;
    T _payload;
};

template <>
class PayloadBase<void>
{
    void set_payload_impl()
    {
    }
};
}

template <typename T>
class ProgressStatus : public detail::PayloadBase<T>
{
public:
    ProgressStatus() = default;
    ProgressStatus(const ProgressStatus& other)
    : detail::PayloadBase<T>(other)
    , _current(other.current())
    , _total(other.total())
    {
    }

    ProgressStatus(int64_t current, int64_t total) noexcept
    : _current(current)
    , _total(total)
    {
    }

    template <typename TPayload, typename = typename std::enable_if<!std::is_void_v<TPayload>>>
    ProgressStatus(int64_t current, int64_t total, TPayload payload) noexcept
    : detail::PayloadBase<T>(payload)
    , _current(current)
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

    template <typename TPayload, typename = typename std::enable_if<!std::is_void_v<TPayload>>>
    void set_payload(const TPayload& payload)
    {
        std::scoped_lock lock(_mutex);
        detail::PayloadBase<T>::set_payload_impl(payload);
    }

    template <typename TPayload = T, typename = typename std::enable_if<!std::is_void_v<TPayload>>>
    TPayload payload() const noexcept
    {
        TPayload result;

        {
            std::scoped_lock lock(_mutex);
            result = detail::PayloadBase<T>::payload_impl();
        }

        return result;
    }

private:
    void increment() noexcept
    {
        ++_current;
    }

    void reset() noexcept
    {
        reset(0);
    }

    void reset(int64_t total) noexcept
    {
        _current = 0;
        _total   = total;
    }

    template <typename U>
    friend class ProgressTracker;

    std::atomic<int64_t> _current = 0;
    int64_t _total                = 0;
    mutable std::mutex _mutex;
};

enum class ProgressStatusResult
{
    Continue,
    Abort,
};

template <typename ProgressPayload = void>
class ProgressTracker
{
public:
    using Status   = ProgressStatus<ProgressPayload>;
    using Callback = std::function<ProgressStatusResult(Status)>;

    ProgressTracker() = default;

    explicit ProgressTracker(Callback cb)
    : _cb(cb)
    {
    }

    ProgressTracker(int64_t totalTicks, Callback cb)
    : _status(0, totalTicks)
    , _cb(cb)
    {
    }

    void set_callback(Callback cb)
    {
        _cb = cb;
    }

    [[deprecated("Use reset to modify the total ticks")]] void set_total_ticks(int64_t ticks) noexcept
    {
        reset(ticks);
    }

    int64_t total_ticks() const noexcept
    {
        return _status.total();
    }

    int64_t current_tick() const noexcept
    {
        return _status.current();
    }

    /*! Use in combination with set_total_ticks, progress will be calculated internally */
    void tick() noexcept
    {
        _status.increment();
        signal_progress();
    }

    /*! Use in combination with set_total_ticks, progress will be calculated internally
     * /throws CancelRequested when cancellation is requested
     **/
    void tick_throw_on_cancel()
    {
        tick();
        throw_on_cancel();
    }

    /*! Provide the current progress explicitely without relying on a tick count
     * /progress value between 0.0 and 1.0
     */
    void tick(float progress) noexcept
    {
        signal_progress(progress);
    }

    void tick_throw_on_cancel(float progress) noexcept
    {
        signal_progress(progress);
        throw_on_cancel();
    }

    bool cancel_requested() const noexcept
    {
        return _cancel;
    }

    void reset() noexcept
    {
        reset(0);
    }

    void reset(int64_t total) noexcept
    {
        _status.reset(total);
        _cancel = false;
    }

    bool is_valid() const noexcept
    {
        return static_cast<bool>(_cb);
    }

    Status& status() noexcept
    {
        return _status;
    }

    template <typename U = ProgressPayload>
    typename std::enable_if_t<!std::is_void_v<U>, void> set_payload(const U& pl)
    {
        _status.set_payload(std::move(pl));
    }

    template <typename U = ProgressPayload>
    typename std::enable_if_t<!std::is_void_v<U>, U> payload() const noexcept
    {
        return _status.payload();
    }

private:
    void signal_progress() noexcept
    {
        if (_cb) {
            _cancel = _cb(_status) == ProgressStatusResult::Abort;
        }
    }

    void signal_progress(float progress) noexcept
    {
        if (_cb) {
            const auto progress100 = truncate<int64_t>(progress * 100.f);
            if constexpr (std::is_void_v<ProgressPayload>) {
                _cancel = _cb(Status(progress100, 100)) == ProgressStatusResult::Abort;
            } else {
                _cancel = _cb(Status(progress100, 100, _status.payload())) == ProgressStatusResult::Abort;
            }
        }
    }

    void throw_on_cancel() const
    {
        if (cancel_requested()) {
            throw CancelRequested();
        }
    }

    Status _status;
    bool _cancel = false;
    Callback _cb;
};

using ProgressInfo        = ProgressTracker<void>;
using MessageProgressInfo = ProgressTracker<std::string>;

}
