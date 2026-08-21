#pragma once

#include "../Importer.hpp"

namespace mir::io
{

/// Optional mesh/asset importer backed by Assimp.
///
/// Assimp is deliberately kept outside Core/Geometry/BRep. The importer
/// produces canonical TriangleMesh3 data and never creates Scene/Node/GPU objects.
class AssimpImporter final : public Importer
{
public:
    [[nodiscard]] ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const override;
};

} // namespace mir::io
