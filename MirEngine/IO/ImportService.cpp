#include "ImportService.hpp"

#include "Mesh/StlImporter.hpp"
#include "Step/StepImporter.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace mir::io
{
namespace
{
[[nodiscard]] std::string lowerExtension(const std::string& path)
{
    std::string extension = std::filesystem::path(path).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character)
                   {
                       return static_cast<char>(std::tolower(character));
                   });
    return extension;
}
} // namespace

Format ImportService::detectFormat(const std::string& path) noexcept
{
    const std::string extension = lowerExtension(path);

    if (extension == ".stl") return Format::StlBinary;
    if (extension == ".obj") return Format::Obj;
    if (extension == ".ply") return Format::Ply;
    if (extension == ".gltf") return Format::Gltf;
    if (extension == ".glb") return Format::Glb;
    if (extension == ".fbx") return Format::Fbx;
    if (extension == ".step" || extension == ".stp") return Format::Step;
    if (extension == ".iges" || extension == ".igs") return Format::Iges;

    return Format::Unknown;
}

ImportResult ImportService::importFile(
    const std::string& path,
    const ImportOptions& options) const
{
    const Format format = detectFormat(path);

    if (format == Format::StlBinary)
        return StlImporter{}.importFile(path, options);

    if (format == Format::Step)
        return step::StepImporter{}.importFile(path, options);

    ImportResult result;
    result.format = format;
    result.sourcePath = path;

    if (format == Format::Iges)
    {
        result.error = "IGES importer is reserved for the optional OCCT bridge.";
    }
    else if (isMeshFormat(format))
    {
        result.error = "Mesh format is recognized but its canonical importer is not enabled yet: " +
                       std::string(formatName(format));
    }
    else
    {
        result.error = "Unsupported import format: " + path;
    }

    return result;
}

} // namespace mir::io
