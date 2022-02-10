#pragma once

#include <tl/expected.hpp>

#include <boost/optional.hpp>

namespace expected_task
{

template <class Value, class Error> tl::expected<Value, Error> from_optional(boost::optional<Value> value, Error&& error)
{
    if(value)
        return tl::expected<Value, Error>{std::forward<Value>(*value)};
    else
        return tl::make_unexpected(std::forward<Error>(error));
}

} // namespace expected_task
