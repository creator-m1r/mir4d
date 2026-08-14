#pragma once

#include "ImportOptions.hpp"
#include "ImportResult.hpp"

#include <functional>
#include <map>
#include <string>

namespace mir::io
{

class ImportService
{
public:
    using ImporterFn = std::function<ImportResult(const std::string&, const ImportOptions&)>;

    [[nodiscard]] ImportResult importFile(
        const std::string& path,
        const ImportOptions& options = {}) const;

    [[nodiscard]] static Format detectFormat(const std::string& path) noexcept;

    static void registerImporter(Format format, ImporterFn fn);
    [[nodiscard]] static bool hasImporter(Format format);

private:
    [[nodiscard]] static std::map<Format, ImporterFn>& registry();
};

} // namespace mir::io
