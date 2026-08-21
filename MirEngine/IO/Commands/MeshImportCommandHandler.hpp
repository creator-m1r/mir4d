#pragma once

#include "../../Document/CommandHandler.hpp"
#include "../ImportService.hpp"

#include <utility>

namespace mir::io
{

class MeshImportCommandHandler final : public mir4d::CommandHandler
{
public:
    explicit MeshImportCommandHandler(ImportService service = {})
        : service_(std::move(service))
    {
    }

    [[nodiscard]] mir4d::CommandResult execute(
        const mir4d::Command& command,
        mir::Scene& scene) override;

private:
    ImportService service_;
};

} // namespace mir::io
