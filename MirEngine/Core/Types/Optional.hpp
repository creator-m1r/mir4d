// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Core/Types/Optional.hpp
// ─────────────────────────────────────────────────────────────
// 📦 ВОЛШЕБНАЯ КОРОБОЧКА Optional<T> — значение, которого может не быть
//
// В программировании часто бывает так: мы ищем деталь по имени,
// но её нет в списке. Или спрашиваем у пользователя возраст,
// а он ещё не ввёл. Как сообщить программе: «Эй, тут пусто»?
//
// Обычный int всегда хранит число (например, 0), и непонятно:
// это настоящий ноль или «ничего»? Optional<T> решает эту проблему!
//
// Optional похож на коробочку:
//   📦[пусто]              — значения нет (состояние "nullopt")
//   📦[42]                 — внутри лежит число 42
//   📦["Привет"]           — внутри лежит строка "Привет"
//
// Прежде чем достать значение, нужно ОБЯЗАТЕЛЬНО проверить,
// есть ли оно там. Коробочка сама подскажет методом has_value().
//
// Использование:
//   Optional<int> age;
//   age = 25;                        // кладём 25 в коробочку
//   if (age.has_value()) {
//       cout << "Возраст: " << age.value();
//   }
//   age.reset();                     // очищаем коробочку
//
//   Optional<std::string> name = "Мир";
//   std::string n = name.value_or("Безымянный"); // если пусто, подставить замену
//
// Чистый C++23, не зависит от стандартной библиотеки (кроме базовых типов)
// ─────────────────────────────────────────────────────────────

#pragma once

#include <new>              // для размещающего new (создание объекта в готовой памяти)
#include <type_traits>      // для проверки свойств типов
#include <utility>          // для std::move (перемещение вместо копирования)
#include <stdexcept>        // для исключений (std::runtime_error)

namespace mir {

// 🚩 Специальный тип-маркер: «пусто»
// Когда мы видим этот тип, мы понимаем: в коробочке ничего нет.
struct nullopt_t {
    // Явный конструктор запрещает создание объекта,
    // кроме как через глобальную константу nullopt.
    explicit nullopt_t() = default;
};

// 🌍 Глобальная константа — символ пустоты.
// Её можно использовать так: Optional<int> x = nullopt;
inline constexpr nullopt_t nullopt{};

// ╔══════════════════════════════════════════════════════════╗
// ║  Шаблонный класс Optional<T>                            ║
// ╚══════════════════════════════════════════════════════════╝
template<typename T>
class Optional {
public:
    // ── Конструкторы (способы создания коробочки) ───────────

    // 1. Пустая коробочка (по умолчанию)
    Optional() noexcept : m_hasValue(false) {}

    // 2. Пустая коробочка (явно указываем nullopt)
    Optional(nullopt_t) noexcept : m_hasValue(false) {}

    // 3. Коробочка со значением (копирование)
    //    Кладём в коробочку копию переданного значения.
    Optional(const T& value) : m_hasValue(true) {
        new (m_data) T(value);   // "размещающий new" — создаём объект T прямо в нашей памяти
    }

    // 4. Коробочка со значением (перемещение)
    //    Забираем значение у другого объекта, не копируя.
    Optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_hasValue(true) {
        new (m_data) T(std::move(value));
    }

    // 5. Копирование другой коробочки
    Optional(const Optional& other) : m_hasValue(other.m_hasValue) {
        if (m_hasValue) {
            new (m_data) T(other.value());
        }
    }

    // 6. Перемещение другой коробочки
    Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_hasValue(other.m_hasValue) {
        if (m_hasValue) {
            new (m_data) T(std::move(other.value()));
            other.m_hasValue = false;   // другая коробочка теперь пуста
        }
    }

    // ── Деструктор (уборка за собой) ────────────────────────
    // Если в коробочке что-то лежит, нужно аккуратно это удалить.
    ~Optional() {
        reset();
    }

    // ── Присваивание (положить новое значение) ──────────────

    // Присвоить пустоту
    Optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    // Присвоить значение (копированием)
    Optional& operator=(const T& value) {
        reset();                       // сначала убираем старое значение
        new (m_data) T(value);         // кладём новое
        m_hasValue = true;
        return *this;
    }

    // Присвоить значение (перемещением)
    Optional& operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>) {
        reset();
        new (m_data) T(std::move(value));
        m_hasValue = true;
        return *this;
    }

    // Скопировать другую коробочку
    Optional& operator=(const Optional& other) {
        if (this != &other) {
            reset();
            if (other.m_hasValue) {
                new (m_data) T(other.value());
                m_hasValue = true;
            }
        }
        return *this;
    }

    // Переместить другую коробочку
    Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (this != &other) {
            reset();
            if (other.m_hasValue) {
                new (m_data) T(std::move(other.value()));
                m_hasValue = true;
                other.m_hasValue = false;
            }
        }
        return *this;
    }

    // ── Проверка: есть ли значение? ─────────────────────────
    // Самый главный вопрос коробочке: «Ты пустая или нет?»
    [[nodiscard]] bool has_value() const noexcept { return m_hasValue; }

    // Удобный оператор: если в if (opt) — это то же, что if (opt.has_value())
    explicit operator bool() const noexcept { return m_hasValue; }

    // ── Достать значение ────────────────────────────────────
    // ВНИМАНИЕ! Если коробочка пуста, программа аварийно завершится.
    // Всегда проверяй через has_value() перед вызовом!

    T& value() {
        if (!m_hasValue) {
            throw std::runtime_error("Optional::value() — коробочка пуста!");
        }
        return *reinterpret_cast<T*>(m_data);
    }

    const T& value() const {
        if (!m_hasValue) {
            throw std::runtime_error("Optional::value() — коробочка пуста!");
        }
        return *reinterpret_cast<const T*>(m_data);
    }

    // ── Достать значение или подставить замену ─────────────
    // Самый безопасный способ: если пусто — вернуть default_value.
    T value_or(const T& default_value) const {
        return m_hasValue ? value() : default_value;
    }

    // ── Очистить коробочку ──────────────────────────────────
    void reset() noexcept {
        if (m_hasValue) {
            reinterpret_cast<T*>(m_data)->~T();   // вызываем деструктор значения
            m_hasValue = false;
        }
    }

    // ── Сравнение коробочек ─────────────────────────────────
    bool operator==(const Optional& other) const {
        if (!m_hasValue && !other.m_hasValue) return true;   // обе пусты — равны
        if ( m_hasValue &&  other.m_hasValue) return value() == other.value();
        return false;   // одна пуста, другая нет — не равны
    }

    bool operator!=(const Optional& other) const {
        return !(*this == other);
    }

private:
    // 🧱 Память для хранения значения.
    // Используем alignas(T), чтобы память была выровнена правильно
    // для любого типа T (даже если T — большая структура).
    alignas(T) unsigned char m_data[sizeof(T)];

    // 🚩 Флаг: есть ли сейчас значение в коробочке?
    bool m_hasValue;
};

// ─────────────────────────────────────────────────────────────
// 🎁 Удобная функция: создать Optional со значением
// ─────────────────────────────────────────────────────────────
template<typename T>
Optional<T> make_optional(T&& value) {
    return Optional<T>(std::forward<T>(value));
}

} // namespace mir