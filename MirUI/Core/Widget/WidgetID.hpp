// MirUI/Core/Widget/WidgetID.hpp
// 🏷️ Уникальный строковый идентификатор любого элемента интерфейса.
//
// Вместо чисел или случайных хешей MirUI использует осмысленные строки,
// например "mainWindow", "createButton", "projectTree".
// Это делает код читаемым и позволяет легко находить элементы в отладчике.
//
// Чистый C++23, без платформенных зависимостей.

// MirUI/Core/Widget/WidgetID.hpp
#pragma once

#include <cstdint>
#include <functional>

namespace MirUI {

class WidgetID {
public:
    constexpr WidgetID() noexcept = default;
    explicit constexpr WidgetID(uint64_t value) noexcept : m_value(value) {}

    [[nodiscard]] constexpr uint64_t value() const noexcept { return m_value; }
    [[nodiscard]] constexpr bool valid() const noexcept { return m_value != 0; }
    [[nodiscard]] explicit constexpr operator bool() const noexcept { return valid(); }

    friend constexpr bool operator==(WidgetID a, WidgetID b) noexcept { return a.m_value == b.m_value; }
    friend constexpr bool operator!=(WidgetID a, WidgetID b) noexcept { return a.m_value != b.m_value; }
    friend constexpr bool operator<(WidgetID a, WidgetID b) noexcept { return a.m_value < b.m_value; }

private:
    uint64_t m_value = 0;
};

} // namespace MirUI

namespace std {
template <>
struct hash<MirUI::WidgetID> {
    size_t operator()(const MirUI::WidgetID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
}