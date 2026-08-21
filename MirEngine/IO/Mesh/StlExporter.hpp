#pragma once

#include "../Exporter.hpp"

namespace mir::io
{

class StlExporter final : public Exporter
{
public:
    [[nodiscard]] ExportResult exportTo(
        const std::string& path,
        const mir4d::Document& document,
        const ExportOptions& options) override;
};

}
