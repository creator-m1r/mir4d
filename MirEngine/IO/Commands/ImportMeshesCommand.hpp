#pragma once

#include "../../Document/Command.hpp"
#include "../ImportOptions.hpp"

#include <string>
#include <utility>

namespace mir::io
{

/// Factory for the persistent document action used by mesh import.
/// Arguments are intentionally plain strings so Document history remains
/// serializable and independent from the IO implementation.
struct ImportMeshesCommand
{
    [[nodiscard]] static mir4d::Command make(
        mir4d::Time time,
        std::string path,
        const ImportOptions& options = {})
    {
        return mir4d::Command::make(
            0,
            time,
            mir4d::CommandType::ImportMeshes,
            mir4d::InvalidObjectId,
            {
                std::move(path),
                options.generateNormals ? "generateNormals=1" : "generateNormals=0",
                options.triangulate ? "triangulate=1" : "triangulate=0",
                options.joinIdenticalVertices ? "joinVertices=1" : "joinVertices=0",
                "unitScale=" + std::to_string(options.unitScale)});
    }
};

} // namespace mir::io
