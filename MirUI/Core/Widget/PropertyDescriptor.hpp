
#pragma once

#include "PropertyType.hpp"
#include "../State/PropertyValue.hpp"
#include <string>
#include <vector>
#include <optional>

namespace MirUI {

struct PropertyDescriptor {
    std::string id;
    std::string displayName;
    PropertyType type;
    PropertyValue defaultValue;

    bool editable = true;
    bool visible  = true;

    std::vector<std::string> enumValues;

    PropertyDescriptor() = default;

    PropertyDescriptor(std::string id,
                       std::string displayName,
                       PropertyType type,
                       PropertyValue defaultValue,
                       std::vector<std::string> enumVals = {})
        : id(std::move(id))
        , displayName(std::move(displayName))
        , type(type)
        , defaultValue(std::move(defaultValue))
        , editable(true)
        , visible(true)
        , enumValues(std::move(enumVals))
    {}

    PropertyDescriptor(std::string id,
                       std::string displayName,
                       PropertyType type,
                       PropertyValue defaultValue,
                       bool editable,
                       bool visible,
                       std::vector<std::string> enumVals = {})
        : id(std::move(id))
        , displayName(std::move(displayName))
        , type(type)
        , defaultValue(std::move(defaultValue))
        , editable(editable)
        , visible(visible)
        , enumValues(std::move(enumVals))
    {}
};

}