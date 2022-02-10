#include <catch2/catch.hpp>

#include <expected_task/expected_task.hpp>
#include <expected_task/expected_optional_bridge.hpp>

using Task = expected_task::expected_task<int>;

using namespace std::string_literals;

namespace
{

Task makeTaskIf(const boost::optional<int>& value)
{
    return expected_task::from_optional(value, L"no"s);
}

} // namespace

TEST_CASE("budling an expected_task from tl::make_unexpected", "[task]")
{

    SECTION("basic test with value")
    {
        const int value = 7;
        const auto task = makeTaskIf(value).get();
        REQUIRE(task.has_value());
        CHECK(*task == value);
    }

    SECTION("basic test with unexpected")
    {
        const auto task = makeTaskIf(boost::none).get();
        REQUIRE_FALSE(task.has_value());
        CHECK(task.error() == L"no");
    }
}
