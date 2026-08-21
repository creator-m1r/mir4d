#pragma once

#include <string_view>

namespace mir::io
{

enum class Format
{
    Unknown,
    StlAscii,
    StlBinary,
    Obj,
    Ply,
    Gltf,
    Glb,
    Fbx,
    Step,
    Iges
};

[[nodiscard]] constexpr std::string_view formatName(Format format) noexcept
{
    switch (format)
    {
        case Format::StlAscii: return "STL ASCII";
        case Format::StlBinary: return "STL Binary";
        case Format::Obj: return "OBJ";
        case Format::Ply: return "PLY";
        case Format::Gltf: return "glTF";
        case Format::Glb: return "GLB";
        case Format::Fbx: return "FBX";
        case Format::Step: return "STEP";
        case Format::Iges: return "IGES";
        default: return "Unknown";
    }
}

[[nodiscard]] constexpr bool isMeshFormat(Format format) noexcept
{
    return format == Format::StlAscii ||
           format == Format::StlBinary ||
           format == Format::Obj ||
           format == Format::Ply ||
           format == Format::Gltf ||
           format == Format::Glb ||
           format == Format::Fbx;
}

} // namespace mir::io
