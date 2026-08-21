
#pragma once

#include "UIDocument.hpp"
#include "UIFormatVersion.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace MirUI {

class UIReader {
public:
    UIReader() = default;

    bool load(const std::string& filePath, UIDocument& document) {

        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        std::string headerLine;
        if (!std::getline(file, headerLine)) {
            return false;
        }

        if (!parseHeader(headerLine)) {
            return false;
        }

        (void)document;

        file.close();
        document.setFilePath(filePath);
        document.setModified(false);
        return true;
    }

private:

    bool parseHeader(const std::string& line) {
        std::istringstream iss(line);
        std::string magic;
        std::string version;

        if (!(iss >> magic >> version)) {
            return false;
        }

        if (magic != "MIRUI") {
            return false;
        }

        if (!UIFormatVersion::isCompatible(version)) {
            return false;
        }

        return true;
    }

    bool parseWidget(const std::string& , UIDocument& ) {

        return false;
    }

    bool parseProperty(const std::string& , UIDocument& ) {

        return false;
    }

    bool parseTheme(const std::string& , UIDocument& ) {

        return false;
    }
};

}