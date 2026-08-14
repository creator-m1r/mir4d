#pragma once

#include "ImportOptions.hpp"
#include "ImportResult.hpp"

#include <string>

namespace mir::io
{

class ImportService
{
public:
    [[nodiscard]] ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const;

    [[nodiscard]] static Format detectFormat(const std::string& path) noexcept;
};

} // namespace mir::io
