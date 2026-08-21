#pragma once

#include "Error.hpp"

#include <expected>
#include <type_traits>
#include <utility>

namespace mir4d
{

template <typename T>
using Result = std::expected<T, Error>;

using Status = std::expected<void, Error>;

template <typename T>
[[nodiscard]] Result<std::decay_t<T>> success(T&& value)
{
    return Result<std::decay_t<T>>(std::forward<T>(value));
}

[[nodiscard]] inline Status success()
{
    return {};
}

[[nodiscard]] inline Error failure(ErrorCode code, std::string message)
{
    return Error{code, std::move(message)};
}

} // namespace mir4d
