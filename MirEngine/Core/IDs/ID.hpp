// MirEngine/Core/IDs/ID.hpp
// 🏷️ Базовый типобезопасный идентификатор — шаблон, который создаёт
//    уникальные типы для разных сущностей (Entity, Object, Feature...).
//
// В больших проектах часто заводят просто числа (uint64_t) или строки
// для идентификации объектов. Но тогда легко перепутать идентификаторы
// разных типов: передать ID объекта туда, где ждут ID документа.
// Компилятор не видит разницы, ошибка проявляется только во время работы.
//
// Шаблонный класс ID<Tag> решает эту проблему элегантно:
//   • Каждый тип сущности получает свой уникальный тип ID.
//   • ID<EntityTag> и ID<ObjectTag> — это РАЗНЫЕ типы, их нельзя
//     случайно сравнить или присвоить друг другу.
//   • Внутри всё равно хранится лёгкое число (uint64_t), так что
//     это не замедляет программу.
//
// Как это работает:
//   1. Мы создаём пустую структуру-тэг, например struct EntityTag {};
//   2. Объявляем тип: using EntityID = ID<EntityTag>;
//   3. Теперь EntityID и ObjectID — разные типы, и компилятор не даст
//      их перепутать.
//
// Это как бирки на ключах: ключ от кабинета и ключ от машины выглядят
// похоже, но бирка говорит, какой ключ для чего. Здесь бирка — это тэг.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>      // для uint64_t
#include <functional>   // для std::hash
#include <compare>      // для operator<=> (C++20)

namespace mir {

// ── Шаблонный типобезопасный идентификатор ──────────────────
// Tag — пустая структура-тэг, которая делает каждый ID уникальным типом.
// ValueType — тип хранимого числа (по умолчанию uint64_t — 64-битное целое).
template <typename Tag, typename ValueType = uint64_t>
class ID {
public:
    // Создаёт невалидный (нулевой) идентификатор.
    constexpr ID() noexcept = default;

    // Создаёт идентификатор с заданным числовым значением.
    // explicit — чтобы нельзя было неявно преобразовать число в ID.
    explicit constexpr ID(ValueType value) noexcept
        : m_value(value)
    {}

    // Получить числовое значение (единственный способ).
    [[nodiscard]] constexpr ValueType value() const noexcept {
        return m_value;
    }

    // Является ли идентификатор валидным (не нулевым)?
    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != ValueType{0};
    }

    // ── Операторы сравнения ──────────────────────────────────
    // Сравнивать можно только ID с одинаковыми тэгами.
    friend constexpr bool operator==(ID lhs, ID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(ID lhs, ID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(ID lhs, ID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator>(ID lhs, ID rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator<=(ID lhs, ID rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>=(ID lhs, ID rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

    // Трёхстороннее сравнение (C++20) — генерирует все 6 операторов.
    friend constexpr auto operator<=>(ID lhs, ID rhs) noexcept {
        return lhs.m_value <=> rhs.m_value;
    }

private:
    ValueType m_value{0};   // хранимое число (0 = невалидный)
};

} // namespace mir

// Специализация std::hash — чтобы ID можно было использовать
// в unordered_map и unordered_set.
template <typename Tag, typename ValueType>
struct std::hash<mir::ID<Tag, ValueType>> {
    std::size_t operator()(const mir::ID<Tag, ValueType>& id) const noexcept {
        return std::hash<ValueType>{}(id.value());
    }
};