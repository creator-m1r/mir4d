// MirEngine/Core/Types/Index.hpp
// 🔢 Типобезопасный индекс — число, которое нельзя спутать с другими числами.
//
// В программировании часто используют обычные int или size_t для индексов:
//   int index = 5;
//   int count = 10;
//   int id = 3;
// Но компилятор не видит разницы между индексом, счётчиком и идентификатором.
// Можно случайно передать count туда, где ожидается index, и получить ошибку,
// которую очень трудно найти.
//
// Класс Index решает эту проблему: он создаёт НОВЫЙ ТИП, который нельзя
// неявно преобразовать в обычное число и обратно. Чтобы получить число,
// нужно явно вызвать метод value(). Это как защитный чехол для числа —
// ты всегда знаешь, что работаешь именно с индексом, а не с чем-то другим.
//
// Пример использования:
//   Index idx{5};              // создать индекс со значением 5
//   size_t raw = idx.value();  // явно получить число (ОК)
//   // size_t bad = idx;       // ОШИБКА! Неявное преобразование запрещено
//   if (idx == Index{3}) ...   // сравнение индексов (ОК)
//   // if (idx == 5) ...       // ОШИБКА! Нельзя сравнить с голым числом
//
// Это маленькая деталь, которая предотвращает тысячи ошибок в большом проекте.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstddef>      // для size_t
#include <functional>   // для std::hash

namespace mir {

class Index {
public:
    // Создаёт индекс со значением 0 (по умолчанию).
    constexpr Index() noexcept = default;

    // Создаёт индекс с заданным значением.
    // Ключевое слово explicit запрещает неявное преобразование:
    //   Index i = 5;  // ОШИБКА!
    //   Index i{5};   // ОК
    explicit constexpr Index(std::size_t value) noexcept
        : m_value(value)
    {}

    // Получить числовое значение индекса (единственный способ).
    [[nodiscard]] constexpr std::size_t value() const noexcept {
        return m_value;
    }

    // ── Операторы сравнения ──────────────────────────────────
    // Индексы можно сравнивать только с другими индексами.
    friend constexpr bool operator==(Index lhs, Index rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(Index lhs, Index rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(Index lhs, Index rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(Index lhs, Index rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(Index lhs, Index rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(Index lhs, Index rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    std::size_t m_value = 0;   // хранимое число
};

} // namespace mir

// Специализация std::hash, чтобы Index можно было использовать
// в качестве ключа в unordered_map и unordered_set.
namespace std {
template <>
struct hash<mir::Index> {
    std::size_t operator()(const mir::Index& idx) const noexcept {
        return hash<std::size_t>{}(idx.value());
    }
};
} // namespace std