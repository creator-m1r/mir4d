
#pragma once

#include <string>
#include <functional>

namespace MirUI {

class StateKey {
public:
    StateKey() = default;
    explicit StateKey(std::string name) : m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool operator==(const StateKey& other) const {
        return m_name == other.m_name;
    }

private:
    std::string m_name;
};

}

namespace std {
template <>
struct hash<MirUI::StateKey> {
    std::size_t operator()(const MirUI::StateKey& key) const {
        return hash<std::string>()(key.name());
    }
};
}