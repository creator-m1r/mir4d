// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Geometry/Coordinate/CoordinateSystem.hpp
// ─────────────────────────────────────────────────────────────
// 🧭 ЛОКАЛЬНАЯ СИСТЕМА КООРДИНАТ (Coordinate System)
//
// Представь, что у тебя есть комната с плиткой на полу — это
// "мировая" система координат. Ты можешь приклеить скотчем
// маленькую линейку прямо на стол — это уже "локальная" система.
// Линейка наклонена и сдвинута относительно пола, но измерять
// длину стола в ней гораздо удобнее.
//
// Так и в 3D‑моделировании: у каждой детали своя система координат,
// повёрнутая и сдвинутая относительно общей сцены.
//
// 📐 Основные части:
//   • origin — точка, где расположен "ноль" локальной линейки.
//   • axisX  — направление оси X (единичный вектор).
//   • axisY  — направление оси Y (единичный вектор).
//   • axisZ  — вычисляется автоматически, всегда перпендикулярна X и Y.
//
// Что умеет:
//   • Переводить точки и векторы из локальной системы в мировую и обратно.
//   • Строить систему по началу и нормали, по трём точкам или по двум осям.
//   • Выдавать матрицы 4×4 для быстрой обработки тысяч точек.
//
// Чистый C++23, без платформенных зависимостей.
// ─────────────────────────────────────────────────────────────

#ifndef MIRENGINE_GEOMETRY_COORDINATE_COORDINATESYSTEM_HPP
#define MIRENGINE_GEOMETRY_COORDINATE_COORDINATESYSTEM_HPP

#include <array>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace MirEngine::Geometry::Coordinate {

// ------------------------------
//  Базовые геометрические типы
// ------------------------------

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    constexpr Vector3() noexcept = default;
    constexpr Vector3(double x_, double y_, double z_) noexcept : x(x_), y(y_), z(z_) {}
};

struct Point3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    constexpr Point3() noexcept = default;
    constexpr Point3(double x_, double y_, double z_) noexcept : x(x_), y(y_), z(z_) {}
};

