#include <catch2/catch.hpp>

#include <expected_task/expected_task.hpp>

#include <string>

#include <boost/hof.hpp>

using namespace boost::hof;

using Expected = tl::expected<int, std::wstring>;
using ExpectedTask = expected_task::expected_task<int, std::wstring>;

namespace
{

int add(const int i1, const int i2)
{
    return i1 + i2;
}

Expected exptectedAdd(const int i1, const int i2)
{
    return i1 + i2;
}

} // namespace

TEST_CASE("Test expected_task's high order functions with std::bind", "[hof]")
{
    auto lmbd_map = std::bind(&add, std::placeholders::_1, 2);

    SECTION("tl::expected::map")
    {
        const auto res = Expected{5}.map(lmbd_map);
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    SECTION("expected_task::then_map")
    {
        const auto res = ExpectedTask{5}.then_map(lmbd_map).get();
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }
}

TEST_CASE("Test expected_task's high order functions with boost::hof::lazy", "[hof]")
{
    auto lmbd = lazy(&add)(_1, 2);

    SECTION("tl::expected::map")
    {
        const auto res = Expected{5}.map(lmbd);
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    SECTION("expected_task::then_map")
    {
        const auto res = ExpectedTask{5}.then_map(lmbd).get();
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    auto lmbd_then = lazy(&exptectedAdd)(_1, 2);

    SECTION("tl::expected::and_then")
    {
        const auto res = Expected{5}.and_then(lmbd_then);
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    SECTION("expected_task::and_then")
    {
        const auto res = ExpectedTask{5}.and_then(lmbd_then).get();
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }
}

TEST_CASE("Test expected_task's high order functions with boost::hof::partial", "[hof]")
{
    auto lmbd = partial(&add)(2);

    SECTION("tl::expected::map")
    {
        const auto res = Expected{5}.map(lmbd);
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    SECTION("expected_task::then_map")
    {
        const auto res = ExpectedTask{5}.then_map(lmbd).get();
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    auto lmbd_then = partial(&exptectedAdd)(2);

    SECTION("tl::expected::and_then")
    {
        const auto res = Expected{5}.and_then(lmbd_then);
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }

    SECTION("expected_task::and_then")
    {
        const auto res = ExpectedTask{5}.and_then(lmbd_then).get();
        REQUIRE(res.has_value());
        CHECK(*res == 7);
    }
}
