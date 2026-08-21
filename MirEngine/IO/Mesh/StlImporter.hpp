#pragma once

#include "../ImportOptions.hpp"
#include "../ImportResult.hpp"

#include <string>

namespace mir::io
{

class StlImporter
{
public:
    [[nodiscard]] ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const;
};

}
