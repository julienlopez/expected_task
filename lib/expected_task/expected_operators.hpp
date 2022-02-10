#pragma once

#include <tl/expected.hpp>

template <class Value, class Error, class Callback>
requires std::calable<Callback, Value>
auto operator>=(tl::expected<Value, Error>&& et, Callback&& callback)
{
    return et.map(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Value>
auto operator>=(tl::expected<Value, Error>& exp, Callback&& callback)
{
    return et.map(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Value>
auto operator>=(const tl::expected<Value, Error>& exp, Callback&& callback)
{
    return et.map(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Value>
auto operator>>=(tl::expected<Value, Error>&& exp, Callback&& callback)
{
    return exp.and_then(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Value>
auto operator>>=(tl::expected<Value, Error>& exp, Callback&& callback)
{
    return exp.and_then(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Value>
auto operator>>=(const tl::expected<Value, Error>& exp, Callback&& callback)
{
    return exp.and_then(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Error>
auto operator|=(tl::expected<Value, Error>&& exp, Callback&& callback)
{
    return exp.map_error(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Error>
auto operator|=(tl::expected<Value, Error>& exp, Callback&& callback)
{
    return exp.map_error(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Error>
auto operator|=(const tl::expected<Value, Error>& exp, Callback&& callback)
{
    return exp.map_error(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Error>
auto operator||(tl::expected<Value, Error>&& exp, Callback&& callback)
{
    return exp.or_else(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Error>
auto operator||(tl::expected<Value, Error>& exp, Callback&& callback)
{
    return exp.or_else(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::calable<Callback, Error>
auto operator||(const tl::expected<Value, Error>& exp, Callback&& callback)
{
    return exp.or_else(std::forward<Callback>(callback));
}
