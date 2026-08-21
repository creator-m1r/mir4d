// MirUI/Core/Commands/CommandBus.hpp
// Central command dispatcher – registers handlers, executes commands,
// checks whether a command can be executed, and supports subscribers.
// Pure C++23, no platform dependencies.

#pragma once

#include "CommandID.hpp"
#include "CommandContext.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace MirUI {

class CommandBus {
public:
    // A command handler receives the context and returns true if handled.
    using Handler = std::function<bool(const CommandContext&)>;
    using CanExecuteChecker = std::function<bool()>;

    // Register a handler for a specific command ID.
    // Returns false if the command is already registered (overwrite not allowed by default).
    bool registerCommand(const CommandID& id, Handler handler) {
        if (m_handlers.find(id) != m_handlers.end()) {
            return false; // already registered
        }
        m_handlers[id] = std::move(handler);
        return true;
    }

    // Unregister a command handler.
    void unregisterCommand(const CommandID& id) {
        m_handlers.erase(id);
    }

    // Execute a command in a given context.
    // Returns true if a handler was found and executed successfully.
    bool execute(const CommandID& id, const CommandContext& context = {}) {
        auto it = m_handlers.find(id);
        if (it != m_handlers.end() && it->second) {
            return it->second(context);
        }
        return false;
    }

    // Check whether a command can currently execute.
    // The checker is registered separately.
    bool canExecute(const CommandID& id) const {
        auto it = m_canExecuteCheckers.find(id);
        if (it != m_canExecuteCheckers.end() && it->second) {
            return it->second();
        }
        // If no checker registered, assume the command can execute if a handler exists.
        return m_handlers.find(id) != m_handlers.end();
    }

    // Set a checker for canExecute.
    void setCanExecuteChecker(const CommandID& id, CanExecuteChecker checker) {
        m_canExecuteCheckers[id] = std::move(checker);
    }

    // ---- Subscriptions (for UI updates) ----
    // Subscribe to any command execution (global listener).
    // The callback receives the executed command ID and the context.
    using Subscriber = std::function<void(const CommandID&, const CommandContext&)>;
    void subscribe(Subscriber subscriber) {
        m_subscribers.push_back(std::move(subscriber));
    }

private:
    std::unordered_map<CommandID, Handler, std::hash<CommandID>> m_handlers;
    std::unordered_map<CommandID, CanExecuteChecker, std::hash<CommandID>> m_canExecuteCheckers;
    std::vector<Subscriber> m_subscribers;

    // Helper to notify subscribers after execution (call in execute if desired).
    void notifySubscribers(const CommandID& id, const CommandContext& context) {
        for (auto& sub : m_subscribers) {
            if (sub) {
                sub(id, context);
            }
        }
    }

    // Modify execute to also notify subscribers.
public:
    // Extended execute with subscriber notification (if needed).
    bool executeAndNotify(const CommandID& id, const CommandContext& context = {}) {
        bool result = execute(id, context);
        if (result) {
            notifySubscribers(id, context);
        }
        return result;
    }
};

} // namespace MirUI