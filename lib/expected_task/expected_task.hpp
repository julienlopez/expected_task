#pragma once

#include <concepts>

#include <pplx/pplxtasks.h>

#include <tl/expected.hpp>

namespace expected_task
{

template <class T, class E> class expected_task;

namespace details
{

    template <class T> struct is_task : std::false_type
    {
    };

    template <class T> struct is_task<pplx::task<T>> : std::true_type
    {
    };

    template <class T> constexpr bool is_task_v = is_task<T>::value;

    template <class T> struct is_expected_task : std::false_type
    {
    };

    template <class T, class E> struct is_expected_task<expected_task<T, E>> : std::true_type
    {
    };

    template <class T> constexpr bool is_expected_task_v = is_expected_task<T>::value;

    template <class T> struct is_expected : std::false_type
    {
    };

    template <class T, class E> struct is_expected<tl::expected<T, E>> : std::true_type
    {
    };

    template <class T> constexpr bool is_expected_v = is_expected<T>::value;

    template <class CallbackType, class ArgType> struct callback_return_type
    {
        using type = decltype(std::declval<CallbackType>()(ArgType{}));
    };

    template <class CallbackType> struct callback_return_type<CallbackType, void>
    {
        using type = decltype(std::declval<CallbackType>()());
    };

    template <class CallbackType, class ArgType>
    using callback_return_type_t = typename callback_return_type<CallbackType, ArgType>::type;

} // namespace details

template <class ValueType, class ErrorType = std::wstring> class expected_task
{
public:
    using expected_type = tl::expected<ValueType, ErrorType>;
    using task_type = pplx::task<expected_type>;
    using value_type = ValueType;
    using error_type = ErrorType;
    using unexpected_type = tl::unexpected<ErrorType>;

    expected_task() = default;

    expected_task(task_type task)
        : m_task{std::move(task)}
    {
    }

    template <class Val>
    expected_task(pplx::task<Val> task) requires std::is_convertible_v<Val, ValueType>
        : m_task{task.then([](Val val) { return expected_type{val}; })}
    {
        static_assert(std::is_convertible_v<Val, value_type>);
    }

    expected_task(pplx::task<void> task)
        : m_task{task.then([]() { return expected_type{}; })}
    {
    }

    template <class NewT, class NewE>
    expected_task(tl::expected<NewT, NewE> value)
        : m_task{pplx::task_from_result(std::move(value))}
    {
        static_assert(std::is_convertible_v<NewT, value_type>);
        static_assert(std::is_convertible_v<NewE, error_type>);
    }

    template <class NewE>
    expected_task(tl::unexpected<NewE> error)
        : m_task{pplx::task_from_result(expected_type{std::move(error)})}
    {
        static_assert(std::is_convertible_v<NewE, error_type>);
    }

    template <class NewT>
    expected_task(NewT&& value)
        : m_task{pplx::task_from_result(expected_type{std::forward<NewT>(value)})}
    {
        static_assert(std::is_convertible_v<NewT, value_type>);
    }

    template <class FCT>
    requires(std::is_same_v<value_type, void>&& std::invocable<FCT>)
        || std::invocable<FCT, value_type> auto then_map(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;

        if constexpr(details::is_task_v<result_type>)
        {
            return then_map_with_task(std::forward<FCT>(callback));
        }
        else
        {
            return then_map_basic(std::forward<FCT>(callback));
        }
    }

    template <class FCT>
    requires(std::is_same_v<value_type, void>&& std::invocable<FCT>)
        || std::invocable<FCT, value_type> auto and_then(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;
        if constexpr(details::is_expected_task_v<result_type>)
        {
            return and_then_with_expectedtask(std::forward<FCT>(callback));
        }
        else if constexpr(details::is_task_v<result_type>)
        {
            return and_then_with_simple_task(std::forward<FCT>(callback));
        }
        else
        {
            return and_then_basic(std::forward<FCT>(callback));
        }
    }

    template <class FCT>
    requires std::invocable<FCT, error_type> expected_task or_else(FCT&& callback)
    const
    {
        return m_task.then([c = std::forward<FCT>(callback)](expected_type res) mutable
                           { return res.or_else(std::forward<FCT>(c)); });
    }

    template <class FCT>
    requires std::invocable<FCT, error_type>
    auto map_error(FCT&& callback) const
    {
        using callback_result_type = decltype(callback({}));
        if constexpr(details::is_task_v<callback_result_type>)
        {
            return map_error_with_task(std::forward<FCT>(callback));
        }
        else
        {
            return map_error_basic(std::forward<FCT>(callback));
        }
    }

    /**
     * @brief experimental : pplx::task::then without the overhead
     * TODO test
     */
    template <class FCT>
    requires std::invocable<FCT, value_type>
    auto then(FCT&& callback) const
    {
        return m_task.then(std::forward<FCT>(callback));
    }

    /**
     * @brief returns the task's result, blocking if the task isn't finished.
     *
     * Does not return a value when called on a task with a value_type of void.
     */
    auto get() const
    {
        return m_task.get();
    }

    /**
     * @brief waits for the task to finish, and then returns its status.
     */
    auto wait() const
    {
        return m_task.wait();
    }

    /**
     * @brief experimental : gives access to the underlying task
     * TODO test
     */
    task_type to_task() const
    {
        return m_task;
    }

    /**
     * @brief returns either the value if there is one, or the mapped error otherwise.
     **/
    template <class FCT> pplx::task<value_type> then_return_value_or_convert_error_to_value(FCT&& callback)
    {
        using fct_return_type = decltype(std::invoke(std::declval<FCT>(), std::declval<error_type>()));
        if constexpr(details::is_task_v<fct_return_type>)
        {
            return then_return_value_or_convert_error_to_value_with_task(std::forward<FCT>(callback));
        }
        else
        {
            return then_return_value_or_convert_error_to_value_basic(std::forward<FCT>(callback));
        }
    }

private:
    task_type m_task;

    template <class FCT> auto then_map_basic(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;

        static_assert(details::is_task_v<result_type> == false,
                      "use then_map_basic only with functions NOT returning a pplx::task");
        static_assert(details::is_expected_v<result_type> == false, "use and_then with functions returning expected");
        return expected_task<result_type, error_type>{m_task.then(
            [c = std::forward<FCT>(callback)](expected_type res) mutable { return res.map(std::forward<FCT>(c)); })};
    }

    template <class FCT> auto then_map_with_task(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;
        static_assert(details::is_task_v<result_type>,
                      "use then_map_with_task only with functions returning a pplx::task");
        using final_type = typename result_type::result_type;
        static_assert(details::is_expected_v<final_type> == false, "use and_then with functions returning expected");
        return expected_task<final_type, error_type>{
            m_task.then([c = std::forward<FCT>(callback)](expected_type res) mutable
                        { return res.map(std::forward<FCT>(c)).map([](auto t) { return t.get(); }); })};
    }

    template <class FCT> auto and_then_basic(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;
        static_assert(details::is_task_v<result_type> == false && details::is_expected_task_v<result_type> == false,
                      "use and_then_basic only with functions NOT returning any kind of task");
        static_assert(details::is_expected_v<result_type>, "use then_map with functions not returning expected");
        static_assert(std::is_convertible_v<typename result_type::error_type, error_type>, "error types must match");
        return expected_task<typename result_type::value_type, error_type>{
            m_task.then([c = std::forward<FCT>(callback)](expected_type res) mutable
                        { return res.and_then(std::forward<FCT>(c)); })};
    }

    template <class FCT> auto and_then_with_simple_task(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;
        static_assert(details::is_task_v<result_type>,
                      "use and_then_with_simple_task only with functions returning a pplx::task<tl::expected<T, F>>");
        using expected_res_type = typename result_type::result_type;
        static_assert(details::is_expected_v<expected_res_type>, "use then_map with functions not returning expected");
        static_assert(std::is_convertible_v<typename expected_res_type::error_type, error_type>,
                      "error types must match");
        const auto t = m_task.then(
            [c = std::forward<FCT>(callback)](expected_type res) mutable -> expected_res_type
            {
                if(res)
                {
                    if constexpr(std::is_same_v<value_type, void>)
                        return c().get();
                    else
                        return c(std::move(*res)).get();
                }
                else
                    return tl::make_unexpected(res.error());
            });
        return expected_task<typename expected_res_type::value_type, error_type>(t);
    }

    template <class FCT> auto and_then_with_expectedtask(FCT&& callback) const
    {
        using result_type = details::callback_return_type_t<FCT, value_type>;
        static_assert(details::is_expected_task_v<result_type>,
                      "use and_then_with_expectedtask only with functions returning an expected_task");
        static_assert(std::is_convertible_v<typename result_type::error_type, error_type>, "error types must match");
        const auto t = m_task.then(
            [c = std::forward<FCT>(callback)](expected_type res) mutable -> typename result_type::expected_type
            {
                if(res)
                {
                    if constexpr(std::is_same_v<value_type, void>)
                        return c().get();
                    else
                        return c(std::move(*res)).get();
                }
                else
                    return tl::make_unexpected(res.error());
            });
        return expected_task<typename result_type::value_type, error_type>(t);
    }

    template <class FCT> auto map_error_basic(FCT&& callback) const
    {
        using callback_result_type = decltype(callback({}));
        static_assert(details::is_task_v<callback_result_type> == false,
                      "use map_error_basic only with functions NOT returning a pplx::task");
        using return_type = expected_task<value_type, callback_result_type>;
        return return_type{m_task.then([c = std::forward<FCT>(callback)](expected_type res) mutable
                                       { return res.map_error(std::forward<FCT>(c)); })};
    }

    template <class FCT> auto map_error_with_task(FCT&& callback) const
    {
        using callback_result_type = decltype(callback({}));
        static_assert(details::is_task_v<callback_result_type>,
                      "use map_error_with_task only with functions returning a pplx::task");
        using return_type = expected_task<value_type, typename callback_result_type::result_type>;
        return return_type{
            m_task.then([c = std::forward<FCT>(callback)](expected_type res) mutable
                        { return res.map_error(std::forward<FCT>(c)).map_error([](auto t) { return t.get(); }); })};
    }

    template <class FCT> pplx::task<value_type> then_return_value_or_convert_error_to_value_basic(FCT&& callback)
    {
        using fct_return_type = decltype(std::invoke(std::declval<FCT>(), std::declval<error_type>()));
        static_assert(details::is_task_v<fct_return_type> == false,
                      "use then_return_value_or_convert_error_to_value_basic on with functions NOT returning a task");
        static_assert(std::is_convertible_v<fct_return_type, value_type>,
                      "the function passed to then_return_value_or_convert_error_to_value must return a "
                      "value convertible into a T");
        return m_task.then(
            [c = std::forward<FCT>(callback)](expected_type exp) -> value_type
            {
                if(exp)
                    return exp.value();
                else
                    return c(std::move(exp.error()));
            });
    }

    template <class FCT> pplx::task<value_type> then_return_value_or_convert_error_to_value_with_task(FCT&& callback)
    {
        using fct_return_type = decltype(std::invoke(std::declval<FCT>(), std::declval<error_type>()));
        static_assert(details::is_task_v<fct_return_type>,
                      "use then_return_value_or_convert_error_to_value_with_task on with functions returning a task");
        using final_return_type = typename fct_return_type::result_type;
        static_assert(std::is_convertible_v<final_return_type, value_type>,
                      "the function passed to then_return_value_or_convert_error_to_value must return a "
                      "value convertible into a T");
        return m_task.then(
            [c = std::forward<FCT>(callback)](expected_type exp) -> value_type
            {
                if(exp)
                    return exp.value();
                else
                    return c(std::move(exp.error())).get();
            });
    }
};

template <class E = std::wstring, class FCT> auto create_task(FCT&& fct)
{
    using ReturnType = decltype(fct());
    return expected_task<ReturnType, E>{pplx::create_task(std::forward<FCT>(fct))};
}

} // namespace expected_task
