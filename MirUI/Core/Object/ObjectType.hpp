#pragma once

#include <string_view>

namespace MirUI
{

enum class ObjectType
{
    Unknown,
    Document,
    Assembly,
    Part,
    Body,
    Sketch,
    Feature,
    Mesh,
    Face,
    Edge,
    Vertex,
    Solid
};

[[nodiscard]] constexpr std::string_view objectTypeName(ObjectType type) noexcept
{
    switch (type)
    {
        case ObjectType::Document: return "Документ";
        case ObjectType::Assembly: return "Сборка";
        case ObjectType::Part: return "Деталь";
        case ObjectType::Body: return "Тело";
        case ObjectType::Sketch: return "Эскиз";
        case ObjectType::Feature: return "Операция";
        case ObjectType::Mesh: return "Сетка";
        case ObjectType::Face: return "Грань";
        case ObjectType::Edge: return "Ребро";
        case ObjectType::Vertex: return "Вершина";
        case ObjectType::Solid: return "Твердое тело";
        case ObjectType::Unknown: break;
    }
    return "Неизвестно";
}

}
