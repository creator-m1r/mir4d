// MirEngine/Math/Quaternion/Quaternion.hpp
// 🔄 Кватернион — математический объект для вращений без проблем.
//
// Обычные углы (Эйлеровы углы: pitch, yaw, roll) страдают от "шарнирного замка" —
// когда две оси вращения совпадают, и одна степень свободы теряется.
// Кватернионы решают эту проблему. Они представляют вращение как одно
// четырёхмерное число (x, y, z, w), которое всегда плавно и однозначно
// описывает любой поворот в 3D-пространстве.
//
// Кватернион можно представить как:
//   • Ось вращения (направление в 3D) + угол поворота вокруг этой оси.
//   • Комбинацию трёх поворотов вокруг осей X, Y, Z (из углов Эйлера).
//   • Результат интерполяции между двумя кватернионами (slerp) —
//     это даёт очень плавные переходы при анимации.
//
// В MirEngine Quaternion будет использоваться:
//   • В Transform (позиция + поворот + масштаб объекта).
//   • В Camera (направление взгляда).
//   • В анимациях (плавное вращение объектов, slerp).
//   • В сборках (Assembly) — поворот компонентов.
//
// Как читать кватернион:
//   Quaternion q{x, y, z, w};
//   x, y, z — компоненты оси вращения (мнимая часть).
//   w — косинус половины угла поворота (действительная часть).
//   Длина кватерниона всегда должна быть = 1.0 (нормализованный).
//
// 🔄 Математический кватернион — для представления вращений без шарнирного замка.
//
// Кватернион — это расширение комплексных чисел, которое используется
// в 3D-графике и CAD для описания поворотов объектов. В отличие от углов
// Эйлера (pitch, yaw, roll), кватернионы никогда не страдают от "шарнирного
// замка" — ситуации, когда две оси вращения совпадают и одна степень
// свободы теряется. Поэтому все серьёзные движки (Unity, Unreal, Blender)
// используют именно кватернионы для хранения ориентации.
//
// Кватернион состоит из четырёх чисел:
//   • x, y, z — компоненты оси вращения (мнимая часть).
//   • w — косинус половины угла поворота (действительная часть).
//
// Длина кватерниона всегда должна быть равна 1.0 (нормализованный),
// иначе при вращении объект будет искажаться.
//
// Как использовать:
//   // Создать кватернион, поворачивающий на 90° вокруг оси Y.
//   Quaternion q = Quaternion::fromAxisAngle(Vector3::unitY(), Angle::degrees(90));
//   Vector3 rotated = q.rotate(someVector);    // повернуть вектор
//   Matrix4 m = q.toMatrix4();                 // получить матрицу для рендеринга
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "../../Core/Types/Angle.hpp"     // mir::Angle
#include "../Vector/Vector3.hpp"          // mir::Vector3
#include "../Matrix3.hpp"          // mir::Matrix3
#include "../Matrix4.hpp"          // mir::Matrix4
#include <cmath>                          // sin, cos, acos, sqrt

namespace mir {

class Quaternion {
public:
    // ── Компоненты кватерниона ───────────────────────────────
    Scalar x = 0.0;   // мнимая часть (ось X)
    Scalar y = 0.0;   // мнимая часть (ось Y)
    Scalar z = 0.0;   // мнимая часть (ось Z)
    Scalar w = 1.0;   // действительная часть (косинус половины угла)

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Quaternion() noexcept = default;

    constexpr Quaternion(Scalar x, Scalar y, Scalar z, Scalar w) noexcept
        : x(x), y(y), z(z), w(w)
    {}

    // ── Статические фабрики ──────────────────────────────────

    // Единичный кватернион (никакого вращения).
    [[nodiscard]] static constexpr Quaternion identity() noexcept {
        return {0.0, 0.0, 0.0, 1.0};
    }

    // Создать кватернион из оси вращения (единичный вектор) и угла.
    // Пример: поворот на 90° вокруг оси X:
    //   Quaternion q = Quaternion::fromAxisAngle(Vector3::unitX(), Angle::degrees(90));
    [[nodiscard]] static Quaternion fromAxisAngle(const Vector3& axis, const Angle& angle) noexcept {
        Scalar halfAngle = angle.radians() * 0.5;
        Scalar s = std::sin(halfAngle);
        Scalar c = std::cos(halfAngle);
        return {axis.x * s, axis.y * s, axis.z * s, c};
    }

    // Создать кватернион из углов Эйлера (порядок: ZYX).
    // yaw   — угол вокруг оси Z (поворот влево-вправо).
    // pitch — угол вокруг оси Y (наклон вверх-вниз).
    // roll  — угол вокруг оси X (крен).
    [[nodiscard]] static Quaternion fromEuler(const Angle& yaw, const Angle& pitch, const Angle& roll) noexcept {
        Scalar halfYaw   = yaw.radians()   * 0.5;
        Scalar halfPitch = pitch.radians() * 0.5;
        Scalar halfRoll  = roll.radians()  * 0.5;

        Scalar cy = std::cos(halfYaw);
        Scalar sy = std::sin(halfYaw);
        Scalar cp = std::cos(halfPitch);
        Scalar sp = std::sin(halfPitch);
        Scalar cr = std::cos(halfRoll);
        Scalar sr = std::sin(halfRoll);

        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        };
    }

