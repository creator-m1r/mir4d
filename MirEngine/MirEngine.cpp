// MirEngine/MirEngine.cpp
// 🚀 Основной файл движка MirEngine.
//
// Подключает все модули движка и служит корневым файлом трансляции
// при сборке в статическую или динамическую библиотеку.
// Вся реализация, зависящая от других модулей, либо уже определена
// в заголовках (как inline/template), либо будет добавлена по мере
// развития движка.
//
// На данный момент вся математическая часть полностью самодостаточна
// в заголовочных файлах. Этот .cpp нужен для:
//   • Гарантии компиляции всех включённых заголовков.
//   • Инициализации статических данных (если появятся).
//   • Возможного экспорта из DLL (если потребуется).
//
// Чистый C++23, без внешних зависимостей.

// Основные математические заголовки (все пути от корня MirEngine/)
#include "Core/Types/Scalar.hpp"      // Scalar
#include "Math/Vector/Vector2.hpp"
#include "Math/Vector/Vector3.hpp"
#include "Math/Vector/Vector4.hpp"
#include "Math/Matrix2.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Quaternion/Quaternion.hpp"
#include "Geometry/Ray/Ray3.hpp"      // Ray3 (предполагаем, что луч у нас здесь)
#include "Core/Types/Angle.hpp"             // Если Angle находится в Math, оставь. Если нет — удали.
#include "Math/Color.hpp"
#include "Math/Transform.hpp"

namespace mir {

// ── Глобальные функции движка ────────────────────────────────

// Инициализация движка (пустая заглушка).
void InitializeEngine() {
    // Здесь можно разместить инициализацию глобальных состояний.
}

// Завершение работы движка.
void ShutdownEngine() {
    // Очистка ресурсов.
}

} // namespace mir