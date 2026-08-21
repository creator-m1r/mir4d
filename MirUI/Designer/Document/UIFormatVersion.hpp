
#pragma once

#include <cstdint>
#include <string>

namespace MirUI {

struct UIFormatVersion {
    static constexpr uint32_t MAJOR = 1;
    static constexpr uint32_t MINOR = 0;
    static constexpr uint32_t PATCH = 0;

    static std::string current() {
        return std::to_string(MAJOR) + "."
             + std::to_string(MINOR) + "."
             + std::to_string(PATCH);
    }

    static bool isCompatible(uint32_t fileMajor, uint32_t fileMinor) {

        return (fileMajor == MAJOR) && (fileMinor <= MINOR);
    }

    static bool isCompatible(const std::string& versionString) {

        uint32_t major = 0, minor = 0, patch = 0;
        char dot1, dot2;

        size_t pos1 = versionString.find('.');
        if (pos1 == std::string::npos) return false;
        size_t pos2 = versionString.find('.', pos1 + 1);
        if (pos2 == std::string::npos) return false;

        try {
            major = std::stoi(versionString.substr(0, pos1));
            minor = std::stoi(versionString.substr(pos1 + 1, pos2 - pos1 - 1));

        } catch (...) {
            return false;
        }

        return isCompatible(major, minor);
    }
};

}