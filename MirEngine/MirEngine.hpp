// MirEngine/MirEngine.hpp
// 🚀 Основной публичный заголовок MirEngine.
//
// Этот файл подключает все основные модули движка, включая:
//   • Математические типы: скаляр, углы, векторы, матрицы, кватернионы, лучи.
//   • Вспомогательные утилиты: цвета, трансформации.
//
// Пользователю достаточно написать:
//   #include <MirEngine/MirEngine.hpp>
// чтобы получить доступ ко всем базовым возможностям движка.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

// ── Базовые типы и константы ──────────────────────────────────
#include "Core/Types/Scalar.hpp"      // mir::Scalar, константы Pi, approxEqual
#include "Core/Types/Angle.hpp"       // mir::Angle (радианы/градусы)

// ── Векторы ──────────────────────────────────────────────────
#include "Math/Vector/Vector2.hpp"
#include "Math/Vector/Vector3.hpp"
#include "Math/Vector/Vector4.hpp"

// ── Матрицы ──────────────────────────────────────────────────
#include "Math/Matrix2.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Matrix4.hpp"

// ── Кватернионы и геометрические примитивы ──────────────────
#include "Math/Quaternion/Quaternion.hpp"   // mir::Quaternion (вращения без шарнирного замка)
#include "Geometry/Ray/Ray3.hpp"          // mir::Ray (луч для пересечений)

// ── Утилиты и вспомогательные классы ─────────────────────────
#include "Math/Color.hpp"        // mir::Color (RGBA, операции)
#include "Math/Transform.hpp"    // mir::Transform (позиция + поворот + масштаб)

// ── Пространство имён движка ─────────────────────────────────
// Все типы уже объявлены в namespace mir. Дополнительно здесь можно
// разместить глобальные функции движка, если они будут добавлены позже.