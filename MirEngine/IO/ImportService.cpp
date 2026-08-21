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
}

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

std::map<Format, ImportService::ImporterFn>& ImportService::registry()
{
    static std::map<Format, ImporterFn> instance;
    return instance;
}

void ImportService::registerImporter(Format format, ImporterFn fn)
{
    registry()[format] = std::move(fn);
}

bool ImportService::hasImporter(Format format)
{
    return registry().find(format) != registry().end();
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

    const auto it = registry().find(format);
    if (it != registry().end())
        return it->second(path, options);

    if (format == Format::Iges)
    {
        result.error = "IGES importer is reserved for the optional OCCT bridge.";
    }
    else
    {
        result.error = "Unsupported import format: " + path;
    }

    return result;
}

}
