
#pragma once

#include <string>
#include <functional>

namespace MirUI {

class IconID {
public:
    IconID() = default;
    explicit IconID(std::string value) : m_value(std::move(value)) {}

    [[nodiscard]] const std::string& value() const { return m_value; }

    friend bool operator==(const IconID& lhs, const IconID& rhs) {
        return lhs.m_value == rhs.m_value;
    }
    friend bool operator!=(const IconID& lhs, const IconID& rhs) {
        return lhs.m_value != rhs.m_value;
    }
    friend bool operator<(const IconID& lhs, const IconID& rhs) {
        return lhs.m_value < rhs.m_value;
    }

private:
    std::string m_value;
};

}

namespace std {
template <>
struct hash<MirUI::IconID> {
    std::size_t operator()(const MirUI::IconID& id) const {
        return hash<std::string>{}(id.value());
    }
};
}