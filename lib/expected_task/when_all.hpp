#pragma once

#include "expected_task.hpp"

#include <algorithm>
#include <numeric>

namespace expected_task
{

namespace details
{

    template <class T, class E> E stackErrors(std::vector<tl::expected<T, E>> results, const E& delimiter)
    {
        const auto it = std::remove_if(begin(results), end(results), std::mem_fn(&tl::expected<T, E>::has_value));
        return std::accumulate(begin(results), it, E{}, [&delimiter](const E& acc, const tl::expected<T, E>& exp) {
            if(acc == E{})
                return exp.error();
            else
                return acc + delimiter + exp.error();
        });
    }

} // namespace details

template <class T, class E>
expected_task<std::vector<T>, E> when_all(const std::vector<expected_task<T, E>>& tasks, const E& delimiter = L" && ")
{
    std::vector<pplx::task<tl::expected<T, E>>> pplx_tasks(tasks.size());
    std::transform(begin(tasks), end(tasks), std::begin(pplx_tasks), std::mem_fn(&expected_task<T, E>::to_task));

    using Expected_type = tl::expected<T, E>;
    return when_all(begin(pplx_tasks), end(pplx_tasks))
        .then([delimiter](std::vector<Expected_type> results) -> tl::expected<std::vector<T>, E> {
            if(std::all_of(begin(results), end(results), std::mem_fn(&Expected_type::has_value)))
            {
                std::vector<T> res(results.size());
                std::transform(begin(results), end(results), begin(res), [](auto& v) { return *v; });
                return res;
            }
            else
            {
                return tl::make_unexpected(details::stackErrors(results, delimiter));
            }
        });
}

template <class T, class E> expected_task<std::vector<T>, E> operator&&(expected_task<T, E> t1, expected_task<T, E> t2)
{
    return when_all(std::vector<expected_task<T, E>>{std::move(t1), std::move(t2)});
}

} // namespace expected_task
