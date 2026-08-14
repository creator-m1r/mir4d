// MirUI/Designer/Document/UIFormatVersion.hpp
// 📦 Версия формата файла .mirui и проверка совместимости.
//
// Когда мы сохраняем интерфейс в файл, мы записываем туда номер версии.
// Позже, когда MirUI будет развиваться, старые файлы могут немного
// отличаться по структуре. Благодаря этому маленькому заголовку
// программа всегда знает, в каком формате сохранён документ,
// и может корректно его загрузить (или сказать: «Извините, файл
// слишком старый / новый, я не умею его читать»).
//
// Пока у нас самая первая версия — 1.0.0.
// Когда мы добавим что-то, что ломает совместимость (например,
// новый тип виджета или другое хранение свойств), мы увеличим
// мажорную или минорную версию.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <cstdint>
#include <string>

namespace MirUI {

// ── Константы текущей версии ─────────────────────────────────
// Мы используем семантическое версионирование (SemVer):
//   Major.Minor.Patch
//
//   Major — увеличивается, когда изменения ломают обратную совместимость.
//           Например, старые файлы вообще не откроются.
//   Minor — увеличивается, когда добавляется что-то новое,
//           но старые файлы по-прежнему читаются.
//   Patch — для мелких исправлений, которые не меняют формат.
struct UIFormatVersion {
    static constexpr uint32_t MAJOR = 1;
    static constexpr uint32_t MINOR = 0;
    static constexpr uint32_t PATCH = 0;

    // Получить версию в виде строки "1.0.0"
    static std::string current() {
        return std::to_string(MAJOR) + "."
             + std::to_string(MINOR) + "."
             + std::to_string(PATCH);
    }

    // Является ли указанная версия совместимой с текущей?
    // Правило: совместимо, если MAJOR совпадает, а MINOR ≤ текущей.
    // Для простоты мы будем требовать точное совпадение мажора.
    static bool isCompatible(uint32_t fileMajor, uint32_t fileMinor) {
        // Пока строго: мажор должен совпадать.
        // В будущем можно разрешить старые миноры.
        return (fileMajor == MAJOR) && (fileMinor <= MINOR);
    }

    // Удобный метод для проверки сохранённой строки версии.
    static bool isCompatible(const std::string& versionString) {
        // Простейший парсинг "x.y.z"
        // Для надёжности в реальном коде используем sscanf или stringstream,
        // но здесь для примера обойдёмся без зависимостей.
        uint32_t major = 0, minor = 0, patch = 0;
        char dot1, dot2;
        // В C++23 у нас нет std::sscanf, но она обычно доступна в <cstdio>.
        // Чтобы не подключать лишнего, пока просто игнорируем patch.
        // Здесь мы вручную разбираем строку — она простая.
        size_t pos1 = versionString.find('.');
        if (pos1 == std::string::npos) return false;
        size_t pos2 = versionString.find('.', pos1 + 1);
        if (pos2 == std::string::npos) return false;

        try {
            major = std::stoi(versionString.substr(0, pos1));
            minor = std::stoi(versionString.substr(pos1 + 1, pos2 - pos1 - 1));
            // patch игнорируем
        } catch (...) {
            return false;
        }

        return isCompatible(major, minor);
    }
};

} // namespace MirUI