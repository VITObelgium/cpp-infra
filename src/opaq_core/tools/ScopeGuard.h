#pragma once

namespace opaq
{

namespace details
{

class guard_base
{
public:
    guard_base()
    : m_released(false)
    {
    }

    guard_base(const guard_base& rhs)
    : m_released(rhs.m_released)
    {
        rhs.release();
    }

    void release() const
    {
        m_released = true;
    }

protected:
    mutable bool m_released;

private:
    guard_base& operator=(const guard_base&);
};

template <typename F>
class guard_impl : private guard_base
{
public:
    explicit guard_impl(const F& x)
    : m_action(x)
    {
    }

    ~guard_impl()
    {
        if (m_released)
        {
            return;
        }

        // Catch any exceptions and continue during guard clean up
        try
        {
            m_action();
        }
        catch (...)
        {
        }
    }

private:
    F m_action;
};

}

template <typename F>
details::guard_impl<F> make_scope_guard(F f)
{
    return details::guard_impl<F>(f);
}

typedef const details::guard_base& scope_guard;

}
