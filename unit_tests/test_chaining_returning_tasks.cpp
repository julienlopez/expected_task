#include <catch2/catch.hpp>

#include <string>

#include <expected_task/expected_task.hpp>

using namespace std::string_literals;

namespace
{

auto makeTimesTwoLambda(std::size_t& has_been_called)
{
    return [&has_been_called](double value) -> pplx::task<double> {
        has_been_called++;
        return pplx::task_from_result(2 * value);
    };
}

auto makeFailableTimesTwoLambdaET(std::size_t& has_been_called)
{
    return [&has_been_called](double value) {
        has_been_called++;
        return expected_task::expected_task<double, std::wstring>{2 * value};
    };
}

template <class T, class Err> auto makeFailLambdaET(std::size_t& has_been_called, Err error)
{
    return [&has_been_called, e = std::move(error)](double) mutable -> expected_task::expected_task<T, Err> {
        has_been_called++;
        return tl::make_unexpected(std::move(e));
    };
}

auto makeFailableTimesTwoLambdaT(std::size_t& has_been_called)
{
    return [&has_been_called](double value) {
        has_been_called++;
        return pplx::task_from_result(tl::expected<double, std::wstring>{2 * value});
    };
}

template <class T, class Err> auto makeFailLambdaT(std::size_t& has_been_called, Err error)
{
    return [&has_been_called, e = std::move(error)](double) mutable {
        has_been_called++;
        return pplx::task_from_result(tl::expected<T, Err>{tl::make_unexpected(std::move(e))});
    };
}

} // namespace

TEST_CASE("Test expected_task with complex chaining", "[task]")
{

    SECTION("basic usage of then_map with functions returning tasks")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .then_map(makeTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(std::is_same_v<decltype(res), const tl::expected<double, std::wstring>>);
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == 2 * init_value);
        }

        SECTION("with no value")
        {
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{tl::make_unexpected(L"nope"s)}
                                 .then_map(makeTimesTwoLambda(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .then_map(makeTimesTwoLambda(has_been_called))
                                 .then_map(makeTimesTwoLambda(has_been_called))
                                 .then_map(makeTimesTwoLambda(has_been_called))
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
                                 .then_map(makeTimesTwoLambda(has_been_called))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(has_been_called == 1);
            REQUIRE(res.has_value());
            const auto final_value = *res;
            CHECK(final_value == std::to_wstring(2 * init_value));
        }
    }

    SECTION("basic usage of and_then with complex chaining returning another expected_task")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(makeFailableTimesTwoLambdaET(has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaET(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(makeFailableTimesTwoLambdaET(has_been_called))
                                 .and_then(makeFailableTimesTwoLambdaET(has_been_called))
                                 .and_then(makeFailableTimesTwoLambdaET(has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaET(normal_lmbd_has_been_called))
                                 .and_then(makeFailLambdaET<double>(error_lmbd_has_been_called, error))
                                 .and_then(makeFailableTimesTwoLambdaET(normal_lmbd_has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaET(has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaET(normal_lmbd_has_been_called))
                                 .and_then(makeFailLambdaET<double>(error_lmbd_has_been_called, error))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(normal_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == error);
        }
    }

    SECTION("basic usage of and_then with complex chaining returning a task<expected<.,.>>")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(makeFailableTimesTwoLambdaT(has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaT(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(makeFailableTimesTwoLambdaT(has_been_called))
                                 .and_then(makeFailableTimesTwoLambdaT(has_been_called))
                                 .and_then(makeFailableTimesTwoLambdaT(has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaT(normal_lmbd_has_been_called))
                                 .and_then(makeFailLambdaT<double>(error_lmbd_has_been_called, error))
                                 .and_then(makeFailableTimesTwoLambdaT(normal_lmbd_has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaT(has_been_called))
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
                                 .and_then(makeFailableTimesTwoLambdaT(normal_lmbd_has_been_called))
                                 .and_then(makeFailLambdaT<double>(error_lmbd_has_been_called, error))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(normal_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == error);
        }
    }
}
