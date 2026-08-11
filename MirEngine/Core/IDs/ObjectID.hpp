// MirEngine/Core/IDs/ObjectID.hpp
// 🧱 Типобезопасный идентификатор объекта документа (DocumentObject) —
//    уникальный номер для каждого элемента внутри документа CAD.
//
// В CAD-документе могут находиться сотни объектов: трёхмерные тела,
// эскизы, кривые, плоскости, материалы, параметры. Каждый объект получает
// свой уникальный идентификатор ObjectID. По этому номеру объект можно
// найти, выделить, изменить его свойства или удалить.
//
// ObjectID — это ОТДЕЛЬНЫЙ ТИП, независимый от EntityID (сущности сцены),
// ComponentID (компоненты сборки), FeatureID (операции моделирования) и т.д.
// Благодаря строгой типизации компилятор не позволит случайно перепутать
// объект документа с деталью на сцене или командой в истории.
//
// Генерацией уникальных ObjectID занимается IDGenerator (createObject).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>
#include <functional>

namespace mir {

class ObjectID {
public:
    // Создаёт невалидный (нулевой) идентификатор объекта.
    // 0 означает "объект не задан" или "отсутствует".
    constexpr ObjectID() noexcept = default;

    // Создаёт идентификатор объекта с заданным числовым значением.
    // explicit запрещает неявное преобразование из uint64_t.
    explicit constexpr ObjectID(uint64_t value) noexcept
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
    friend constexpr bool operator==(ObjectID lhs, ObjectID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(ObjectID lhs, ObjectID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(ObjectID lhs, ObjectID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(ObjectID lhs, ObjectID rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(ObjectID lhs, ObjectID rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(ObjectID lhs, ObjectID rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    uint64_t m_value = 0;   // номер объекта (0 = невалидный)
};

} // namespace mir

// Специализация std::hash для использования ObjectID в unordered-контейнерах.
namespace std {
template <>
struct hash<mir::ObjectID> {
    std::size_t operator()(const mir::ObjectID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std