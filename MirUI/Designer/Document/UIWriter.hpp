// MirUI/Designer/Document/UIWriter.hpp
// 📝 Писатель UI-документов — сохраняет интерфейс в файл .mirui.
//
// Когда пользователь нажимает «Сохранить» в MirUI Designer,
// UIWriter обходит всё дерево виджетов, собирает свойства каждого,
// активную тему, состояние и записывает их в текстовый или бинарный файл.
// Формат файла начинается с заголовка: сигнатура "MIRUI" и версия формата,
// чтобы UIReader потом мог проверить совместимость.
//
// В будущем мы реализуем компактный бинарный формат (или JSON),
// а пока что класс содержит интерфейс и заглушки с подробными
// комментариями, описывающими, как будет устроено сохранение.
//
// Чистый C++23, без платформенных зависимостей.

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

    // ── Основной метод сохранения ────────────────────────────
    // Принимает путь к файлу и документ, который нужно сохранить.
    // Возвращает true, если сохранение прошло успешно.
    // Документ после сохранения помечается как «не изменённый».
    bool save(const std::string& filePath, UIDocument& document) {
        // Открываем файл для записи (перезаписываем, если существует).
        std::ofstream file(filePath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            return false; // не удалось создать файл
        }

        // Записываем заголовок с версией формата.
        writeHeader(file);

        // Записываем метаданные документа (название, рабочее пространство и т.д.).
        writeMetadata(file, document);

        // Записываем все виджеты и их свойства.
        writeWidgets(file, document);

        // Записываем тему.
        writeTheme(file, document);

        // В будущем: записываем состояние (StateStore), команды, выделение...

        file.close();

        // Обновляем метаданные документа.
        document.setFilePath(filePath);
        document.setModified(false);

        return true;
    }

private:
    // ── Запись заголовка ─────────────────────────────────────
    // Формат: MIRUI <major>.<minor>.<patch>
    void writeHeader(std::ofstream& file) {
        file << "MIRUI " << UIFormatVersion::current() << "\n";
    }

    // ── Запись метаданных ────────────────────────────────────
    // Здесь сохраняется имя документа, рабочее пространство,
    // размеры корневого окна и другая общая информация.
    void writeMetadata(std::ofstream& file, const UIDocument& document) {
        file << "# Metadata\n";
        file << "NAME " << document.name() << "\n";

        // Если есть корень, сохраняем его ID для восстановления структуры.
        if (auto* root = document.widgetTree().root()) {
            file << "ROOT " << root->id().value() << "\n";
            file << "ROOT_SIZE " << root->bounds().width << " " << root->bounds().height << "\n";
        }

        file << "# End Metadata\n\n";
    }

    // ── Запись виджетов ──────────────────────────────────────
    // Обходим всё дерево в глубину и для каждого виджета выводим
    // строку с его ID, типом, родителем, именем и границами.
    // Затем перечисляем все дополнительные свойства из карты.
    void writeWidgets(std::ofstream& file, const UIDocument& document) {
        file << "# Widgets\n";

        document.widgetTree().forEach([&](Widget* widget) {
            // Основная информация: ID, тип, родитель, имя, bounds
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

            // Дополнительные свойства из карты (всё, кроме стандартных).
            for (const auto& [key, value] : widget->allProperties()) {
                // Пропускаем стандартные, которые уже записаны в основной строке.
                if (key == "name" || key == "visible" || key == "enabled") continue;

                file << "PROPERTY " << widget->id().value()
                     << " \"" << key << "\" ";
                // Записываем значение в зависимости от типа.
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

    // ── Запись темы ──────────────────────────────────────────
    // Сохраняем текущую тему: цвета, метрики, типографику.
    // Пока что пишем упрощённо, без детализации.
    void writeTheme(std::ofstream& file, const UIDocument& document) {
        file << "# Theme\n";
        // В будущем здесь будет перебор всех полей Theme:
        //   THEME colors.background #FFFFFF
        //   THEME metrics.spacingXS 4.0
        //   ...
        // Для заглушки просто пометим, что тема сохранена.
        file << "THEME default\n";
        file << "# End Theme\n";
    }
};

} // namespace MirUI