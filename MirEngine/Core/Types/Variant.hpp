// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Core/Types/Variant.hpp
// ─────────────────────────────────────────────────────────────
// 🎒 ВОЛШЕБНЫЙ РЮКЗАК Variant<Ts...> — хранит ОДНУ вещь из списка
//
// В программировании часто нужно хранить значение, которое может
// быть разных типов. Например, результат измерения может быть
// целым числом, дробным или даже строкой с ошибкой.
//
// Обычная переменная может хранить только один тип (int, double…).
// А Variant — как рюкзак с кармашками для разных игрушек:
//   🧸 Мячик   (int)
//   🚗 Машинка (double)
//   📚 Книжка  (std::string)
//
// Но в рюкзак можно положить только ОДНУ игрушку за раз!
// И мы всегда знаем, какая именно игрушка лежит внутри.
//
// Как это работает:
//   Variant<int, double, std::string> v;
//   v = 42;                  // кладём число 42 (тип int)
//   v = 3.14;                // теперь там 3.14 (тип double), число забыто
//   v = "Привет"s;           // теперь там строка "Привет"
//
//   if (v.is<int>())         // проверяем: внутри лежит int?
//       int x = v.get<int>(); // достаём значение (если тип совпадает)
//
//   // Самый безопасный способ — "посетитель" (visitor)
//   v.visit([](auto&& val) {
//       std::cout << "В рюкзаке: " << val << std::endl;
//   });
//
// 🔒 Безопасность:
//   • get<T>() проверяет тип; если не совпадает — кидает исключение.
//   • visit() автоматически вызывает правильную функцию для любого типа.
//   • Память используется экономно — ровно столько, сколько нужно
//     для самого большого типа из списка.
//
// Чистый C++23, не зависит от внешних библиотек.
// ─────────────────────────────────────────────────────────────

#pragma once

#include <new>              // размещающий new
#include <type_traits>      // для проверки типов (std::is_same_v и др.)
#include <utility>          // std::forward, std::move
#include <stdexcept>        // std::runtime_error
#include <cstddef>          // size_t

namespace mir {

// ╔══════════════════════════════════════════════════════════╗
// ║  Вспомогательные инструменты (внутренние)               ║
// ╚══════════════════════════════════════════════════════════╝

namespace detail {
    // 🔎 Поиск типа T в списке типов Ts...
    // Возвращает индекс (позицию) типа T, или большое число если не найден.
    template<typename T, typename... Ts>
    struct IndexOf;

    // Рекурсивный случай: проверяем первый тип в списке
    template<typename T, typename Head, typename... Tail>
    struct IndexOf<T, Head, Tail...> {
        static constexpr size_t value = std::is_same_v<T, Head>
            ? 0
            : 1 + IndexOf<T, Tail...>::value;
    };

    // Конец рекурсии: список пуст, тип не найден
    template<typename T>
    struct IndexOf<T> {
        static constexpr size_t value = 0; // заглушка, не используется
    };

    // 📏 Размер самого большого типа из списка
    template<typename... Ts>
    struct MaxSize;

    template<typename Head, typename... Tail>
    struct MaxSize<Head, Tail...> {
        static constexpr size_t value =
            sizeof(Head) > MaxSize<Tail...>::value
                ? sizeof(Head)
                : MaxSize<Tail...>::value;
    };

    template<>
    struct MaxSize<> {
        static constexpr size_t value = 1; // пустой список — 1 байт
    };

    // 📏 Самое строгое выравнивание среди типов
    template<typename... Ts>
    struct MaxAlign;

    template<typename Head, typename... Tail>
    struct MaxAlign<Head, Tail...> {
        static constexpr size_t value =
            alignof(Head) > MaxAlign<Tail...>::value
                ? alignof(Head)
                : MaxAlign<Tail...>::value;
    };

    template<>
    struct MaxAlign<> {
        static constexpr size_t value = 1;
    };
} // namespace detail

// ╔══════════════════════════════════════════════════════════╗
// ║  Основной класс Variant<Ts...>                          ║
// ╚══════════════════════════════════════════════════════════╝

template<typename... Ts>
class Variant {
public:
    // ── Конструкторы ────────────────────────────────────────

    // 1. Пустой рюкзак (по умолчанию). Внутри — первый тип из списка.
    Variant() noexcept(std::is_nothrow_default_constructible_v<
                       typename std::tuple_element<0, std::tuple<Ts...>>::type>)
        : m_activeIndex(0) {
        using FirstType = typename std::tuple_element<0, std::tuple<Ts...>>::type;
        new (m_data) FirstType{};
    }

    // 2. Рюкзак с конкретным значением (копирование)
    template<typename T, typename = std::enable_if_t<
                 (std::is_same_v<T, Ts> || ...)> >
    Variant(const T& value)
        : m_activeIndex(detail::IndexOf<T, Ts...>::value) {
        new (m_data) T(value);
    }

