
#pragma once

#include <string>
#include <functional>

namespace MirUI {

class CommandID {
public:
    CommandID() = default;
    explicit CommandID(std::string value) : m_value(std::move(value)) {}

    [[nodiscard]] const std::string& value() const { return m_value; }

    friend bool operator==(const CommandID& lhs, const CommandID& rhs) {
        return lhs.m_value == rhs.m_value;
    }
    friend bool operator!=(const CommandID& lhs, const CommandID& rhs) {
        return lhs.m_value != rhs.m_value;
    }
    friend bool operator<(const CommandID& lhs, const CommandID& rhs) {
        return lhs.m_value < rhs.m_value;
    }

private:
    std::string m_value;
};

}

namespace std {
template <>
struct hash<MirUI::CommandID> {
    std::size_t operator()(const MirUI::CommandID& id) const {
        return hash<std::string>{}(id.value());
    }
};
}