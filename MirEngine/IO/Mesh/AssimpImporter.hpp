#pragma once

#include "../Importer.hpp"

namespace mir::io
{

class AssimpImporter final : public Importer
{
public:
    [[nodiscard]] ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const override;
};

}
