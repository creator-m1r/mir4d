// MirEngine/Core/IDs/EntityID.hpp
// 🏷️ Типобезопасный идентификатор сущности (Entity) — уникальный номер,
//    который нельзя спутать с другими идентификаторами.
//
// В игровых и CAD-движках есть множество объектов: детали, компоненты,
// камеры, источники света, материалы... У каждого должен быть свой
// уникальный идентификатор, чтобы можно было сказать: «Поверни деталь №42»,
// «Удали камеру №7», «Выдели компонент №3».
//
// Но если использовать просто числа (uint64_t), легко перепутать:
//   uint64_t entityId = 5;
//   uint64_t objectId = 5;
//   if (entityId == objectId) // true, но это РАЗНЫЕ сущности!
//
// EntityID решает эту проблему: это ОТДЕЛЬНЫЙ ТИП. Его нельзя неявно
// преобразовать в другой ID-тип (ObjectID, ComponentID…) или в число.
// Компилятор не даст перепутать их местами.
//
// Внутри EntityID — просто 64-битное число, но "завёрнутое" в класс.
// Это даёт типобезопасность без накладных расходов: sizeof(EntityID) == 8.
//
// Генерацией уникальных ID занимается IDGenerator (будет создан позже).
//
// Использование:
//   EntityID id{42};                  // создать с номером 42
//   uint64_t raw = id.value();        // явно получить число (ОК)
//   // uint64_t bad = id;             // ОШИБКА! Неявное преобразование запрещено
//   if (id == EntityID{42}) ...       // сравнение (ОК)
//   // if (id == 42) ...              // ОШИБКА! Нельзя сравнить с голым числом
//   // ObjectID oid = id;             // ОШИБКА! Разные типы
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>      // для uint64_t
#include <functional>   // для std::hash

namespace mir {

class EntityID {
public:
    // Создаёт невалидный (нулевой) идентификатор.
    // 0 обычно означает "отсутствует" или "не задан".
    constexpr EntityID() noexcept = default;

    // Создаёт идентификатор с заданным числовым значением.
    // explicit запрещает неявное преобразование из uint64_t.
    explicit constexpr EntityID(uint64_t value) noexcept
        : m_value(value)
    {}

    // Получить числовое значение (единственный способ).
    [[nodiscard]] constexpr uint64_t value() const noexcept {
        return m_value;
    }

    // Является ли идентификатор валидным (не нулевым)?
    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != 0;
    }

    // ── Операторы сравнения ──────────────────────────────────
    // Сравнивать можно только с другими EntityID.
    friend constexpr bool operator==(EntityID lhs, EntityID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(EntityID lhs, EntityID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(EntityID lhs, EntityID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(EntityID lhs, EntityID rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(EntityID lhs, EntityID rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(EntityID lhs, EntityID rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    uint64_t m_value = 0;   // хранимое число (0 = невалидный)
};

} // namespace mir

// Специализация std::hash для использования EntityID в unordered-контейнерах.
namespace std {
template <>
struct hash<mir::EntityID> {
    std::size_t operator()(const mir::EntityID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std