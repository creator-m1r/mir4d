#pragma once

#include "ExportOptions.hpp"
#include "ExportResult.hpp"

#include <string>

namespace mir4d
{
class Document;
}

namespace mir::io
{

class ExportService
{
public:
    [[nodiscard]] ExportResult exportFile(
        const std::string& path,
        const mir4d::Document& document,
        const ExportOptions& options = {}) const;

    [[nodiscard]] static Format detectFormat(const std::string& path) noexcept;
};

}
