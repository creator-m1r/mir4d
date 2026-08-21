#pragma once

#include <cstdint>

namespace mir4d {

class Event {
public:
    virtual ~Event() = default;

    void setHandled(bool value) noexcept { m_handled = value; }
    [[nodiscard]] bool isHandled() const noexcept { return m_handled; }

private:
    bool m_handled{false};
};

}
