#pragma once

#include "MirEngine/BRep/Core/BRepModel.hpp"

#include <memory>
#include <string>

namespace mir::io::step
{

class BRepStepBridge
{
public:

    [[nodiscard]] static std::shared_ptr<mir::BRepModel> read(
        const std::string& path,
        std::string& error);

    [[nodiscard]] static bool write(
        const std::string& path,
        const mir::BRepModel& model,
        std::string& error);

    [[nodiscard]] static std::shared_ptr<mir::BRepModel> readFromText(
        const std::string& text,
        std::string& error);

    [[nodiscard]] static bool writeToText(
        std::string& out,
        const mir::BRepModel& model,
        std::string& error);
};

}
