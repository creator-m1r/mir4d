#pragma once

#include "ImportOptions.hpp"
#include "ImportResult.hpp"

#include <string>

namespace mir::io
{

class Importer
{
public:
    virtual ~Importer() = default;

    [[nodiscard]] virtual ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const = 0;
};

}
