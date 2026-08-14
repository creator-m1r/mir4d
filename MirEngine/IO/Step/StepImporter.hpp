#pragma once

#include "../ImportOptions.hpp"
#include "../ImportResult.hpp"

#include <string>

namespace mir::io::step
{

class StepImporter
{
public:
    [[nodiscard]] ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const;
};

} // namespace mir::io::step
