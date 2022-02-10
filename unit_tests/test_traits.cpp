#include <catch2/catch.hpp>

#include <string>

#include <expected_task/expected_task.hpp>

TEST_CASE("Test expected_task's utility functions", "[task]")
{
    using expected_task_type = expected_task::expected_task<double, std::wstring>;
    using expected_type = tl::expected<double, std::wstring>;

    SECTION("expected_task::details::is_task_v")
    {
        CHECK_FALSE(expected_task::details::is_task_v<double>);
        CHECK(expected_task::details::is_task_v<pplx::task<double>>);
        CHECK(expected_task::details::is_task_v<pplx::task<expected_type>>);
        CHECK_FALSE(expected_task::details::is_task_v<expected_task_type>);
    }

    SECTION("expected_task::details::is_expected_task_v")
    {
        CHECK_FALSE(expected_task::details::is_expected_task_v<double>);
        CHECK_FALSE(expected_task::details::is_expected_task_v<pplx::task<double>>);
        CHECK(expected_task::details::is_expected_task_v<expected_task_type>);
    }

    SECTION("expected_task::details::is_expected_v")
    {
        CHECK_FALSE(expected_task::details::is_expected_v<double>);
        CHECK(expected_task::details::is_expected_v<expected_type>);
        CHECK(expected_task::details::is_expected_v<expected_task_type::expected_type>);
    }
}
