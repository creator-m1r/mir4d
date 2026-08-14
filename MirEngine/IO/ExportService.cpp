#include "ExportService.hpp"

#include "Mesh/StlExporter.hpp"
#include "Step/StepExporter.hpp"

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

Format ExportService::detectFormat(const std::string& path) noexcept
{
    const std::string extension = lowerExtension(path);

    if (extension == ".stl") return Format::StlBinary;
    if (extension == ".step" || extension == ".stp") return Format::Step;
    if (extension == ".iges" || extension == ".igs") return Format::Iges;
    if (extension == ".obj") return Format::Obj;
    if (extension == ".ply") return Format::Ply;
    if (extension == ".gltf") return Format::Gltf;
    if (extension == ".glb") return Format::Glb;

    return Format::Unknown;
}

ExportResult ExportService::exportFile(
    const std::string& path,
    const mir4d::Document& document,
    const ExportOptions& options) const
{
    const Format format = detectFormat(path);

    if (format == Format::StlBinary || format == Format::StlAscii)
        return StlExporter{}.exportTo(path, document, options);

    if (format == Format::Step)
        return step::StepExporter{}.exportTo(path, document, options);

    ExportResult result;
    result.format = format;
    result.targetPath = path;

    if (format == Format::Iges)
    {
        result.error = "IGES exporter is reserved for the optional OCCT bridge.";
    }
    else if (isMeshFormat(format))
    {
        result.error = "Exporter is not enabled yet for: " +
                       std::string(formatName(format));
    }
    else
    {
        result.error = "Unsupported export format: " + path;
    }

    return result;
}

} // namespace mir::io
