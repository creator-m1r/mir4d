#pragma once

#include <cstdint>
#include <string_view>

namespace MirEngine::Rendering
{

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

struct MaterialData
{
    const char* name{""};
    float baseColor[3]{0.7F, 0.7F, 0.7F};
    float roughness{0.5F};
    float metallic{0.0F};
    float specular{0.5F};
    float ior{1.5F};
    float emission[3]{0.0F, 0.0F, 0.0F};
    float opacity{1.0F};
};

namespace MaterialLibrary
{

constexpr MaterialData kMaterials[MaterialCount] = {

    {"Engineering Steel", {0.55F, 0.58F, 0.62F}, 0.38F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Stainless Steel", {0.66F, 0.68F, 0.71F}, 0.30F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Aluminium", {0.78F, 0.79F, 0.80F}, 0.34F, 1.0F, 0.4F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Copper", {0.88F, 0.52F, 0.35F}, 0.36F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Brass", {0.83F, 0.70F, 0.36F}, 0.34F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Titanium", {0.60F, 0.61F, 0.65F}, 0.45F, 1.0F, 0.5F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"ABS Plastic", {0.25F, 0.27F, 0.30F}, 0.55F, 0.0F, 0.5F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Rubber", {0.13F, 0.13F, 0.14F}, 0.90F, 0.0F, 0.2F, 1.4F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Glass", {0.70F, 0.78F, 0.82F}, 0.05F, 0.0F, 1.0F, 1.52F, {0.0F, 0.0F, 0.0F}, 0.85F},

    {"Concrete", {0.58F, 0.57F, 0.54F}, 0.92F, 0.0F, 0.3F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Wood", {0.55F, 0.40F, 0.24F}, 0.80F, 0.0F, 0.3F, 1.5F, {0.0F, 0.0F, 0.0F}, 1.0F},

    {"Ceramic", {0.86F, 0.84F, 0.80F}, 0.25F, 0.0F, 0.7F, 1.6F, {0.0F, 0.0F, 0.0F}, 1.0F},

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

[[nodiscard]] inline MaterialId findByName(std::string_view name) noexcept
{
    for (int i = 0; i < MaterialCount; ++i)
    {
        if (name == kMaterials[i].name)
            return static_cast<MaterialId>(i);
    }
    return defaultMaterial();
}

}

}