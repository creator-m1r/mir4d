// MirUI/Schema/CommandSchema.hpp
// 📋 Схема команд MirUI — реестр всех стандартных команд приложения.
//
// В MirUI команды идентифицируются строковыми идентификаторами (CommandID),
// такими как "transform.move", "selection.select", "view.fit".
// Такая гибкость позволяет плагинам добавлять свои команды, но при этом
// стандартные команды должны быть описаны централизованно.
//
// CommandSchema решает несколько задач:
//   1. Определяет перечень всех встроенных команд и их метаданные:
//      категорию, отображаемое имя, описание, иконку, горячие клавиши.
//   2. Служит контрактом между ядром и рендерерами: SwiftUI, WinUI,
//      WebUI читают схему и автоматически строят меню, тулбары,
//      контекстные меню, списки горячих клавиш.
//   3. Позволяет Designer показывать редактор команд: разработчик
//      может привязать любую команду к кнопке, выбрав её из списка,
//      который формируется на основе этой схемы.
//   4. Упрощает локализацию: все строки имён и описаний команд
//      хранятся в одном месте и могут быть переведены.
//
// Структура CommandDescriptor содержит:
//   • id          — CommandID (например, "transform.move")
//   • category    — категория для группировки в меню (например, "Трансформация")
//   • name        — короткое имя на русском (например, "Переместить")
//   • description — подробное описание (тултип)
//   • icon        — идентификатор иконки (не SF Symbol, а платформонезависимый)
//   • defaultShortcut — строка горячей клавиши по умолчанию (например, "Ctrl+M")
//                      в формате, понятном платформенному рендереру.
//
// Сам класс CommandSchema — это статический реестр, который содержит
// вектор всех предопределённых команд и предоставляет методы поиска.
// В будущем он может загружать дополнительные команды из конфигурации.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Core/Commands/CommandID.hpp"
#include "../Foundation/Icons/IconID.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

namespace MirUI {

// ── Дескриптор одной команды ────────────────────────────────
struct CommandDescriptor {
    CommandID   id;               // строковый идентификатор (например, "transform.move")
    std::string category;         // категория (например, "Трансформация", "Файл")
    std::string name;             // отображаемое имя (например, "Переместить")
    std::string description;      // описание для тултипа
    IconID      icon;             // иконка (платформонезависимая строка, как "move")
    std::string defaultShortcut;  // горячая клавиша по умолчанию (например, "Ctrl+M")
};

// ── Схема (реестр) команд ──────────────────────────────────
class CommandSchema {
public:
    // Получить полный список всех зарегистрированных команд.
    [[nodiscard]] static const std::vector<CommandDescriptor>& allCommands() {
        return commands();
    }

    // Найти команду по её идентификатору. Возвращает указатель на дескриптор или nullptr.
    [[nodiscard]] static const CommandDescriptor* find(const CommandID& id) {
        auto& cmds = commands();
        auto it = std::find_if(cmds.begin(), cmds.end(),
            [&id](const CommandDescriptor& desc) { return desc.id == id; });
        return (it != cmds.end()) ? &(*it) : nullptr;
    }

