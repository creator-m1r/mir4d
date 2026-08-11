// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Geometry/Bounding/BoundingBox.hpp
// ─────────────────────────────────────────────────────────────
// 📦 ОГРАНИЧИВАЮЩИЙ ПАРАЛЛЕЛЕПИПЕД, ВЫРОВНЕННЫЙ ПО ОСЯМ (AABB)
//
// BoundingBox — это минимальный прямоугольный параллелепипед (в 3D)
// или прямоугольник (в 2D), стороны которого параллельны осям координат.
// Он описывается двумя точками: минимум (min) и максимум (max).
// Такой примитив служит для быстрых приближённых проверок:
//   • попала ли точка в область объекта?
//   • пересекаются ли два объекта?
//   • находится ли объект в поле зрения камеры?
//   • какова грубая геометрия набора точек?
//
// Алгоритмическая ценность AABB — в константном времени проверок
// пересечения/вхождения. Если два AABB не пересекаются, то и
// сложная геометрия внутри них точно не пересекается, что позволяет
// отбрасывать заведомо ложные пары на ранних этапах (broad phase).
//
// 📐 Шаблонный дизайн
// Класс параметризован типом вектора `VectorType`, поэтому работает
// с любыми размерностями, поддерживаемыми движком:
//   - BoundingBox<Vec2>   – плоская коробка (2D)
//   - BoundingBox<Vec3>   – объёмная коробка (3D)
//   - BoundingBox<Vec4>   – четырёхмерная коробка (4D)
// В качестве скалярного типа автоматически используется тип координат
// вектора (float или double).
//
// 📦 Используемые компоненты:
//   #include "../Math/Vector/Vector2.hpp"
//   #include "../Math/Vector/Vector3.hpp"
//   #include "../Math/Vector/Vector4.hpp"
//   + стандартные <limits>, <algorithm>, <vector>
//
// Чистый C++23, без внешних зависимостей.
// ─────────────────────────────────────────────────────────────

#pragma once

#include "../Math/Vector/Vector2.hpp"
#include "../Math/Vector/Vector3.hpp"
#include "../Math/Vector/Vector4.hpp"
#include <limits>
#include <algorithm>
#include <vector>

namespace M1R {

template<typename VectorType>
class BoundingBox {
public:
    using Scalar = decltype(VectorType::x);   // float или double

    VectorType min;   // минимальная угловая точка (для 3D: левый-нижний-ближний)
    VectorType max;   // максимальная угловая точка

    // ── КОНСТРУКТОРЫ ────────────────────────────────────────
    
    // По умолчанию: «пустая» коробка.
    // min инициализируется +∞, max — -∞, так что любой последующий вызов
    // extend(point) установит реальные границы.
    BoundingBox() {
        Scalar inf = std::numeric_limits<Scalar>::infinity();
        min = VectorType(inf, inf, inf, inf);
        max = VectorType(-inf, -inf, -inf, -inf);
    }

    // Коробка от двух явно заданных углов.
    // Порядок точек не важен — они не обязаны быть именно min и max,
    // но для предсказуемости лучше передавать их в правильном порядке.
    BoundingBox(const VectorType& min_, const VectorType& max_)
        : min(min_), max(max_) {}

    // ── ФАБРИЧНЫЕ МЕТОДЫ ────────────────────────────────────
    
    // Построить коробку, охватывающую все переданные точки.
    // Если список пуст, вернётся пустая коробка (isEmpty() == true).
    static BoundingBox fromPoints(const std::vector<VectorType>& points) {
        BoundingBox box;
        for (const auto& p : points) {
            box.extend(p);
        }
        return box;
    }

    // Статическая пустая коробка — семантически то же, что BoundingBox{}.
    static BoundingBox empty() {
        return BoundingBox{};
    }

    // ── БАЗОВЫЕ ЗАПРОСЫ СОСТОЯНИЯ ───────────────────────────
    
    // Является ли коробка вырожденной (не содержит ни одной точки)?
    // Пустая, если хотя бы по одной координате min > max.
    [[nodiscard]] bool isEmpty() const {
        return (min.x > max.x) || (min.y > max.y) || (min.z > max.z) || (min.w > max.w);
    }

    // ── РАСШИРЕНИЕ ГРАНИЦ ──────────────────────────────────
    
    // Включить точку в коробку. Границы раздвигаются так, чтобы
    // охватить и текущее содержимое, и новую точку.
    void extend(const VectorType& point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        min.w = std::min(min.w, point.w);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
        max.w = std::max(max.w, point.w);
    }

    // Включить все точки другой коробки. Результат — наименьшая коробка,
    // содержащая обе исходные.
    void extend(const BoundingBox& other) {
        min.x = std::min(min.x, other.min.x);
        min.y = std::min(min.y, other.min.y);
        min.z = std::min(min.z, other.min.z);
        min.w = std::min(min.w, other.min.w);
        max.x = std::max(max.x, other.max.x);
        max.y = std::max(max.y, other.max.y);
        max.z = std::max(max.z, other.max.z);
        max.w = std::max(max.w, other.max.w);
    }

    // ── ГЕОМЕТРИЧЕСКИЕ ХАРАКТЕРИСТИКИ ─────────────────────
    
    // Центр коробки (полусумма min и max).
    [[nodiscard]] VectorType center() const {
        return VectorType(
            (min.x + max.x) * Scalar(0.5),
            (min.y + max.y) * Scalar(0.5),
            (min.z + max.z) * Scalar(0.5),
            (min.w + max.w) * Scalar(0.5)
        );
    }

    // Размер вдоль каждой оси (max - min).
    [[nodiscard]] VectorType size() const {
        return VectorType(
            max.x - min.x,
            max.y - min.y,
            max.z - min.z,
            max.w - min.w
        );
    }

    // Мера объёма: для 2D — площадь, для 3D — объём, для 4D — гиперобъём.
    // Вычисляется как произведение неотрицательных компонент размера.
    [[nodiscard]] Scalar volume() const {
        VectorType s = size();
        return s.x * s.y * s.z * s.w;
    }

    // ── ПРОВЕРКИ ВХОЖДЕНИЯ И ПЕРЕСЕЧЕНИЯ ──────────────────
    
    // Содержит ли коробка указанную точку (включая границы)?
    [[nodiscard]] bool contains(const VectorType& point) const {
        return (point.x >= min.x && point.x <= max.x) &&
               (point.y >= min.y && point.y <= max.y) &&
               (point.z >= min.z && point.z <= max.z) &&
               (point.w >= min.w && point.w <= max.w);
    }

    // Пересекаются ли две коробки?
    // Два AABB не пересекаются, если существует ось, вдоль которой
    // проекции не перекрываются (максимум одного меньше минимума другого).
    [[nodiscard]] bool intersects(const BoundingBox& other) const {
        return !((min.x > other.max.x) || (max.x < other.min.x) ||
                 (min.y > other.max.y) || (max.y < other.min.y) ||
                 (min.z > other.max.z) || (max.z < other.min.z) ||
                 (min.w > other.max.w) || (max.w < other.min.w));
    }

    // ── ОПЕРАТОРЫ СРАВНЕНИЯ ─────────────────────────────────
    bool operator==(const BoundingBox& other) const {
        return min == other.min && max == other.max;
    }
    bool operator!=(const BoundingBox& other) const {
        return !(*this == other);
    }
};

} // namespace M1R