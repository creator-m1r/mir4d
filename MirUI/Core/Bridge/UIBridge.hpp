
#pragma once

#include "../Widget/WidgetTreeSnapshot.hpp"
#include "../Events/Event.hpp"
#include <memory>

namespace MirUI {

class UIBridge {
public:
    virtual ~UIBridge() = default;

    virtual void present(const WidgetTreeSnapshot& snapshot) = 0;

    virtual void update() = 0;

    virtual void dispatchEvent(const Event& event) = 0;

    [[nodiscard]] virtual const WidgetTreeSnapshot* lastSnapshot() const {
        return m_lastSnapshot.get();
    }

protected:

    void setLastSnapshot(std::unique_ptr<WidgetTreeSnapshot> snapshot) {
        m_lastSnapshot = std::move(snapshot);
    }

private:
    std::unique_ptr<WidgetTreeSnapshot> m_lastSnapshot;
};

}