#include <catch2/catch.hpp>

#include <string>

#include <expected_task/expected_task.hpp>

using namespace std::string_literals;

namespace
{

using Task = expected_task::expected_task<void, std::wstring>;

Task createVoidExpectedTask()
{
    return expected_task::create_task<std::wstring>([]() {});
}

template <class T> auto makeSimpleLambda(const T& return_value, std::size_t& has_been_called)
{
    return [return_value, &has_been_called]() -> T {
        has_been_called++;
        return return_value;
    };
}

auto makeSimpleLambda(std::size_t& has_been_called)
{
    return [&has_been_called]() { has_been_called++; };
}

template <class T> auto makeTaskLambda(const T& return_value, std::size_t& has_been_called)
{
    return [return_value, &has_been_called]() -> pplx::task<T> {
        has_been_called++;
        return pplx::task_from_result(return_value);
    };
}

auto makeTaskLambda(std::size_t& has_been_called)
{
    return [&has_been_called]() -> pplx::task<void> {
        has_been_called++;
        return pplx::create_task([]() {});
    };
}

template <class T, class E = std::wstring> auto makeExpectedLambda(const T& return_value, std::size_t& has_been_called)
{
    return [return_value, &has_been_called]() -> tl::expected<T, E> {
        has_been_called++;
        return return_value;
    };
}

template <class E = std::wstring> auto makeExpectedLambda(std::size_t& has_been_called)
{
    return [&has_been_called]() {
        has_been_called++;
        return tl::expected<void, E>{};
    };
}

template <class T, class E = std::wstring>
auto makeExpectedTaskLambda(const T& return_value, std::size_t& has_been_called)
{
    return [return_value, &has_been_called]() -> expected_task::expected_task<T, E> {
        has_been_called++;
        return return_value;
    };
}

template <class E = std::wstring> auto makeExpectedTaskLambda(std::size_t& has_been_called)
{
    return [&has_been_called]() -> expected_task::expected_task<void, E> {
        has_been_called++;
        return {pplx::create_task([]() {})};
    };
}

template <class T, class E = std::wstring>
auto makeTaskOfExpectedLambda(const T& return_value, std::size_t& has_been_called)
{
    return [return_value, &has_been_called]() {
        has_been_called++;
        return pplx::task_from_result(tl::expected<T, E>{return_value});
    };
}

template <class E = std::wstring> auto makeTaskOfExpectedLambda(std::size_t& has_been_called)
{
    return [&has_been_called]() {
        has_been_called++;
        return pplx::task_from_result(tl::expected<void, E>{});
    };
}

} // namespace

TEST_CASE("Test expected_task with void return", "[task]")
{

    SECTION("simple construction")
    {
        Task simple_task = createVoidExpectedTask();
        CHECK(simple_task.get().has_value());
    }

    SECTION("then_map on a void ET with a normal lambda")
    {
        const auto return_value = 12;
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().then_map(makeSimpleLambda(return_value, has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(*task == return_value);
        CHECK(has_been_called == 1);
    }

    SECTION("then_map on a void ET with a task returning lambda")
    {
        const auto return_value = 12;
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().then_map(makeTaskLambda(return_value, has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(*task == return_value);
        CHECK(has_been_called == 1);
    }

    SECTION("then_map chaining a normal ET with a void ET")
    {
        auto task = expected_task::expected_task<double, std::wstring>{25.2}.then_map([](const double) {}).get();
        REQUIRE(task.has_value());
    }

    SECTION("and_then on a void ET with a lambda returning a simple expected")
    {
        const auto return_value = 12;
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().and_then(makeExpectedLambda(return_value, has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(*task == return_value);
        CHECK(has_been_called == 1);
    }

    SECTION("and_then on a void ET with a lambda returning an ET")
    {
        const auto return_value = 12;
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().and_then(makeExpectedTaskLambda(return_value, has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(*task == return_value);
        CHECK(has_been_called == 1);
    }

    SECTION("and_then on a void ET with a lambda returning a pplx::task<expected<>>")
    {
        const auto return_value = 12;
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().and_then(makeTaskOfExpectedLambda(return_value, has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(*task == return_value);
        CHECK(has_been_called == 1);
    }

    SECTION("and_then chaining a normal ET with a void ET")
    {
        auto task = expected_task::expected_task<double, std::wstring>{25.2}
                        .and_then([](const double) -> expected_task::expected_task<void, std::wstring> {
                            return pplx::create_task([]() {});
                        })
                        .get();
        REQUIRE(task.has_value());
    }
}

TEST_CASE("Test multiple expected_task chained together with void return", "[task]")
{

    SECTION("then_map on a void ET with a normal lambda")
    {
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().then_map(makeSimpleLambda(has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(has_been_called == 1);
    }

    SECTION("then_map on a void ET with a task returning lambda")
    {
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().then_map(makeTaskLambda(has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(has_been_called == 1);
    }

    SECTION("then_map chaining a normal ET with two void ETs")
    {
        auto task = expected_task::expected_task<double, std::wstring>{25.2}
                        .then_map([](const double) {})
                        .then_map([]() {})
                        .get();
        REQUIRE(task.has_value());
    }

    SECTION("and_then on a void ET with a lambda returning a simple expected")
    {
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().and_then(makeExpectedLambda(has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(has_been_called == 1);
    }

    SECTION("and_then on a void ET with a lambda returning an ET")
    {
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().and_then(makeExpectedTaskLambda(has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(has_been_called == 1);
    }

    SECTION("and_then on a void ET with a lambda returning a pplx::task<expected<>>")
    {
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask().and_then(makeTaskOfExpectedLambda(has_been_called)).get();
        REQUIRE(task.has_value());
        CHECK(has_been_called == 1);
    }

    SECTION("and_then chaining a normal ET with two void ETs")
    {
        auto task = expected_task::expected_task<double, std::wstring>{25.2}
                        .and_then([](const double) -> expected_task::expected_task<void, std::wstring> {
                            return pplx::create_task([]() {});
                        })
                        .and_then([]() -> expected_task::expected_task<void, std::wstring> {
                            return pplx::create_task([]() {});
                        })
                        .get();
        REQUIRE(task.has_value());
    }

    SECTION("multiple mixed chaining with voids")
    {
        std::size_t has_been_called = 0;
        auto task = createVoidExpectedTask()
                        .and_then(makeTaskOfExpectedLambda(has_been_called))
                        .then_map([]() {})
                        .and_then(makeExpectedTaskLambda(has_been_called))
                        .get();
        REQUIRE(task.has_value());
        CHECK(has_been_called == 2);
    }
}
