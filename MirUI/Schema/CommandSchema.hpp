
#pragma once

#include "../Core/Commands/CommandID.hpp"
#include "../Foundation/Icons/IconID.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

namespace MirUI {

struct CommandDescriptor {
    CommandID   id;
    std::string category;
    std::string name;
    std::string description;
    IconID      icon;
    std::string defaultShortcut;
};

class CommandSchema {
public:

    [[nodiscard]] static const std::vector<CommandDescriptor>& allCommands() {
        return commands();
    }

    [[nodiscard]] static const CommandDescriptor* find(const CommandID& id) {
        auto& cmds = commands();
        auto it = std::find_if(cmds.begin(), cmds.end(),
            [&id](const CommandDescriptor& desc) { return desc.id == id; });
        return (it != cmds.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static std::vector<const CommandDescriptor*> findByCategory(const std::string& category) {
        std::vector<const CommandDescriptor*> result;
        for (const auto& desc : commands()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

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

    static std::vector<CommandDescriptor>& commands() {
        static std::vector<CommandDescriptor> s_commands = {

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

}