    // Найти все команды из заданной категории.
    [[nodiscard]] static std::vector<const CommandDescriptor*> findByCategory(const std::string& category) {
        std::vector<const CommandDescriptor*> result;
        for (const auto& desc : commands()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

    // Получить список всех уникальных категорий.
    [[nodiscard]] static std::vector<std::string> allCategories() {
        std::vector<std::string> categories;
        for (const auto& desc : commands()) {
            if (std::find(categories.begin(), categories.end(), desc.category) == categories.end()) {
                categories.push_back(desc.category);
            }
        }
        return categories;
    }

private:
    // Статический реестр всех встроенных команд.
    // В будущем может быть дополнен из файла конфигурации.
    static std::vector<CommandDescriptor>& commands() {
        static std::vector<CommandDescriptor> s_commands = {
            // ── Трансформация ────────────────────────────────
            {
                CommandID("transform.move"),
                "Трансформация",
                "Переместить",
                "Переместить выделенный объект",
                IconID("move"),
                "W"
            },
            {
                CommandID("transform.rotate"),
                "Трансформация",
                "Повернуть",
                "Повернуть выделенный объект",
                IconID("rotate"),
                "E"
            },
            {
                CommandID("transform.scale"),
                "Трансформация",
                "Масштабировать",
                "Изменить размер выделенного объекта",
                IconID("scale"),
                "R"
            },

            // ── Выделение ────────────────────────────────────
            {
                CommandID("selection.select"),
                "Выделение",
                "Выделить",
                "Выделить объект под курсором",
                IconID("select"),
                "V"
            },
            {
                CommandID("selection.selectAll"),
                "Выделение",
                "Выделить всё",
                "Выделить все объекты в сцене",
                IconID("select_all"),
                "Ctrl+A"
            },

            // ── Вид ──────────────────────────────────────────
            {
                CommandID("view.fit"),
                "Вид",
                "Вписать в окно",
                "Показать всю сцену целиком",
                IconID("fit"),
                "F"
            },
            {
                CommandID("view.zoomIn"),
                "Вид",
                "Приблизить",
                "Увеличить масштаб вида",
                IconID("zoom_in"),
                "Ctrl+Plus"
            },
            {
                CommandID("view.zoomOut"),
                "Вид",
                "Отдалить",
                "Уменьшить масштаб вида",
                IconID("zoom_out"),
                "Ctrl+Minus"
            },

            // ── Файл ─────────────────────────────────────────
            {
                CommandID("file.new"),
                "Файл",
                "Новый",
                "Создать новый документ",
                IconID("new"),
                "Ctrl+N"
            },
            {
                CommandID("file.open"),
                "Файл",
                "Открыть",
                "Открыть существующий документ",
                IconID("open"),
                "Ctrl+O"
            },
            {
                CommandID("file.save"),
                "Файл",
                "Сохранить",
                "Сохранить текущий документ",
                IconID("save"),
                "Ctrl+S"
            },
            {
                CommandID("file.saveAs"),
                "Файл",
                "Сохранить как…",
                "Сохранить документ под новым именем",
                IconID("save_as"),
                "Ctrl+Shift+S"
            },

            // ── Правка ───────────────────────────────────────
            {
                CommandID("edit.undo"),
                "Правка",
                "Отменить",
                "Отменить последнее действие",
                IconID("undo"),
                "Ctrl+Z"
            },
            {
                CommandID("edit.redo"),
                "Правка",
                "Повторить",
                "Повторить отменённое действие",
                IconID("redo"),
                "Ctrl+Shift+Z"
            },
            {
                CommandID("edit.cut"),
                "Правка",
                "Вырезать",
                "Вырезать выделенный объект в буфер обмена",
                IconID("cut"),
                "Ctrl+X"
            },
            {
                CommandID("edit.copy"),
                "Правка",
                "Копировать",
                "Копировать выделенный объект в буфер обмена",
                IconID("copy"),
                "Ctrl+C"
            },
            {
                CommandID("edit.paste"),
                "Правка",
                "Вставить",
                "Вставить объект из буфера обмена",
                IconID("paste"),
                "Ctrl+V"
            },
            {
                CommandID("edit.delete"),
                "Правка",
                "Удалить",
                "Удалить выделенный объект",
                IconID("delete"),
                "Delete"
            },

            // ── Рабочее пространство ─────────────────────────
            {
                CommandID("workspace.toggleNavigator"),
                "Рабочее пространство",
                "Навигатор",
                "Показать/скрыть панель навигатора",
                IconID("navigator"),
                "Ctrl+1"
            },
            {
                CommandID("workspace.toggleInspector"),
                "Рабочее пространство",
                "Инспектор",
                "Показать/скрыть панель инспектора",
                IconID("inspector"),
                "Ctrl+2"
            },
            {
                CommandID("workspace.toggleTimeline"),
                "Рабочее пространство",
                "Таймлайн",
                "Показать/скрыть панель таймлайна",
                IconID("timeline"),
                "Ctrl+3"
            },
            {
                CommandID("workspace.toggleToolbar"),
                "Рабочее пространство",
                "Панель инструментов",
                "Показать/скрыть основную панель инструментов",
                IconID("toolbar"),
                "Ctrl+4"
            },

            // ── Инструменты измерения ────────────────────────
            {
                CommandID("measure.distance"),
                "Измерение",
                "Расстояние",
                "Измерить расстояние между двумя точками",
                IconID("measure_distance"),
                "D"
            },
            {
                CommandID("measure.angle"),
                "Измерение",
                "Угол",
                "Измерить угол между тремя точками",
                IconID("measure_angle"),
                "A"
            }
        };
        return s_commands;
    }
};

} // namespace MirUI