#pragma once

#include <concepts>

#include <expected_task/expected_task.hpp>

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Value>
auto operator>=(expected_task::expected_task<Value, Error>&& et, Callback&& callback)
{
    return et.then_map(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Value>
auto operator>=(expected_task::expected_task<Value, Error>& et, Callback&& callback)
{
    return et.then_map(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Value>
auto operator>=(const expected_task::expected_task<Value, Error>& et, Callback&& callback)
{
    return et.then_map(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Value>
auto operator>>=(expected_task::expected_task<Value, Error>&& exp, Callback&& callback)
{
    return exp.and_then(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Value>
auto operator>>=(expected_task::expected_task<Value, Error>& exp, Callback&& callback)
{
    return exp.and_then(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Value>
auto operator>>=(const expected_task::expected_task<Value, Error>& exp, Callback&& callback)
{
    return exp.and_then(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Error>
auto operator|=(expected_task::expected_task<Value, Error>&& exp, Callback&& callback)
{
    return exp.map_error(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Error>
auto operator|=(expected_task::expected_task<Value, Error>& exp, Callback&& callback)
{
    return exp.map_error(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Error>
auto operator|=(const expected_task::expected_task<Value, Error>& exp, Callback&& callback)
{
    return exp.map_error(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Error>
auto operator||(expected_task::expected_task<Value, Error>&& exp, Callback&& callback)
{
    return exp.or_else(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Error>
auto operator||(expected_task::expected_task<Value, Error>& exp, Callback&& callback)
{
    return exp.or_else(std::forward<Callback>(callback));
}

template <class Value, class Error, class Callback>
requires std::invocable<Callback, Error>
auto operator||(const expected_task::expected_task<Value, Error>& exp, Callback&& callback)
{
    return exp.or_else(std::forward<Callback>(callback));
}
