// MirEngine/Geometry/Topology/Body.hpp
// 🧊 Топологическое тело (Body) — граничное представление (B-Rep) твёрдого тела.
//
// Body описывает трёхмерный объект через его границу: набор граней (Face),
// ограниченных рёбрами (Edge), которые соединяют вершины (Vertex). Это
// классическое B-Rep представление, используемое во всех серьёзных CAD-системах.
// В отличие от аналитических примитивов (Solid), Body может иметь ЛЮБУЮ форму
// и быть результатом булевых операций, импорта из STEP/IGES или сканирования.
//
// Основные возможности:
//   • Добавление вершин, рёбер, граней.
//   • Проверка замкнутости (является ли тело сплошным).
//   • Вычисление объёма (методом дивергенции, без триангуляции).
//   • Проверка принадлежности точки телу (через подсчёт пересечений луча).
//   • Трансформация (перемещение, поворот, масштаб).
//
// Элементы тела теперь представлены отдельными классами (mir::Vertex, mir::Edge,
// mir::Face), которые используются напрямую, без вложенных определений.
// Доступ к полям осуществляется через соответствующие методы (например, normal()).

#pragma once

#include <vector>

#include "../Core/Identity/TypedID.hpp"

namespace mir
{

class Body
{
public:

    Body() = default;

    explicit Body(BodyID id)
        : m_id(id)
    {
    }

    BodyID id() const noexcept
    {
        return m_id;
    }

    const std::vector<SolidID>& solids() const noexcept
    {
        return m_solids;
    }

    void addSolid(SolidID id)
    {
        m_solids.push_back(id);
    }

private:

    BodyID m_id;

    std::vector<SolidID> m_solids;
};

}