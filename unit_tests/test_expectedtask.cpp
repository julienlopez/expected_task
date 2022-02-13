#include <catch2/catch.hpp>

#include <string>

#include "utilities.hpp"

using namespace std::string_literals;

namespace
{

std::string convertStr(const std::wstring& e)
{
    std::string res(e.size(), '0');
    std::transform(begin(e), end(e), begin(res), [](const wchar_t c) { return static_cast<char>(c); });
    return res;
}

pplx::task<std::string> convertStrTask(std::wstring e)
{
    return pplx::create_task([e]() { return convertStr(e); });
}

auto makeConvertLambda(std::size_t& has_been_called)
{
    return [&has_been_called](const std::wstring& str) -> double
    {
        has_been_called++;
        return std::stod(str);
    };
}

auto makeConvertLambdaT(std::size_t& has_been_called)
{
    return [&has_been_called](const std::wstring& str) -> pplx::task<double>
    {
        has_been_called++;
        return pplx::task_from_result(std::stod(str));
    };
}

} // namespace

TEST_CASE("Test expected_task", "[task]")
{

    SECTION("construction from a task<value>")
    {
        const double init_value = 1.2;
        const expected_task::expected_task<double, std::wstring> task{pplx::task_from_result(init_value)};
        const auto res = task.get();
        REQUIRE(res.has_value());
        const auto final_value = *res;
        CHECK(final_value == init_value);
    }

    SECTION("construction from a task<T> convertible in a value")
    {
        const int init_value = 2;
        const expected_task::expected_task<double, std::wstring> task{pplx::task_from_result(init_value)};
        const auto res = task.get();
        REQUIRE(res.has_value());
        const auto final_value = *res;
        CHECK(final_value == init_value);
    }

    SECTION("basic usage of then_map")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 2 * init_value);
        }

        SECTION("with no value")
        {
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{tl::make_unexpected(L"nope"s)}
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 3);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 8 * init_value);
        }

        SECTION("with type change")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == std::to_wstring(2 * init_value));
        }

        SECTION("with conversion")
        {
            const double init_value = 2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<int, std::wstring>{init_value}
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 2 * init_value);
        }
    }

    SECTION("basic usage of and_then")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 2 * init_value);
        }

        SECTION("with no value")
        {
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{tl::make_unexpected(L"nope"s)}
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 3);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 8 * init_value);
        }

        SECTION("chaining with value and failing")
        {
            const double init_value = 1.2;
            std::size_t normal_lmbd_has_been_called = 0;
            std::size_t error_lmbd_has_been_called = 0;
            const std::wstring error = L"error!";
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambda(normal_lmbd_has_been_called))
                                 .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
                                 .and_then(Testing::makeFailableTimesTwoLambda(normal_lmbd_has_been_called))
                                 .get();
            REQUIRE(normal_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == error);
        }

        SECTION("with type change")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == std::to_wstring(2 * init_value));
        }

        SECTION("with type change and failing")
        {
            const double init_value = 1.2;
            std::size_t normal_lmbd_has_been_called = 0;
            std::size_t error_lmbd_has_been_called = 0;
            const std::wstring error = L"error!";
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambda(normal_lmbd_has_been_called))
                                 .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(normal_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == error);
        }

        SECTION("with conversion")
        {
            const double init_value = 2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<int, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 2 * init_value);
        }
    }

    SECTION("basic or_else usage")
    {
        const double init_value = 1.2;
        std::size_t error_lmbd_has_been_called = 0;
        std::size_t final_lmbd_has_been_called = 0;
        const std::wstring error = L"error!";
        expected_task::expected_task<double, std::wstring>{init_value}
            .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
            .or_else(
                [&final_lmbd_has_been_called, &error](auto err)
                {
                    final_lmbd_has_been_called++;
                    CHECK(error == err);
                })
            .wait();
        REQUIRE(final_lmbd_has_been_called == 1);
        REQUIRE(error_lmbd_has_been_called == 1);
    }

    SECTION("basic map_error usage")
    {

        SECTION("with same type")
        {
            const double init_value = 1.2;
            std::size_t error_lmbd_has_been_called = 0;
            std::size_t map_lmbd_has_been_called = 0;
            const std::wstring error = L"error!";
            const std::wstring prefix = L"new ";
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
                                 .map_error(
                                     [&map_lmbd_has_been_called, &prefix](const std::wstring err) -> std::wstring
                                     {
                                         map_lmbd_has_been_called++;
                                         return prefix + err;
                                     })
                                 .get();
            CHECK(std::is_same_v<std::wstring, typename decltype(res)::error_type>);
            REQUIRE(map_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == prefix + error);
        }

        SECTION("with type conversion")
        {
            const double init_value = 1.2;
            std::size_t error_lmbd_has_been_called = 0;
            const std::wstring error = L"error!";
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
                                 .map_error(&convertStr)
                                 .get();
            CHECK(std::is_same_v<std::string, typename decltype(res)::error_type>);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == convertStr(error));
        }
    }

    SECTION("map_error with tasks")
    {

        SECTION("with same type")
        {
            const double init_value = 1.2;
            std::size_t error_lmbd_has_been_called = 0;
            std::size_t map_lmbd_has_been_called = 0;
            const std::wstring error = L"error!";
            const std::wstring prefix = L"new ";
            const auto res
                = expected_task::expected_task<double, std::wstring>{init_value}
                      .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
                      .map_error(
                          [&map_lmbd_has_been_called, &prefix](const std::wstring err) -> pplx::task<std::wstring>
                          {
                              map_lmbd_has_been_called++;
                              return pplx::task_from_result(prefix + err);
                          })
                      .get();
            static_assert(std::is_same_v<std::wstring, typename decltype(res)::error_type>, "the error type is wrong");
            static_assert(std::is_same_v<double, typename decltype(res)::value_type>, "the value type is wrong");
            REQUIRE(map_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == prefix + error);
        }

        SECTION("with type conversion")
        {
            const double init_value = 1.2;
            std::size_t error_lmbd_has_been_called = 0;
            const std::wstring error = L"error!";
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailLambda<double>(error_lmbd_has_been_called, error))
                                 .map_error(&convertStrTask)
                                 .get();
            static_assert(std::is_same_v<std::string, typename decltype(res)::error_type>, "the error type is wrong");
            static_assert(std::is_same_v<double, typename decltype(res)::value_type>, "the value type is wrong");
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == convertStr(error));
        }
    }

    SECTION("basic use of then_return_value_or_convert_error_to_value")
    {

        SECTION("with a value")
        {
            const double init_value = 1.2;
            std::size_t convert_lmbd_has_been_called = 0;
            const auto res
                = expected_task::expected_task<double, std::wstring>{init_value}
                      .then_return_value_or_convert_error_to_value(makeConvertLambda(convert_lmbd_has_been_called))
                      .get();
            static_assert(std::is_same_v<decltype(res), const double>);
            CHECK(res == init_value);
            CHECK(convert_lmbd_has_been_called == 0);
        }

        SECTION("with an error")
        {
            const std::wstring error = L"5.2";
            std::size_t convert_lmbd_has_been_called = 0;
            const auto res
                = expected_task::expected_task<double, std::wstring>{tl::make_unexpected(error)}
                      .then_return_value_or_convert_error_to_value(makeConvertLambda(convert_lmbd_has_been_called))
                      .get();
            static_assert(std::is_same_v<decltype(res), const double>);
            CHECK(res == std::stod(error));
            CHECK(convert_lmbd_has_been_called == 1);
        }
    }

    SECTION("basic use of then_return_value_or_convert_error_to_value with a function returning a task")
    {

        SECTION("with a value")
        {
            const double init_value = 1.2;
            std::size_t convert_lmbd_has_been_called = 0;
            const auto res
                = expected_task::expected_task<double, std::wstring>{init_value}
                      .then_return_value_or_convert_error_to_value(makeConvertLambdaT(convert_lmbd_has_been_called))
                      .get();
            static_assert(std::is_same_v<decltype(res), const double>);
            CHECK(res == init_value);
            CHECK(convert_lmbd_has_been_called == 0);
        }

        SECTION("with an error")
        {
            const std::wstring error = L"5.2";
            std::size_t convert_lmbd_has_been_called = 0;
            const auto res
                = expected_task::expected_task<double, std::wstring>{tl::make_unexpected(error)}
                      .then_return_value_or_convert_error_to_value(makeConvertLambdaT(convert_lmbd_has_been_called))
                      .get();
            static_assert(std::is_same_v<decltype(res), const double>);
            CHECK(res == std::stod(error));
            CHECK(convert_lmbd_has_been_called == 1);
        }
    }
}
