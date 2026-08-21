// MirUI/Core/State/StateKey.hpp
// String-based key for the state store.
// Pure C++23, no platform dependencies.

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

} // namespace MirUI

// Specialise std::hash so StateKey can be used in unordered_map.
namespace std {
template <>
struct hash<MirUI::StateKey> {
    std::size_t operator()(const MirUI::StateKey& key) const {
        return hash<std::string>()(key.name());
    }
};
} // namespace std