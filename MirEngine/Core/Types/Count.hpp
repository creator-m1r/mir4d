// MirEngine/Core/Types/Count.hpp
// 🔢 Типобезопасный счётчик количества — число, которое нельзя спутать с индексом.
//
// Представь, что у тебя есть коробка с деталями. Ты можешь сказать:
//   • «Дай мне пятую деталь» — это ИНДЕКС (порядковый номер).
//   • «В коробке 10 деталей» — это КОЛИЧЕСТВО (сколько всего).
// Это разные понятия, хотя оба выражаются числами.
//
// В коде их тоже важно различать:
//   Index i{5};   // индекс (какой по счёту)
//   Count c{10};  // количество (сколько штук)
//   i = c;        // ОШИБКА! Нельзя присвоить количество индексу
//   if (i == c)   // ОШИБКА! Нельзя сравнить индекс с количеством
//
// Класс Count создаёт отдельный тип для количества, так же как Index —
// для индекса. Это как подписанные коробочки: в одной лежат индексы,
// в другой — количества, и перепутать их невозможно.
//
// Примеры использования:
//   Count vertexCount{mesh.vertexCount()};  // количество вершин в меше
//   Count faceCount{100};                   // явно заданное количество
//   size_t raw = faceCount.value();         // получить число (ОК)
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstddef>      // для size_t
#include <functional>   // для std::hash

namespace mir {

class Count {
public:
    // Создаёт счётчик со значением 0 (пустое количество).
    constexpr Count() noexcept = default;

    // Создаёт счётчик с заданным значением.
    // explicit запрещает неявные преобразования от обычных чисел.
    explicit constexpr Count(std::size_t value) noexcept
        : m_value(value)
    {}

    // Получить числовое значение (единственный способ).
    [[nodiscard]] constexpr std::size_t value() const noexcept {
        return m_value;
    }

    // ── Удобные проверки ─────────────────────────────────────
    // Пусто ли количество? (равно ли нулю)
    [[nodiscard]] constexpr bool empty() const noexcept {
        return m_value == 0;
    }

    // ── Операторы сравнения ──────────────────────────────────
    // Сравнивать можно только с другими Count, не с Index и не с числами.
    friend constexpr bool operator==(Count lhs, Count rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(Count lhs, Count rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(Count lhs, Count rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(Count lhs, Count rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(Count lhs, Count rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(Count lhs, Count rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    std::size_t m_value = 0;   // хранимое количество
};

} // namespace mir

// Специализация std::hash для использования Count в unordered-контейнерах.
namespace std {
template <>
struct hash<mir::Count> {
    std::size_t operator()(const mir::Count& count) const noexcept {
        return hash<std::size_t>{}(count.value());
    }
};
} // namespace std