#include <catch2/catch.hpp>

#include <string>

#include "utilities.hpp"

using namespace std::string_literals;


TEST_CASE("Test expected_task with complex chaining", "[task]")
{

    SECTION("basic usage of then_map with functions returning tasks")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .then_map(Testing::makeTimesTwoLambda(has_been_called))
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
    }

    SECTION("basic usage of and_then with complex chaining returning another expected_task")
    {

        SECTION("with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(has_been_called))
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(has_been_called))
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(normal_lmbd_has_been_called))
                                 .and_then(Testing::makeFailLambdaET<double>(error_lmbd_has_been_called, error))
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(normal_lmbd_has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaET(normal_lmbd_has_been_called))
                                 .and_then(Testing::makeFailLambdaET<double>(error_lmbd_has_been_called, error))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(has_been_called))
                                 .get();
            REQUIRE(has_been_called == 0);
            REQUIRE_FALSE(res.has_value());
        }

        SECTION("chaining with value")
        {
            const double init_value = 1.2;
            std::size_t has_been_called = 0;
            const auto res = expected_task::expected_task<double, std::wstring>{init_value}
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(has_been_called))
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(has_been_called))
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(normal_lmbd_has_been_called))
                                 .and_then(Testing::makeFailLambdaT<double>(error_lmbd_has_been_called, error))
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(normal_lmbd_has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(has_been_called))
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
                                 .and_then(Testing::makeFailableTimesTwoLambdaT(normal_lmbd_has_been_called))
                                 .and_then(Testing::makeFailLambdaT<double>(error_lmbd_has_been_called, error))
                                 .then_map(static_cast<std::wstring (*)(double)>(&std::to_wstring))
                                 .get();
            REQUIRE(normal_lmbd_has_been_called == 1);
            REQUIRE(error_lmbd_has_been_called == 1);
            REQUIRE_FALSE(res.has_value());
            CHECK(res.error() == error);
        }
    }
}
