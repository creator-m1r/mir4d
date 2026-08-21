
#pragma once

#include <string>
#include <functional>

namespace MirUI {

class ThemeID {
public:

    ThemeID() = default;
    explicit ThemeID(std::string value) : m_value(std::move(value)) {}

    [[nodiscard]] const std::string& value() const { return m_value; }

    friend bool operator==(const ThemeID& a, const ThemeID& b) {
        return a.m_value == b.m_value;
    }
    friend bool operator!=(const ThemeID& a, const ThemeID& b) {
        return a.m_value != b.m_value;
    }
    friend bool operator<(const ThemeID& a, const ThemeID& b) {
        return a.m_value < b.m_value;
    }

private:
    std::string m_value;
};

}

namespace std {
template <>
struct hash<MirUI::ThemeID> {
    std::size_t operator()(const MirUI::ThemeID& id) const {
        return hash<std::string>{}(id.value());
    }
};
}