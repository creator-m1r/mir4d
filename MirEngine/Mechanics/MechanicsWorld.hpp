#pragma once

#include "Mechanism.hpp"
#include "../Time/Time.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace mir
{

class MechanicsWorld
{
public:
    using Id = Mechanism::Id;
    using Time = mir4d::Time;

    [[nodiscard]] std::shared_ptr<Mechanism> create(MechanismType type, std::string name = {})
    {
        const Id id = nextId_++;
        auto mechanism = std::make_shared<Mechanism>(id, type);
        mechanism->setName(std::move(name));
        mechanisms_.emplace(id, mechanism);
        order_.push_back(id);
        return mechanism;
    }

    [[nodiscard]] std::shared_ptr<Mechanism> find(Id id) const noexcept
    {
        const auto it = mechanisms_.find(id);
        return it == mechanisms_.end() ? nullptr : it->second;
    }

    bool remove(Id id) noexcept
    {
        if (mechanisms_.erase(id) == 0)
            return false;

        order_.erase(std::remove(order_.begin(), order_.end(), id), order_.end());
        return true;
    }

    void clear() noexcept
    {
        mechanisms_.clear();
        order_.clear();
        time_ = Time{};
    }

    void update(Time time) noexcept
    {
        if (!time.isValid())
            return;

        const Scalar dt = time.seconds() - time_.seconds();
        if (dt < 0.0)
            return;

        for (const Id id : order_)
        {
            auto mechanism = find(id);
            if (!mechanism)
                continue;

            auto angular = mechanism->angularState();
            angular.velocity += angular.acceleration * dt;
            angular.position += angular.velocity * dt;
            mechanism->setAngularState(angular);

            auto linear = mechanism->linearState();
            linear.velocity += linear.acceleration * dt;
            linear.position += linear.velocity * dt;
            mechanism->setLinearState(linear);
        }

        time_ = time;
    }

    [[nodiscard]] Time time() const noexcept { return time_; }
    [[nodiscard]] std::size_t size() const noexcept { return mechanisms_.size(); }
    [[nodiscard]] bool empty() const noexcept { return mechanisms_.empty(); }

private:
    Id nextId_{1};
    Time time_{};
    std::unordered_map<Id, std::shared_ptr<Mechanism>> mechanisms_{};
    std::vector<Id> order_{};
};

} // namespace mir