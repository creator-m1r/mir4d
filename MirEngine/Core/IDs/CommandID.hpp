// MirEngine/Core/IDs/CommandID.hpp
// 🎮 Типобезопасный идентификатор команды — уникальный номер для каждой операции.
//
// В инженерном движке все действия пользователя оформляются как команды:
// «создать куб», «выдавить эскиз на 100 мм», «повернуть деталь на 45°».
// Каждая команда получает свой уникальный идентификатор — CommandID.
// По этому номеру команду можно найти в истории, отменить (Undo) или
// повторить (Redo), сохранить в файл проекта или передать по сети.
//
// Как и другие ID в MirEngine, CommandID — это отдельный тип.
// Его нельзя спутать с EntityID (номер детали), ObjectID (номер объекта
// документа) или любым другим идентификатором. Компилятор не даст
// передать CommandID туда, где ожидается EntityID, и наоборот.
//
// Внутри CommandID — просто 64-битное число, но благодаря классу-обёртке
// оно становится неуязвимым для случайных ошибок.
//
// Генерацией уникальных CommandID занимается IDGenerator (createCommand).
//
// Использование:
//   CommandID cmd{1};                    // команда с номером 1
//   uint64_t raw = cmd.value();          // явно получить число
//   if (cmd == CommandID{2}) ...         // сравнение команд
//   // EntityID e = cmd;                 // ОШИБКА! Нельзя присвоить команду в EntityID
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>      // для uint64_t
#include <functional>   // для std::hash

namespace mir {

class CommandID {
public:
    // Создаёт невалидный (нулевой) идентификатор команды.
    // 0 обычно означает "команда отсутствует" или "не задана".
    constexpr CommandID() noexcept = default;

    // Создаёт идентификатор команды с заданным числовым значением.
    // explicit запрещает неявное преобразование из uint64_t.
    explicit constexpr CommandID(uint64_t value) noexcept
        : m_value(value)
    {}

    // Получить числовое значение (единственный способ).
    [[nodiscard]] constexpr uint64_t value() const noexcept {
        return m_value;
    }

    // Является ли команда валидной (не нулевой)?
    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != 0;
    }

    // ── Операторы сравнения ──────────────────────────────────
    friend constexpr bool operator==(CommandID lhs, CommandID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(CommandID lhs, CommandID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(CommandID lhs, CommandID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(CommandID lhs, CommandID rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(CommandID lhs, CommandID rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(CommandID lhs, CommandID rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    uint64_t m_value = 0;   // номер команды (0 = невалидный)
};

} // namespace mir

// Специализация std::hash для использования CommandID в unordered-контейнерах.
namespace std {
template <>
struct hash<mir::CommandID> {
    std::size_t operator()(const mir::CommandID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std