#pragma once

#include <expected_task/expected_task.hpp>

namespace Testing
{

auto makeTimesTwoLambda(std::size_t& has_been_called)
{
    return [&has_been_called](double value) -> pplx::task<double>
    {
        has_been_called++;
        return pplx::task_from_result(2 * value);
    };
}

auto makeFailableTimesTwoLambda(std::size_t& has_been_called)
{
    return [&has_been_called](double value) -> tl::expected<double, std::wstring>
    {
        has_been_called++;
        return 2 * value;
    };
}

auto makeFailableTimesTwoLambdaT(std::size_t& has_been_called)
{
    return [&has_been_called](double value)
    {
        has_been_called++;
        return pplx::task_from_result(tl::expected<double, std::wstring>{2 * value});
    };
}

auto makeFailableTimesTwoLambdaET(std::size_t& has_been_called)
{
    return [&has_been_called](double value)
    {
        has_been_called++;
        return expected_task::expected_task<double, std::wstring>{2 * value};
    };
}

template <class T, class Err> auto makeFailLambda(std::size_t& has_been_called, Err error)
{
    return [&has_been_called, e = std::move(error)](double) mutable -> tl::expected<T, Err>
    {
        has_been_called++;
        return tl::make_unexpected(std::move(e));
    };
}

template <class T, class Err> auto makeFailLambdaT(std::size_t& has_been_called, Err error)
{
    return [&has_been_called, e = std::move(error)](double) mutable
    {
        has_been_called++;
        return pplx::task_from_result(tl::expected<T, Err>{tl::make_unexpected(std::move(e))});
    };
}

template <class T, class Err> auto makeFailLambdaET(std::size_t& has_been_called, Err error)
{
    return [&has_been_called, e = std::move(error)](double) mutable -> expected_task::expected_task<T, Err>
    {
        has_been_called++;
        return tl::make_unexpected(std::move(e));
    };
}

} // namespace Testing