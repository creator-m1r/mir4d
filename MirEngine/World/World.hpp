#pragma once

#include "WorldObject.hpp"
#include "../Time/Time.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mir
{

class World
{
public:
    using Id = WorldObject::Id;
    using Time = mir4d::Time;

    [[nodiscard]] std::shared_ptr<WorldObject> create(WorldObjectType type, std::string name = {})
    {
        const Id id = nextId_++;
        auto object = std::make_shared<WorldObject>(id, type);
        object->setName(std::move(name));
        objects_.emplace(id, object);
        order_.push_back(id);
        return object;
    }

    [[nodiscard]] std::shared_ptr<WorldObject> find(Id id) const noexcept
    {
        const auto it = objects_.find(id);
        return it == objects_.end() ? nullptr : it->second;
    }

    bool remove(Id id) noexcept
    {
        if (objects_.erase(id) == 0)
            return false;
        order_.erase(std::remove(order_.begin(), order_.end(), id), order_.end());
        return true;
    }

    void clear() noexcept
    {
        objects_.clear();
        order_.clear();
        time_ = Time{};
    }

    void advance(Time time) noexcept
    {
        if (!time.isValid() || time.seconds() < time_.seconds())
            return;
        time_ = time;
    }

    void setSettings(const WorldSettings& settings) noexcept { settings_ = settings; }
    [[nodiscard]] const WorldSettings& settings() const noexcept { return settings_; }
    [[nodiscard]] Time time() const noexcept { return time_; }
    [[nodiscard]] std::size_t size() const noexcept { return objects_.size(); }

    template <typename F>
    void forEach(F&& fn) const
    {
        for (const auto& [id, object] : objects_)
            fn(id, object);
    }

private:
    Id nextId_{1};
    Time time_{};
    WorldSettings settings_{};
    std::unordered_map<Id, std::shared_ptr<WorldObject>> objects_{};
    std::vector<Id> order_{};
};

}