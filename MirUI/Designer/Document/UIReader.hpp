// MirUI/Designer/Document/UIReader.hpp
// 📖 Читатель UI-документов — загружает интерфейс из файла .mirui.
//
// Когда пользователь открывает сохранённый ранее интерфейс (например,
// main_window.mirui), UIReader читает файл и восстанавливает
// полный MirUI::UIDocument: дерево виджетов, их свойства, тему,
// активное рабочее пространство и всю дополнительную информацию.
//
// UIReader тесно связан с UIFormatVersion — сначала он проверяет версию
// файла и, если она несовместима, отказывается загружать.
//
// Внутри он устроен как конечный автомат, который построчно
// или по токенам разбирает текстовый (JSON) или бинарный формат.
// В будущем мы реализуем конкретный парсер, а пока что
// этот класс содержит только интерфейс и заглушки с подробными
// комментариями о том, как всё должно работать.
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

class UIReader {
public:
    UIReader() = default;

    // ── Основной метод загрузки ──────────────────────────────
    // Принимает путь к файлу и пустой (или непустой) документ.
    // Загружает все данные из файла в документ.
    // Если загрузка прошла успешно, возвращает true, иначе false.
    // При ошибке документ остаётся в исходном состоянии (частичная загрузка не допускается).
    bool load(const std::string& filePath, UIDocument& document) {
        // В будущем здесь будет открытие файла и определение формата.
        // Пока что мы только проверяем, существует ли файл, и имитируем загрузку.
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false; // файл не найден или не может быть открыт
        }

        // Читаем первую строку — она должна содержать сигнатуру и версию формата.
        std::string headerLine;
        if (!std::getline(file, headerLine)) {
            return false; // файл пустой
        }

        // Проверяем сигнатуру "MIRUI" и версию.
        if (!parseHeader(headerLine)) {
            return false; // неверный формат или версия
        }

        // В будущем здесь будет основной цикл разбора:
        //   while (getline(file, line)) {
        //       if (line starts with "WIDGET") { parseWidget(...); }
        //       else if (line starts with "PROPERTY") { parseProperty(...); }
        //       else if (line starts with "THEME") { parseTheme(...); }
        //   }
        // А пока что просто эмулируем успешную загрузку (документ остаётся пустым).
        (void)document; // чтобы компилятор не ругался на неиспользуемый параметр

        file.close();
        document.setFilePath(filePath);
        document.setModified(false);
        return true;
    }

private:
    // ── Разбор заголовка файла ───────────────────────────────
    // Ожидаемый формат заголовка:
    //   MIRUI 1.0.0
    // Где 1.0.0 — версия формата.
    bool parseHeader(const std::string& line) {
        std::istringstream iss(line);
        std::string magic;
        std::string version;

        if (!(iss >> magic >> version)) {
            return false; // не удалось прочитать два слова
        }

        if (magic != "MIRUI") {
            return false; // неверная сигнатура
        }

        if (!UIFormatVersion::isCompatible(version)) {
            return false; // версия несовместима
        }

        return true;
    }

    // ── Заглушки для будущих парсеров ────────────────────────
    // Эти методы будут разбирать отдельные блоки файла и наполнять документ.
    // Пока они не реализованы, но описывают архитектуру.

    // Разобрать виджет и добавить его в дерево.
    // Входная строка может выглядеть так:
    //   WIDGET id=3 type=Button parent=1
    bool parseWidget(const std::string& /*line*/, UIDocument& /*doc*/) {
        // TODO: реализовать извлечение ID, типа, родителя,
        // создание виджета через AddWidgetCommand или напрямую.
        return false;
    }

    // Разобрать свойство виджета.
    //   PROPERTY widget=3 name="text" value="Нажми меня"
    bool parseProperty(const std::string& /*line*/, UIDocument& /*doc*/) {
        // TODO: применить свойство через widget->setProperty(...)
        return false;
    }

    // Разобрать тему и применить её к документу.
    //   THEME colors.background=#FFFFFF ...
    bool parseTheme(const std::string& /*line*/, UIDocument& /*doc*/) {
        // TODO: прочитать значения и установить через doc.themeManager()
        return false;
    }
};

} // namespace MirUI