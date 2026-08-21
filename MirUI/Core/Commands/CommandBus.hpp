
#pragma once

#include "CommandID.hpp"
#include "CommandContext.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace MirUI {

class CommandBus {
public:

    using Handler = std::function<bool(const CommandContext&)>;
    using CanExecuteChecker = std::function<bool()>;

    bool registerCommand(const CommandID& id, Handler handler) {
        if (m_handlers.find(id) != m_handlers.end()) {
            return false;
        }
        m_handlers[id] = std::move(handler);
        return true;
    }

    void unregisterCommand(const CommandID& id) {
        m_handlers.erase(id);
    }

    bool execute(const CommandID& id, const CommandContext& context = {}) {
        auto it = m_handlers.find(id);
        if (it != m_handlers.end() && it->second) {
            return it->second(context);
        }
        return false;
    }

    bool canExecute(const CommandID& id) const {
        auto it = m_canExecuteCheckers.find(id);
        if (it != m_canExecuteCheckers.end() && it->second) {
            return it->second();
        }

        return m_handlers.find(id) != m_handlers.end();
    }

    void setCanExecuteChecker(const CommandID& id, CanExecuteChecker checker) {
        m_canExecuteCheckers[id] = std::move(checker);
    }

    using Subscriber = std::function<void(const CommandID&, const CommandContext&)>;
    void subscribe(Subscriber subscriber) {
        m_subscribers.push_back(std::move(subscriber));
    }

private:
    std::unordered_map<CommandID, Handler, std::hash<CommandID>> m_handlers;
    std::unordered_map<CommandID, CanExecuteChecker, std::hash<CommandID>> m_canExecuteCheckers;
    std::vector<Subscriber> m_subscribers;

    void notifySubscribers(const CommandID& id, const CommandContext& context) {
        for (auto& sub : m_subscribers) {
            if (sub) {
                sub(id, context);
            }
        }
    }

public:

    bool executeAndNotify(const CommandID& id, const CommandContext& context = {}) {
        bool result = execute(id, context);
        if (result) {
            notifySubscribers(id, context);
        }
        return result;
    }
};

}