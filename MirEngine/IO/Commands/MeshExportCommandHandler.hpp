#pragma once

#include "../../Document/CommandHandler.hpp"
#include "../ExportService.hpp"

namespace mir::io
{

class MeshExportCommandHandler final : public CommandHandler
{
public:
    explicit MeshExportCommandHandler(ExportService service = {})
        : service_(std::move(service)) {}

    [[nodiscard]] CommandResult execute(
        const Command& command,
        Scene& scene) override;

private:
    ExportService service_;
};

} // namespace mir::io
