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

class Exporter
{
public:
    virtual ~Exporter() = default;

    [[nodiscard]] virtual ExportResult exportTo(
        const std::string& path,
        const mir4d::Document& document,
        const ExportOptions& options) = 0;
};

}
