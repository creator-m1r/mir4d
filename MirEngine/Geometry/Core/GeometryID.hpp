// ─────────────────────────────────────────────────────────────
//  MirEngine/Geometry/Core/GeometryID.hpp
// ─────────────────────────────────────────────────────────────
//  ИДЕНТИФИКАТОР ГЕОМЕТРИЧЕСКОГО ОБЪЕКТА (GeometryID)
//
// В системе МИР 4D каждая точка, линия, тело, поверхность
// должна иметь свой уникальный номер — как номер паспорта
// у человека. По этому номеру мы можем быстро найти объект
// в большой картотеке GeometryContext.
//
// GeometryID — это просто целое 64-битное число (uint64_t),
// обёрнутое в красивую структуру с удобными операторами:
//   • Сравнивать (==, !=) — чтобы понять, один и тот же объект или нет.
//   • Печатать (toString) — для отладки и записи в лог.
//   • Хранить в контейнерах (unordered_map) — благодаря хеш-функции.
//
// Почему не просто int?
//   1. Безопасность типов: если перепутать int ID детали и int ID документа,
//      компилятор не заметит ошибки. А с GeometryID — заметит!
//   2. Читаемость кода: по сигнатуре функции сразу видно, что она принимает
//      идентификатор геометрического объекта, а не «какое-то число».
//   3. Будущая расширяемость: сегодня это число, завтра можно добавить
//      проверки на валидность или генерацию из UUID.
//
// Использование:
//   GeometryID id1{42};
//   GeometryID id2{100};
//   if (id1 == id2) { ... }          // сравнение
//   std::cout << id1.toString();     // "GeometryID(42)"
//
// Чистый C++23, без внешних зависимостей.
// ─────────────────────────────────────────────────────────────

#pragma once

#include <cstdint>        // uint64_t
#include <string>         // std::string, std::to_string
#include <functional>     // std::hash

namespace M1R {

class GeometryID {
public:
    // ── Значение идентификатора (можно читать и менять напрямую) ──
    uint64_t value;

    // ── Конструкторы ──────────────────────────────────────────
    // Создать ID из числа (например, GeometryID(42))
    constexpr GeometryID() noexcept : value(0) {}
    constexpr GeometryID(uint64_t id) noexcept : value(id) {}

    // ── Сравнение (одинаковые ли ID?) ─────────────────────────
    constexpr bool operator==(const GeometryID& other) const noexcept {
        return value == other.value;
    }
    constexpr bool operator!=(const GeometryID& other) const noexcept {
        return value != other.value;
    }

    // Чтобы можно было использовать GeometryID как ключ в std::map
    constexpr bool operator<(const GeometryID& other) const noexcept {
        return value < other.value;
    }

    // ── Печать в строку (для логов и отладки) ─────────────────
    [[nodiscard]] std::string toString() const {
        return "GeometryID(" + std::to_string(value) + ")";
    }

    // ── Удобный оператор вывода в поток (cout, cerr, файл) ────
    friend std::ostream& operator<<(std::ostream& os, const GeometryID& id) {
        os << id.toString();
        return os;
    }
};

} // namespace M1R

// ─────────────────────────────────────────────────────────────
//  Специализация std::hash для использования GeometryID
//    в качестве ключа в unordered_map / unordered_set
// ─────────────────────────────────────────────────────────────
namespace std {
    template<>
    struct hash<M1R::GeometryID> {
        size_t operator()(const M1R::GeometryID& id) const noexcept {
            return hash<uint64_t>{}(id.value);
        }
    };
} // namespace std