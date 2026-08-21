
#pragma once

#include "UIDocument.hpp"
#include "UIFormatVersion.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace MirUI {

class UIWriter {
public:
    UIWriter() = default;

    bool save(const std::string& filePath, UIDocument& document) {

        std::ofstream file(filePath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }

        writeHeader(file);

        writeMetadata(file, document);

        writeWidgets(file, document);

        writeTheme(file, document);

        file.close();

        document.setFilePath(filePath);
        document.setModified(false);

        return true;
    }

private:

    void writeHeader(std::ofstream& file) {
        file << "MIRUI " << UIFormatVersion::current() << "\n";
    }

    void writeMetadata(std::ofstream& file, const UIDocument& document) {
        file << "# Metadata\n";
        file << "NAME " << document.name() << "\n";

        if (auto* root = document.widgetTree().root()) {
            file << "ROOT " << root->id().value() << "\n";
            file << "ROOT_SIZE " << root->bounds().width << " " << root->bounds().height << "\n";
        }

        file << "# End Metadata\n\n";
    }

    void writeWidgets(std::ofstream& file, const UIDocument& document) {
        file << "# Widgets\n";

        document.widgetTree().forEach([&](Widget* widget) {

            WidgetID parentId = widget->parent() ? widget->parent()->id() : WidgetID{0};
            file << "WIDGET "
                 << widget->id().value() << " "
                 << static_cast<int>(widget->type()) << " "
                 << parentId.value() << " "
                 << "\"" << widget->name() << "\" "
                 << widget->bounds().x << " "
                 << widget->bounds().y << " "
                 << widget->bounds().width << " "
                 << widget->bounds().height << "\n";

            for (const auto& [key, value] : widget->allProperties()) {

                if (key == "name" || key == "visible" || key == "enabled") continue;

                file << "PROPERTY " << widget->id().value()
                     << " \"" << key << "\" ";

                std::visit([&file](const auto& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, bool>) {
                        file << (v ? "true" : "false");
                    } else if constexpr (std::is_same_v<T, int64_t>) {
                        file << v;
                    } else if constexpr (std::is_same_v<T, double>) {
                        file << v;
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        file << "\"" << v << "\"";
                    }
                }, value);
                file << "\n";
            }
        });

        file << "# End Widgets\n\n";
    }

    void writeTheme(std::ofstream& file, const UIDocument& document) {
        file << "# Theme\n";

        file << "THEME default\n";
        file << "# End Theme\n";
    }
};

}