// MirEngine/Geometry/Topology/Shell.hpp
// 🐚 Топологическая оболочка (Shell) — совокупность граней, образующих
//    замкнутую или открытую поверхность тела.
//
// В иерархии граничного представления (B-Rep) Shell находится на уровень
// выше Face. Она объединяет несколько граней, которые топологически
// связаны друг с другом через общие рёбра. Одно твёрдое тело (Body)
// может состоять из нескольких оболочек (например, внешняя оболочка
// и оболочка внутренней полости).
//
// Основные свойства:
//   • faces     — индексы граней, входящих в оболочку.
//   • isClosed() — проверяет, замкнута ли оболочка (каждое ребро принадлежит
//                  ровно двум граням из этой оболочки).
//   • isManifold() — проверяет, является ли оболочка многообразием (каждое
//                    ребро принадлежит ровно двум граням).
//
// Shell используется в:
//   • Body — как составная часть твёрдого тела (один Body может содержать
//            несколько Shell).
//   • Проверке замкнутости геометрии перед объёмными операциями.
//   • Экспорте в форматы (STEP, IGES) — оболочки соответствуют понятию SHELL.
//
// Чистый C++23, без внешних зависимостей.


#pragma once

#include <vector>

#include "../Core/Identity/TypedID.hpp"

namespace mir
{

class Shell
{
public:

    Shell() = default;

    explicit Shell(ShellID id)
        : m_id(id)
    {
    }

    ShellID id() const noexcept
    {
        return m_id;
    }

    const std::vector<FaceID>& faces() const noexcept
    {
        return m_faces;
    }

    void addFace(FaceID id)
    {
        m_faces.push_back(id);
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

    ShellID m_id;

    std::vector<FaceID> m_faces;

    bool m_closed = false;
};

}