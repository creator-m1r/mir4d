
#pragma once

#include "../Result.hpp"
#include <string>

namespace mir {

using mir4d::Result;

class IService {
public:
    virtual ~IService() = default;

    virtual Result<void> initialize() = 0;

    virtual void shutdown() = 0;

    [[nodiscard]] virtual bool isReady() const noexcept = 0;

    [[nodiscard]] virtual std::string name() const = 0;
};

}