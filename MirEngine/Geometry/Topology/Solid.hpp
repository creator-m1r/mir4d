// MirEngine/Geometry/Topology/Solid.hpp
// 🧊 Твёрдое тело (Solid) — топологическое представление трёхмерного объекта.
//
// Solid — это вершина иерархии топологического представления. Оно состоит
// из одной или нескольких оболочек (Shell), которые вместе образуют
// замкнутую поверхность, ограничивающую объём. Каждая оболочка содержит
// грани (Face), ограниченные контурами (Loop) из рёбер (Edge), которые
// соединяют вершины (Vertex).
//
// Основные свойства:
//   • shells       — оболочки, образующие тело (обычно одна внешняя + опциональные внутренние).
//   • isClosed()   — проверяет, замкнуто ли тело (все оболочки замкнуты).
//   • volume()     — вычисляет объём тела (сумма объёмов внешних оболочек минус внутренние).
//   • contains(pt) — проверяет, находится ли точка внутри тела.
//   • transform()  — применяет трансформацию ко всем вершинам.
//   • toBody()     — конвертирует в низкоуровневый Body (B-Rep) для операций.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <vector>

#include "../Core/Identity/TypedID.hpp"

namespace mir
{

class Solid
{
public:

    Solid() = default;

    explicit Solid(SolidID id)
        : m_id(id)
    {
    }

    SolidID id() const noexcept
    {
        return m_id;
    }

    const std::vector<ShellID>& shells() const noexcept
    {
        return m_shells;
    }

    void addShell(ShellID id)
    {
        m_shells.push_back(id);
    }

private:

    SolidID m_id;

    std::vector<ShellID> m_shells;
};

}