#include <catch2/catch.hpp>

#include <expected_task/when_all.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace std::string_literals;

namespace
{
using Task = expected_task::expected_task<int, std::wstring>;
using Expected = typename Task::expected_type;

Task makeTask(const int value)
{
    return Expected{value};
}

Task makeError(std::wstring error)
{
    return tl::make_unexpected(std::move(error));
}

} // namespace

TEST_CASE("Test expected_task chaining with when_all", "[when_all]")
{
    SECTION("when_all with all tasks successfull")
    {
        const std::vector<int> values{1, 2, 3, 4, 5};
        std::vector<Task> tasks(values.size());
        std::transform(begin(values), end(values), std::begin(tasks), &makeTask);

        const auto res = when_all(tasks).get();
        REQUIRE(res.has_value());
        CHECK(res == values);
    }

    SECTION("when_all with one error")
    {
        const auto error = L"error"s;
        std::vector<Task> tasks({makeTask(1), makeTask(2), makeError(error), makeTask(3)});
        const auto res = when_all(tasks).get();
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == error);
    }

    SECTION("when_all with two errors")
    {
        const auto error1 = L"error1"s;
        const auto error2 = L"error2"s;
        std::vector<Task> tasks({makeTask(1), makeError(error1), makeError(error2), makeTask(3)});
        const auto res = when_all(tasks).get();
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == error1 + L" && " + error2);
    }

    SECTION("when_all all errors")
    {
        const auto error1 = L"error1"s;
        const auto error2 = L"error2"s;
        const auto error3 = L"error3"s;
        std::vector<Task> tasks({makeError(error1), makeError(error2), makeError(error3)});
        const auto res = when_all(tasks).get();
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == error1 + L" && " + error2 + L" && " + error3);
    }
}

TEST_CASE("Test expected_task chaining with the && operator", "[when_all]")
{
    SECTION(" with all tasks successfull")
    {
        const auto res = (makeTask(1) && makeTask(2)).get();
        REQUIRE(res.has_value());
        CHECK(res == std::vector{1, 2});
    }

    SECTION("with one error first")
    {
        const auto error = L"error"s;
        const auto res = (makeError(error) && makeTask(1)).get();
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == error);
    }

    SECTION("with one error last")
    {
        const auto error = L"error"s;
        const auto res = (makeTask(1) && makeError(error)).get();
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == error);
    }

    SECTION("with two errors")
    {
        const auto error1 = L"error1"s;
        const auto error2 = L"error2"s;
        const auto res = (makeError(error1) && makeError(error2)).get();
        REQUIRE_FALSE(res.has_value());
        CHECK(res.error() == error1 + L" && " + error2);
    }
}
