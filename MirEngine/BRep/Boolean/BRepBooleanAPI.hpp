#pragma once

// MirEngine/BRep/Boolean/BRepBooleanAPI.hpp

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Core/BRepHandles.hpp"

#include <cstdint>
#include <string>

namespace mir
{

enum class BRepBooleanOperation : std::uint8_t
{
    Fuse = 0,
    Cut,
    Common
};

enum class BRepBooleanStatus : std::uint8_t
{
    NotImplemented = 0,
    Success,
    InvalidArgument,
    EmptyResult,
    Failed
};

struct BRepBooleanResult
{
    BRepBooleanStatus status{BRepBooleanStatus::NotImplemented};
    BRepSolidHandle solid{};
    std::string message{"BRep boolean kernel is not implemented yet"};

    [[nodiscard]] bool success() const noexcept
    {
        return status == BRepBooleanStatus::Success && solid.valid();
    }
};

class BRepBooleanAPI
{
public:
    [[nodiscard]] static BRepBooleanResult execute(
        BRepModel& /*outModel*/,
        const BRepModel& /*argumentModel*/,
        BRepSolidHandle argumentSolid,
        const BRepModel& /*toolModel*/,
        BRepSolidHandle toolSolid,
        BRepBooleanOperation operation)
    {
        BRepBooleanResult result{};
        if (!argumentSolid.valid() || !toolSolid.valid())
        {
            result.status = BRepBooleanStatus::InvalidArgument;
            result.message = "Boolean requires valid argument and tool solids";
            return result;
        }
        result.status = BRepBooleanStatus::NotImplemented;
        result.message = booleanOperationName(operation);
        result.message += " is not implemented in this BRep milestone";
        return result;
    }

    [[nodiscard]] static BRepBooleanResult fuse(BRepModel& outModel, const BRepModel& argumentModel,
                                                 BRepSolidHandle argumentSolid, const BRepModel& toolModel,
                                                 BRepSolidHandle toolSolid)
    { return execute(outModel, argumentModel, argumentSolid, toolModel, toolSolid, BRepBooleanOperation::Fuse); }

    [[nodiscard]] static BRepBooleanResult cut(BRepModel& outModel, const BRepModel& argumentModel,
                                                BRepSolidHandle argumentSolid, const BRepModel& toolModel,
                                                BRepSolidHandle toolSolid)
    { return execute(outModel, argumentModel, argumentSolid, toolModel, toolSolid, BRepBooleanOperation::Cut); }

    [[nodiscard]] static BRepBooleanResult common(BRepModel& outModel, const BRepModel& argumentModel,
                                                   BRepSolidHandle argumentSolid, const BRepModel& toolModel,
                                                   BRepSolidHandle toolSolid)
    { return execute(outModel, argumentModel, argumentSolid, toolModel, toolSolid, BRepBooleanOperation::Common); }

private:
    [[nodiscard]] static const char* booleanOperationName(BRepBooleanOperation operation) noexcept
    {
        switch (operation)
        {
            case BRepBooleanOperation::Fuse: return "Fuse";
            case BRepBooleanOperation::Cut: return "Cut";
            case BRepBooleanOperation::Common: return "Common";
        }
        return "Boolean";
    }
};

} // namespace mir
