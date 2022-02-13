#pragma once

#include <boost/optional.hpp>

template <class T, class F>
requires std::invocable<F, T>
auto operator>=(boost::optional<T>&& opt, F f)
{
    return opt.map(std::move(f));
}

template <class T, class F>
requires std::invocable<F, T>
auto operator>=(boost::optional<T>& opt, F f)
{
    return opt.map(std::move(f));
}

template <class T, class F>
requires std::invocable<F, T>
auto operator>=(const boost::optional<T>& opt, F f)
{
    return opt.map(std::move(f));
}

template <class T, class F>
requires std::invocable<F, T>
auto operator>>=(boost::optional<T>&& opt, F f)
{
    return opt.flat_map(std::move(f));
}

template <class T, class F>
requires std::invocable<F, T>
auto operator>>=(boost::optional<T>& opt, F f)
{
    return opt.flat_map(std::move(f));
}

template <class T, class F>
requires std::invocable<F, T>
auto operator>>=(const boost::optional<T>& opt, F f)
{
    return opt.flat_map(std::move(f));
}
