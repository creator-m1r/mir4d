#pragma once

#include <cstdint>
#include <string_view>

namespace MirEngine::Rendering
{

/// Material id referenced by objects. A material is data, not an image:
/// all appearance comes from scalar/vector parameters evaluated in shaders.
enum MaterialId : std::int32_t
{
    MaterialEngineeringSteel = 0,
    MaterialStainlessSteel,
    MaterialAluminium,
    MaterialCopper,
    MaterialBrass,
    MaterialTitanium,
    MaterialABSPlastic,
    MaterialRubber,
    MaterialGlass,
    MaterialConcrete,
    MaterialWood,
    MaterialCeramic,
    MaterialPaintedMetal,
    MaterialCount
};

/// Procedural PBR material parameters (no texture images).
/// baseColor/reflectance: linear RGB albedo.
struct MaterialData
{
    const char* name{""};
    float baseColor[3]{0.7F, 0.7F, 0.7F};
    float roughness{0.5F};    ///< 0 = mirror, 1 = fully rough
    float metallic{0.0F};     ///< 0 = dielectric, 1 = metal
    float specular{0.5F};     ///< dielectric specular strength
    float ior{1.5F};          ///< index of refraction
    float emission[3]{0.0F, 0.0F, 0.0F};
    float opacity{1.0F};
};

/// Canonical engineering material library.
/// All presets are procedural: no PNG/JPG/TIFF textures are required.
namespace MaterialLibrary
{

constexpr MaterialData kMaterials[MaterialCount] = {
    // Engineering Steel — the default engineering material.
    {"Engineering Steel", {0.55F, 0.58F, 0.62F}, 0.38F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Stainless Steel — brighter, smoother.
    {"Stainless Steel", {0.66F, 0.68F, 0.71F}, 0.30F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Aluminium — light, low-luminance metal.
    {"Aluminium", {0.78F, 0.79F, 0.80F}, 0.34F, 1.0F, 0.4F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Copper — warm orange metal.
    {"Copper", {0.88F, 0.52F, 0.35F}, 0.36F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Brass — yellow metal.
    {"Brass", {0.83F, 0.70F, 0.36F}, 0.34F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Titanium — grey metal with slight blue tint.
    {"Titanium", {0.60F, 0.61F, 0.65F}, 0.45F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // ABS Plastic — opaque dielectric.
    {"ABS Plastic", {0.25F, 0.27F, 0.30F}, 0.55F, 0.0F, 0.5F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Rubber — dark, rough dielectric.
    {"Rubber", {0.13F, 0.13F, 0.14F}, 0.90F, 0.0F, 0.2F, 1.4F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Glass — transparent dielectric (opacity handled by the pipeline).
    {"Glass", {0.70F, 0.78F, 0.82F}, 0.05F, 0.0F, 1.0F, 1.52F, {0.0F, 0.0F, 0.0F}, 0.85F},
    // Concrete — rough grey building material.
    {"Concrete", {0.58F, 0.57F, 0.54F}, 0.92F, 0.0F, 0.3F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Wood — warm dielectric.
    {"Wood", {0.55F, 0.40F, 0.24F}, 0.80F, 0.0F, 0.3F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Ceramic — smooth dielectric.
    {"Ceramic", {0.86F, 0.84F, 0.80F}, 0.25F, 0.0F, 0.7F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},
    // Painted Metal — metal base under a dielectric clear coat look.
    {"Painted Metal", {0.32F, 0.55F, 0.62F}, 0.42F, 0.4F, 0.6F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},
};

[[nodiscard]] constexpr MaterialId defaultMaterial() noexcept { return MaterialEngineeringSteel; }

[[nodiscard]] constexpr MaterialData material(MaterialId id) noexcept
{
    const std::int32_t index =
        id < 0 ? 0 : (id >= MaterialCount ? MaterialCount - 1 : static_cast<std::int32_t>(id));
    return kMaterials[index];
}

[[nodiscard]] constexpr int count() noexcept { return MaterialCount; }

[[nodiscard]] inline const char* name(MaterialId id) noexcept { return material(id).name; }

/// Resolves a material id by case-insensitive name; returns default when absent.
[[nodiscard]] inline MaterialId findByName(std::string_view name) noexcept
{
    for (int i = 0; i < MaterialCount; ++i)
    {
        if (name == kMaterials[i].name)
            return static_cast<MaterialId>(i);
    }
    return defaultMaterial();
}

} // namespace MaterialLibrary

} // namespace MirEngine::Rendering