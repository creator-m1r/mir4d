#pragma once

#include <string>
#include <string_view>

namespace mir4d
{

enum class ErrorCode
{
    None = 0,
    InvalidArgument,
    InvalidState,
    NotFound,
    AlreadyExists,
    Conflict,
    OutOfRange,
    Unsupported,
    ValidationFailed,
    GeometryFailed,
    TopologyFailed,
    IoFailed,
    SerializationFailed,
    Internal
};

class Error
{
public:
    constexpr Error() noexcept = default;

    Error(ErrorCode code, std::string_view message)
        : code_(code), message_(message)
    {
    }

    [[nodiscard]] constexpr ErrorCode code() const noexcept { return code_; }
    [[nodiscard]] const std::string& message() const noexcept { return message_; }
    [[nodiscard]] explicit constexpr operator bool() const noexcept
    {
        return code_ != ErrorCode::None;
    }

private:
    ErrorCode code_{ErrorCode::None};
    std::string message_{};
};

}