    // 3. Рюкзак с конкретным значением (перемещение)
    template<typename T, typename = std::enable_if_t<
                 (std::is_same_v<T, Ts> || ...)> >
    Variant(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_activeIndex(detail::IndexOf<T, Ts...>::value) {
        new (m_data) T(std::move(value));
    }

    // 4. Копирование другого рюкзака
    Variant(const Variant& other)
        : m_activeIndex(other.m_activeIndex) {
        other.visit([this](const auto& val) {
            using ValueType = std::decay_t<decltype(val)>;
            new (m_data) ValueType(val);
        });
    }

    // 5. Перемещение другого рюкзака
    Variant(Variant&& other) noexcept
        : m_activeIndex(other.m_activeIndex) {
        other.visit([this](auto&& val) {
            using ValueType = std::decay_t<decltype(val)>;
            new (m_data) ValueType(std::move(val));
        });
        // Опустошаем другой рюкзак (без вызова деструктора, так как память перемещена)
        other.m_activeIndex = 0;
        new (other.m_data) typename std::tuple_element<0, std::tuple<Ts...>>::type{};
    }

    // ── Деструктор ──────────────────────────────────────────
    // Аккуратно уничтожаем то, что лежит внутри.
    ~Variant() {
        destroyCurrent();
    }

    // ── Присваивание ────────────────────────────────────────

    Variant& operator=(const Variant& other) {
        if (this != &other) {
            destroyCurrent();
            other.visit([this](const auto& val) {
                using ValueType = std::decay_t<decltype(val)>;
                new (m_data) ValueType(val);
            });
            m_activeIndex = other.m_activeIndex;
        }
        return *this;
    }

    Variant& operator=(Variant&& other) noexcept {
        if (this != &other) {
            destroyCurrent();
            other.visit([this](auto&& val) {
                using ValueType = std::decay_t<decltype(val)>;
                new (m_data) ValueType(std::move(val));
            });
            m_activeIndex = other.m_activeIndex;
        }
        return *this;
    }

    // ── Положить новое значение ─────────────────────────────
    // Заменяет содержимое рюкзака на новое значение типа T.
    template<typename T, typename... Args>
    void emplace(Args&&... args) {
        static_assert((std::is_same_v<T, Ts> || ...),
                      "Variant::emplace — тип T не входит в список допустимых типов");
        destroyCurrent();
        new (m_data) T(std::forward<Args>(args)...);
        m_activeIndex = detail::IndexOf<T, Ts...>::value;
    }

    // ── Узнать, какой тип внутри (индекс) ───────────────────
    [[nodiscard]] size_t index() const noexcept { return m_activeIndex; }

    // ── Проверка: лежит ли сейчас тип T? ────────────────────
    template<typename T>
    [[nodiscard]] bool is() const noexcept {
        return m_activeIndex == detail::IndexOf<T, Ts...>::value;
    }

    // ── Достать значение по типу (с проверкой!) ─────────────
    template<typename T>
    T& get() {
        if (!is<T>()) {
            throw std::runtime_error(
                "Variant::get<T>() — неверный тип! "
                "Проверьте тип через is<T>() перед вызовом.");
        }
        return *reinterpret_cast<T*>(m_data);
    }

    template<typename T>
    const T& get() const {
        if (!is<T>()) {
            throw std::runtime_error(
                "Variant::get<T>() — неверный тип! "
                "Проверьте тип через is<T>() перед вызовом.");
        }
        return *reinterpret_cast<const T*>(m_data);
    }

    // ── Посетитель (visitor) ─────────────────────────────────
    // Самый безопасный способ работы: передаём функцию,
    // которая умеет обрабатывать ЛЮБОЙ возможный тип.
    // Variant сам вызовет правильную версию.
    template<typename Visitor>
    decltype(auto) visit(Visitor&& visitor) {
        return visitImpl(std::forward<Visitor>(visitor),
                         std::index_sequence_for<Ts...>{});
    }

    template<typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const {
        return visitImpl(std::forward<Visitor>(visitor),
                         std::index_sequence_for<Ts...>{});
    }

    // ── Сравнение рюкзаков ──────────────────────────────────
    bool operator==(const Variant& other) const {
        if (m_activeIndex != other.m_activeIndex) return false;
        return other.visit([this](const auto& val) -> bool {
            using T = std::decay_t<decltype(val)>;
            return get<T>() == val;
        });
    }

    bool operator!=(const Variant& other) const {
        return !(*this == other);
    }

private:
    // 🧱 Память, вмещающая самый большой из возможных типов
    alignas(detail::MaxAlign<Ts...>::value)
    unsigned char m_data[detail::MaxSize<Ts...>::value];

    // 🏷️ Индекс активного типа (0 = первый тип, 1 = второй…)
    size_t m_activeIndex;

    // 🧹 Уничтожить текущее значение (вызвать деструктор)
    void destroyCurrent() noexcept {
        visit([](auto& val) {
            using T = std::decay_t<decltype(val)>;
            val.~T(); // явный вызов деструктора
        });
    }

    // 🔧 Реализация visit с распаковкой индексов
    template<typename Visitor, size_t... Indices>
    decltype(auto) visitImpl(Visitor&& visitor, std::index_sequence<Indices...>) {
        using ReturnType = std::common_type_t<
            decltype(std::forward<Visitor>(visitor)(
                std::declval<Ts&>()))...>;
        ReturnType result;
        ((Indices == m_activeIndex
              ? (result = std::forward<Visitor>(visitor)(
                     *reinterpret_cast<Ts*>(m_data)), true)
              : false) || ...);
        return result;
    }

    template<typename Visitor, size_t... Indices>
    decltype(auto) visitImpl(Visitor&& visitor, std::index_sequence<Indices...>) const {
        using ReturnType = std::common_type_t<
            decltype(std::forward<Visitor>(visitor)(
                std::declval<const Ts&>()))...>;
        ReturnType result;
        ((Indices == m_activeIndex
              ? (result = std::forward<Visitor>(visitor)(
                     *reinterpret_cast<const Ts*>(m_data)), true)
              : false) || ...);
        return result;
    }
};

// ─────────────────────────────────────────────────────────────
// 🎁 Удобная функция: создать Variant со значением
// ─────────────────────────────────────────────────────────────
template<typename T, typename... Ts>
Variant<Ts...> make_variant(T&& value) {
    return Variant<Ts...>(std::forward<T>(value));
}

} // namespace mir