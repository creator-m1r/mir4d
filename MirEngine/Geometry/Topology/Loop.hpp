// MirEngine/Geometry/Topology/Loop.hpp
// 🔁 Топологический контур (Loop) — замкнутая цепочка рёбер, ограничивающая грань.
//
// Loop (он же Wire) — это упорядоченный набор рёбер, который образует
// замкнутую или разомкнутую петлю. В граничном представлении (B-Rep)
// каждый Face имеет один внешний контур (outer loop) и ноль или более
// внутренних контуров (inner loops — отверстия). Loop объединяет индексы
// рёбер в правильном порядке, гарантируя, что концы рёбер стыкуются.
//
// Основные свойства:
//   • edges         — индексы рёбер, образующих контур.
//   • isClosed()    — проверяет, замкнут ли контур (последнее ребро соединяется с первым).
//   • length()      — суммарная длина всех рёбер контура.
//   • area()        — площадь, ограниченная контуром (для плоских контуров).
//   • containsEdge() — проверяет, входит ли ребро в контур.
//   • reverse()     — меняет направление обхода контура на противоположное.
//
// Loop используется в:
//   • Face — как внешняя или внутренняя граница.
//   • Эскизах (Sketch) — для выдавливания и вращения.
//   • Построении сложных контуров из отрезков и дуг.
//   • Проверке замкнутости геометрии перед созданием твёрдого тела.
//
// Чистый C++23, без внешних зависимостей.


#pragma once

#include <vector>

#include "../Core/Identity/TypedID.hpp"

namespace mir
{

class Loop
{
public:

    Loop() = default;

    explicit Loop(LoopID id)
        : m_id(id)
    {
    }

    LoopID id() const noexcept
    {
        return m_id;
    }

    const std::vector<EdgeID>& edges() const noexcept
    {
        return m_edges;
    }

    void addEdge(EdgeID id)
    {
        m_edges.push_back(id);
    }

    bool isClosed() const noexcept
    {
        return m_closed;
    }

    void setClosed(bool value) noexcept
    {
        m_closed = value;
    }

private:

    LoopID m_id;

    std::vector<EdgeID> m_edges;

    bool m_closed = false;
};

}