    // ── Длина и нормализация ─────────────────────────────────

    // Квадрат длины (быстрее, чем length()).
    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept {
        return x * x + y * y + z * z + w * w;
    }

    // Длина кватерниона (должна быть ≈1.0 для чистого вращения).
    [[nodiscard]] Scalar length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    // Нормализация — приведение к единичной длине.
    // Если длина близка к нулю, возвращает identity().
    [[nodiscard]] Quaternion normalized() const noexcept {
        Scalar len = length();
        if (len < 1e-20) {
            return identity();
        }
        Scalar invLen = 1.0 / len;
        return {x * invLen, y * invLen, z * invLen, w * invLen};
    }

    // ── Обратный кватернион ─────────────────────────────────
    // Вращение в противоположную сторону.
    // Для единичного кватерниона обратный = сопряжённый.
    [[nodiscard]] Quaternion inverse() const noexcept {
        return {-x, -y, -z, w};
    }

    // ── Умножение кватернионов ──────────────────────────────
    // Комбинирует два вращения: сначала применяется a, потом b.
    // Порядок важен! Умножение кватернионов НЕ коммутативно.
    friend Quaternion operator*(const Quaternion& a, const Quaternion& b) noexcept {
        return {
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
        };
    }

    // ── Поворот вектора кватернионом ────────────────────────
    // Применяет вращение к 3D-вектору. Формула: q * v * q⁻¹.
    [[nodiscard]] Vector3 rotate(const Vector3& v) const noexcept {
        // Оптимизированная версия (без создания полного кватерниона для v).
        Vector3 qvec{x, y, z};
        Vector3 uv = Vector3::cross(qvec, v);
        Vector3 uuv = Vector3::cross(qvec, uv);
        return v + (uv * w + uuv) * 2.0;
    }

    // ── Преобразование в матрицу 3×3 ────────────────────────
    // Позволяет использовать кватернион там, где нужна матрица поворота.
    [[nodiscard]] Matrix3 toMatrix3() const noexcept;

    // ── Преобразование в матрицу 4×4 ────────────────────────
    // Матрица 4×4 с поворотом из кватерниона и нулевым переносом.
    [[nodiscard]] Matrix4 toMatrix4() const noexcept;

    // ── Сферическая линейная интерполяция (SLERP) ────────────
    // Плавно переходит от q1 к q2. t = 0 → q1, t = 1 → q2.
    // Используется для анимации вращений.
    [[nodiscard]] static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, Scalar t) noexcept {
        // Косинус угла между кватернионами.
        Scalar dot = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

        // Если кватернионы почти противоположны, выбираем кратчайший путь.
        Quaternion q2mod = q2;
        if (dot < 0.0) {
            q2mod = {-q2.x, -q2.y, -q2.z, -q2.w};
            dot = -dot;
        }

        // Если угол очень маленький, используем линейную интерполяцию.
        if (dot > 0.9995) {
            Quaternion result{
                q1.x + t * (q2mod.x - q1.x),
                q1.y + t * (q2mod.y - q1.y),
                q1.z + t * (q2mod.z - q1.z),
                q1.w + t * (q2mod.w - q1.w)
            };
            return result.normalized();
        }

        // Стандартный SLERP.
        Scalar theta_0 = std::acos(dot);
        Scalar theta = theta_0 * t;
        Scalar sin_theta = std::sin(theta);
        Scalar sin_theta_0 = std::sin(theta_0);

        Scalar s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
        Scalar s1 = sin_theta / sin_theta_0;

        return {
            s0 * q1.x + s1 * q2mod.x,
            s0 * q1.y + s1 * q2mod.y,
            s0 * q1.z + s1 * q2mod.z,
            s0 * q1.w + s1 * q2mod.w
        };
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Quaternion& a, const Quaternion& b) noexcept {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }
    friend constexpr bool operator!=(const Quaternion& a, const Quaternion& b) noexcept {
        return !(a == b);
    }
};

// ── Реализация методов, зависящих от Matrix ──────────────────
// (Вынесены сюда, чтобы избежать циклических зависимостей.)

inline Matrix3 Quaternion::toMatrix3() const noexcept {
    Scalar xx = x * x, yy = y * y, zz = z * z;
    Scalar xy = x * y, xz = x * z, yz = y * z;
    Scalar wx = w * x, wy = w * y, wz = w * z;

    return Matrix3{
        1.0 - 2.0 * (yy + zz),  2.0 * (xy - wz),        2.0 * (xz + wy),
        2.0 * (xy + wz),        1.0 - 2.0 * (xx + zz),  2.0 * (yz - wx),
        2.0 * (xz - wy),        2.0 * (yz + wx),        1.0 - 2.0 * (xx + yy)
    };
}

inline Matrix4 Quaternion::toMatrix4() const noexcept {
    Matrix3 m3 = toMatrix3();
    return Matrix4{
        m3(0,0), m3(0,1), m3(0,2), 0.0,
        m3(1,0), m3(1,1), m3(1,2), 0.0,
        m3(2,0), m3(2,1), m3(2,2), 0.0,
        0.0,     0.0,     0.0,     1.0
    };
}

} // namespace mir