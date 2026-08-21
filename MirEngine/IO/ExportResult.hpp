#pragma once

#include "Format.hpp"

#include <cstddef>
#include <string>

namespace mir::io
{

struct ExportResult
{
    Format format{Format::Unknown};
    std::string targetPath;
    std::string error;
    std::size_t triangleCount{0};

    [[nodiscard]] bool ok() const noexcept
    {
        return error.empty();
    }
};

}
