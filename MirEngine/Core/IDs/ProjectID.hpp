// MirEngine/Core/IDs/ProjectID.hpp
// 📁 Типобезопасный идентификатор проекта (Project) — уникальный номер
//    проекта верхнего уровня, который нельзя спутать с другими ID.
//
// Проект — это самый верхний уровень организации данных в САПР.
// Он содержит документы, настройки, пути к файлам, историю изменений.
// Каждый проект получает свой уникальный идентификатор ProjectID.
//
// ProjectID — это ОТДЕЛЬНЫЙ ТИП, независимый от DocumentID, EntityID и т.д.
// Благодаря строгой типизации компилятор не позволит случайно перепутать
// проект с документом или сущностью сцены.
//
// Генерацией уникальных ProjectID занимается IDGenerator (createProject).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>
#include <functional>

namespace mir {

class ProjectID {
public:
    // Создаёт невалидный (нулевой) идентификатор проекта.
    // 0 означает "проект не задан" или "отсутствует".
    constexpr ProjectID() noexcept = default;

    // Создаёт идентификатор проекта с заданным числовым значением.
    explicit constexpr ProjectID(uint64_t value) noexcept
        : m_value(value) {}

    // Получить числовое значение (единственный способ).
    [[nodiscard]] constexpr uint64_t value() const noexcept {
        return m_value;
    }

    // Является ли идентификатор валидным (не нулевым)?
    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != 0;
    }

    // ── Операторы сравнения ──────────────────────────────────
    friend constexpr bool operator==(ProjectID lhs, ProjectID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(ProjectID lhs, ProjectID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(ProjectID lhs, ProjectID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(ProjectID lhs, ProjectID rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(ProjectID lhs, ProjectID rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(ProjectID lhs, ProjectID rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    uint64_t m_value = 0;   // номер проекта (0 = невалидный)
};

} // namespace mir

// Специализация std::hash для использования ProjectID в unordered-контейнерах.
namespace std {
template <>
struct hash<mir::ProjectID> {
    std::size_t operator()(const mir::ProjectID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std