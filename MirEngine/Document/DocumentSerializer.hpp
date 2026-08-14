#pragma once

#include "Document.hpp"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace mir
{

/// Deterministic text serializer for the .m1r project container.
class DocumentSerializer
{
public:
    static constexpr std::uint32_t formatMajor = 1;
    static constexpr std::uint32_t formatMinor = 1;

    [[nodiscard]] static std::string serialize(const Document& document)
    {
        std::ostringstream out;
        out << "M1R-DOCUMENT " << formatMajor << '.' << formatMinor << '\n';
        out << "name " << std::quoted(document.name()) << '\n';
        out << "revision " << document.revision() << '\n';
        out << "modified " << (document.isModified() ? 1 : 0) << '\n';
        out << std::setprecision(std::numeric_limits<Scalar>::max_digits10);
        out << "time " << document.time().seconds() << '\n';
        out << "nodes " << document.scene().size() << '\n';

        for (const auto& node : document.scene().nodes())
        {
            if (!node)
            {
                out << "node 0 0 0 0 0 0 0 1 1 1\n";
                continue;
            }

            const Transform3& t = node->transform();
            out << "node "
                << t.position.x << ' ' << t.position.y << ' ' << t.position.z << ' '
                << t.rotationRadians.x << ' ' << t.rotationRadians.y << ' ' << t.rotationRadians.z << ' '
                << t.scale.x << ' ' << t.scale.y << ' ' << t.scale.z << '\n';
        }

        out << "history " << document.history().size() << '\n';
        for (const Command& command : document.history().commands())
        {
            out << "command "
                << command.sequence << ' '
                << command.time.seconds() << ' '
                << commandTypeName(command.type);

            for (const std::string& argument : command.arguments)
                out << ' ' << std::quoted(argument);

            out << '\n';
        }

        out << "end\n";
        return out.str();
    }

    static void save(const Document& document, const std::string& path)
    {
        if (path.empty())
            throw std::invalid_argument("M1R document path must not be empty");

        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file)
            throw std::runtime_error("Unable to open M1R document for writing: " + path);

        file << serialize(document);
        if (!file)
            throw std::runtime_error("Unable to write M1R document: " + path);
    }
};

} // namespace mir
