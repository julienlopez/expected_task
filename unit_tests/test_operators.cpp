#include <catch2/catch.hpp>

#include "utilities.hpp"

#include <expected_task/expected_task_operators.hpp>

using Task = expected_task::expected_task<double, std::wstring>;

TEST_CASE("testing operators for expected_tasks", "[operators]")
{

    SECTION(">=")
    {
        const double init_value = 1.2;
        std::size_t has_been_called = 0;
        const std::wstring error = L"error!";

        auto task = Task{init_value} >= Testing::makeTimesTwoLambda(has_been_called);
        const auto res = task.get();

        REQUIRE(has_been_called == 1);
        REQUIRE(res.has_value());
        const auto final_value = *res;
        CHECK(final_value == 2 * init_value);
    }

    SECTION(">==")
    {
        const double init_value = 1.2;
        std::size_t has_been_called = 0;
        const std::wstring error = L"error!";

        auto task = Task{init_value} >>= Testing::makeFailableTimesTwoLambda(has_been_called);
        const auto res = task.get();

        REQUIRE(has_been_called == 1);
        REQUIRE(res.has_value());
        const auto final_value = *res;
        CHECK(final_value == 2 * init_value);
    }

    SECTION("|=")
    {
        std::size_t map_lmbd_has_been_called = 0;
        const std::wstring error = L"error!";
        const std::wstring prefix = L"new ";

        auto task = Task{tl::make_unexpected(error)} |= [&map_lmbd_has_been_called, &prefix](const std::wstring err)
        {
            map_lmbd_has_been_called++;
            return prefix + err;
        };

        const auto res = task.get();

        REQUIRE(map_lmbd_has_been_called == 1);
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == prefix + error);
    }

    SECTION("||")
    {
        std::size_t final_lmbd_has_been_called = 0;
        const std::wstring error = L"error!";
        const auto task = Task{tl::make_unexpected(error)} ||
                          [&final_lmbd_has_been_called, &error](auto err)
        {
            final_lmbd_has_been_called++;
            CHECK(error == err);
        };
        task.wait();
        REQUIRE(final_lmbd_has_been_called == 1);
    }
}
