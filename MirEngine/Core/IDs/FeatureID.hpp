// MirEngine/Core/IDs/FeatureID.hpp
// 🏗️ Типобезопасный идентификатор конструктивного элемента (Feature) —
//    уникальный номер операции моделирования, который нельзя спутать с другими ID.
//
// В параметрическом CAD каждая деталь строится как последовательность операций
// (features): «выдавить эскиз на 100 мм», «сделать скругление радиусом 5 мм»,
// «просверлить отверстие». Каждая такая операция получает свой уникальный
// идентификатор FeatureID. По этому номеру операцию можно найти в дереве
// построения, отредактировать её параметры, временно подавить или удалить.
//
// FeatureID — это ОТДЕЛЬНЫЙ ТИП. Нельзя передать FeatureID туда, где ожидается
// EntityID, ObjectID или ComponentID. Благодаря этому компилятор не даст
// перепутать операцию выдавливания с трёхмерным телом или компонентом сборки.
//
// Генерацией уникальных FeatureID занимается IDGenerator (createFeature).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>
#include <functional>

namespace mir {

class FeatureID {
public:
    constexpr FeatureID() noexcept = default;

    explicit constexpr FeatureID(uint64_t value) noexcept
        : m_value(value) {}

    [[nodiscard]] constexpr uint64_t value() const noexcept {
        return m_value;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != 0;
    }

    friend constexpr bool operator==(FeatureID lhs, FeatureID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(FeatureID lhs, FeatureID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(FeatureID lhs, FeatureID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }

private:
    uint64_t m_value = 0;
};

} // namespace mir

namespace std {
template <>
struct hash<mir::FeatureID> {
    std::size_t operator()(const mir::FeatureID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std