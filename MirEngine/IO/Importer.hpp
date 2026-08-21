#pragma once

#include "ImportOptions.hpp"
#include "ImportResult.hpp"

#include <string>

namespace mir::io
{

/// Canonical format-agnostic import contract.
/// IO returns CPU engineering data only; it never creates Scene, GPU or UI state.
class Importer
{
public:
    virtual ~Importer() = default;

    [[nodiscard]] virtual ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const = 0;
};

} // namespace mir::io