// Векторные операции
[[nodiscard]] inline double dot(const Vector3& a, const Vector3& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] inline Vector3 cross(const Vector3& a, const Vector3& b) noexcept {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

[[nodiscard]] inline double lengthSquared(const Vector3& v) noexcept {
    return dot(v, v);
}

[[nodiscard]] inline double length(const Vector3& v) noexcept {
    return std::sqrt(lengthSquared(v));
}

[[nodiscard]] inline Vector3 normalize(const Vector3& v) {
    const double len = length(v);
    if (len == 0.0) {
        throw std::invalid_argument("Cannot normalize a zero-length vector");
    }
    const double inv = 1.0 / len;
    return { v.x * inv, v.y * inv, v.z * inv };
}

// Арифметика Vector3
[[nodiscard]] inline Vector3 operator+(const Vector3& a, const Vector3& b) noexcept {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
[[nodiscard]] inline Vector3 operator-(const Vector3& a, const Vector3& b) noexcept {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
[[nodiscard]] inline Vector3 operator*(double s, const Vector3& v) noexcept {
    return { s * v.x, s * v.y, s * v.z };
}
[[nodiscard]] inline Vector3 operator*(const Vector3& v, double s) noexcept {
    return s * v;
}
[[nodiscard]] inline Vector3 operator/(const Vector3& v, double s) {
    return { v.x / s, v.y / s, v.z / s };
}

// Операции Point3 и Vector3
[[nodiscard]] inline Vector3 operator-(const Point3& a, const Point3& b) noexcept {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
[[nodiscard]] inline Point3 operator+(const Point3& p, const Vector3& v) noexcept {
    return { p.x + v.x, p.y + v.y, p.z + v.z };
}
[[nodiscard]] inline Point3 operator-(const Point3& p, const Vector3& v) noexcept {
    return { p.x - v.x, p.y - v.y, p.z - v.z };
}

// ------------------------------
//  Система координат
// ------------------------------

class CoordinateSystem {
public:
    using Matrix4 = std::array<std::array<double, 4>, 4>;

    // --- Конструкторы / фабрики ---

    // Тождественная система координат (мировая)
    constexpr CoordinateSystem() noexcept
        : origin_{0.0, 0.0, 0.0}
        , axisX_{1.0, 0.0, 0.0}
        , axisY_{0.0, 1.0, 0.0}
        , axisZ_{0.0, 0.0, 1.0} {}

    // Система по началу и двум (неколлинеарным) осям.
    // Оси ортонормируются: X сохраняет направление, Y подправляется,
    // Z = X × Y.
    CoordinateSystem(const Point3& origin, const Vector3& xAxis, const Vector3& yAxis)
        : origin_(origin)
    {
        axisX_ = normalize(xAxis);

        // Удаляем проекцию Y на X
        Vector3 yPerp = yAxis - dot(yAxis, axisX_) * axisX_;
        double yLenSq = lengthSquared(yPerp);
        if (yLenSq < 1e-12) {
            throw std::invalid_argument(
                "CoordinateSystem: axisX and axisY are nearly parallel");
        }
        axisY_ = normalize(yPerp);
        axisZ_ = cross(axisX_, axisY_); // правый базис
    }

    // Статический фабричный метод: система по началу и нормали (Z).
    // Нормаль становится осью Z, X и Y выбираются произвольно,
    // но ортогонально Z и друг другу.
    [[nodiscard]] static CoordinateSystem fromOriginNormal(
        const Point3& origin, const Vector3& normal)
    {
        Vector3 zDir = normalize(normal);

        // Выбираем вспомогательный вектор, не параллельный zDir.
        Vector3 ref = (std::abs(dot(zDir, Vector3{0.0, 1.0, 0.0})) < 0.999)
                          ? Vector3{0.0, 1.0, 0.0}
                          : Vector3{1.0, 0.0, 0.0};

        Vector3 xDir = normalize(cross(ref, zDir)); // ортогонален zDir
        Vector3 yDir = cross(zDir, xDir);           // правый базис

        CoordinateSystem cs;
        cs.origin_ = origin;
        cs.axisX_  = xDir;
        cs.axisY_  = yDir;
        cs.axisZ_  = zDir;
        return cs;
    }

    // Статический фабричный метод: система по трём точкам.
    // origin -> начало, pointOnX -> задаёт направление X,
    // pointInXYPlane -> точка, лежащая в плоскости XY (но не на оси X).
    [[nodiscard]] static CoordinateSystem fromThreePoints(
        const Point3& origin,
        const Point3& pointOnX,
        const Point3& pointInXYPlane)
    {
        Vector3 xDir = normalize(pointOnX - origin);

        Vector3 temp = pointInXYPlane - origin;
        Vector3 yPerp = temp - dot(temp, xDir) * xDir;
        if (lengthSquared(yPerp) < 1e-12) {
            throw std::invalid_argument(
                "fromThreePoints: points are collinear");
        }
        Vector3 yDir = normalize(yPerp);
        Vector3 zDir = cross(xDir, yDir);

        CoordinateSystem cs;
        cs.origin_ = origin;
        cs.axisX_  = xDir;
        cs.axisY_  = yDir;
        cs.axisZ_  = zDir;
        return cs;
    }

    // ------------------------------
    //  Доступ к осям и началу
    // ------------------------------

    [[nodiscard]] const Point3& origin() const noexcept { return origin_; }
    [[nodiscard]] const Vector3& axisX() const noexcept { return axisX_; }
    [[nodiscard]] const Vector3& axisY() const noexcept { return axisY_; }
    [[nodiscard]] const Vector3& axisZ() const noexcept { return axisZ_; }

    // ------------------------------
    //  Преобразование координат
    // ------------------------------

    // Точка из локальной системы в мировую
    [[nodiscard]] Point3 localToWorldPoint(const Point3& local) const noexcept {
        return origin_ +
               (local.x * axisX_) +
               (local.y * axisY_) +
               (local.z * axisZ_);
    }

    // Вектор из локальной системы в мировую (только поворот)
    [[nodiscard]] Vector3 localToWorldVector(const Vector3& local) const noexcept {
        return (local.x * axisX_) +
               (local.y * axisY_) +
               (local.z * axisZ_);
    }

    // Точка из мировой системы в локальную
    [[nodiscard]] Point3 worldToLocalPoint(const Point3& world) const noexcept {
        Vector3 rel = world - origin_;
        return {
            dot(rel, axisX_),
            dot(rel, axisY_),
            dot(rel, axisZ_)
        };
    }

    // Вектор из мировой системы в локальную (только поворот)
    [[nodiscard]] Vector3 worldToLocalVector(const Vector3& world) const noexcept {
        return {
            dot(world, axisX_),
            dot(world, axisY_),
            dot(world, axisZ_)
        };
    }

    // ------------------------------
    //  Матрицы 4×4
    // ------------------------------

    // Матрица перехода из локальной системы в мировую (M_local→world)
    // Столбцы: axisX, axisY, axisZ, origin (в однородных координатах)
    [[nodiscard]] Matrix4 localToWorldMatrix() const noexcept {
        return {{
            { axisX_.x, axisX_.y, axisX_.z, 0.0 },
            { axisY_.x, axisY_.y, axisY_.z, 0.0 },
            { axisZ_.x, axisZ_.y, axisZ_.z, 0.0 },
            { origin_.x, origin_.y, origin_.z, 1.0 }
        }};
    }

    // Матрица перехода из мировой системы в локальную (M_world→local)
    // Обратная к localToWorld, использует ортонормированность базиса.
    [[nodiscard]] Matrix4 worldToLocalMatrix() const noexcept {
        // Транспонированная матрица поворота и сдвиг: -R^T * origin
        double tx = -dot(axisX_, Vector3{origin_.x, origin_.y, origin_.z});
        double ty = -dot(axisY_, Vector3{origin_.x, origin_.y, origin_.z});
        double tz = -dot(axisZ_, Vector3{origin_.x, origin_.y, origin_.z});

        return {{
            { axisX_.x, axisY_.x, axisZ_.x, 0.0 },
            { axisX_.y, axisY_.y, axisZ_.y, 0.0 },
            { axisX_.z, axisY_.z, axisZ_.z, 0.0 },
            { tx,       ty,       tz,       1.0 }
        }};
    }

private:
    Point3  origin_;
    Vector3 axisX_;
    Vector3 axisY_;
    Vector3 axisZ_;
};

} // namespace MirEngine::Geometry::Coordinate

#endif // MIRENGINE_GEOMETRY_COORDINATE_COORDINATESYSTEM_HPP