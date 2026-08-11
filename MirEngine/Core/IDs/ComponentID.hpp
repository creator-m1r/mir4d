// MirEngine/Core/IDs/ComponentID.hpp
// 🧩 Типобезопасный идентификатор компонента (Component) — уникальный номер
//    компонента в сборке (Assembly), который нельзя спутать с другими ID.
//
// В САПР сборка состоит из компонентов — отдельных деталей или под-сборок,
// которые могут быть вставлены в общую модель. Каждый компонент получает
// свой уникальный идентификатор ComponentID, чтобы можно было:
//   • Найти компонент по номеру.
//   • Переместить, повернуть или удалить компонент.
//   • Задать ограничения (constraints) между компонентами.
//   • Сохранить и восстановить структуру сборки.
//
// ComponentID, как и другие ID в MirEngine, — это ОТДЕЛЬНЫЙ ТИП.
// Нельзя неявно преобразовать ComponentID в EntityID, ObjectID или
// любое другое число. Благодаря этому компилятор не даст перепутать
// компонент с деталью, камерой или командой.
//
// Внутри ComponentID — 64-битное число, завёрнутое в класс-обёртку.
// Никаких дополнительных накладных расходов по памяти или времени.
//
// Генерацией уникальных ComponentID занимается IDGenerator (createComponent).
//
// Использование:
//   ComponentID comp{5};                  // компонент с номером 5
//   uint64_t raw = comp.value();          // явно получить число
//   if (comp == ComponentID{3}) ...       // сравнение компонентов
//   // EntityID e = comp;                 // ОШИБКА! Нельзя присвоить компонент в EntityID
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>      // для uint64_t
#include <functional>   // для std::hash

namespace mir {

class ComponentID {
public:
    // Создаёт невалидный (нулевой) идентификатор компонента.
    // 0 обычно означает "компонент отсутствует" или "не задан".
    constexpr ComponentID() noexcept = default;

    // Создаёт идентификатор компонента с заданным числовым значением.
    // explicit запрещает неявное преобразование из uint64_t.
    explicit constexpr ComponentID(uint64_t value) noexcept
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
    // Сравнивать можно только с другими ComponentID.
    friend constexpr bool operator==(ComponentID lhs, ComponentID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(ComponentID lhs, ComponentID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(ComponentID lhs, ComponentID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(ComponentID lhs, ComponentID rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(ComponentID lhs, ComponentID rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(ComponentID lhs, ComponentID rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    uint64_t m_value = 0;   // хранимое число (0 = невалидный)
};

} // namespace mir

// Специализация std::hash для использования ComponentID в unordered-контейнерах.
namespace std {
template <>
struct hash<mir::ComponentID> {
    std::size_t operator()(const mir::ComponentID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std