// MirUI/Core/State/StateStore.hpp
// Central store for all UI state values.
// Pure C++23, no platform dependencies.

#pragma once

#include "StateKey.hpp"
#include "StateValue.hpp"
#include <unordered_map>
#include <optional>

namespace MirUI {

class StateStore {
public:
    void set(const StateKey& key, StateValue value) {
        m_values[key] = std::move(value);
    }

    [[nodiscard]] std::optional<StateValue> get(const StateKey& key) const {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool contains(const StateKey& key) const {
        return m_values.find(key) != m_values.end();
    }

    void remove(const StateKey& key) {
        m_values.erase(key);
    }

    void clear() {
        m_values.clear();
    }

private:
    std::unordered_map<StateKey, StateValue, std::hash<StateKey>> m_values;
};

} // namespace MirUI