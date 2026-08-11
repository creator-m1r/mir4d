// MirEngine/Core/IDs/DocumentID.hpp
// 📄 Типобезопасный идентификатор документа (Document) — уникальный номер
//    документа в проекте, который нельзя спутать с другими ID.
//
// В инженерном проекте может быть открыто несколько документов одновременно:
// чертежи, 3D-модели, спецификации. Каждый документ получает свой уникальный
// идентификатор DocumentID. По этому номеру документ можно найти, сохранить,
// закрыть или передать в другой модуль системы.
//
// DocumentID, как и все ID в MirEngine, — это ОТДЕЛЬНЫЙ ТИП. Компилятор
// не позволит случайно использовать его вместо EntityID, ObjectID и т.д.
//
// Генерацией уникальных DocumentID занимается IDGenerator (createDocument).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cstdint>
#include <functional>

namespace mir {

class DocumentID {
public:
    constexpr DocumentID() noexcept = default;

    explicit constexpr DocumentID(uint64_t value) noexcept
        : m_value(value) {}

    [[nodiscard]] constexpr uint64_t value() const noexcept {
        return m_value;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != 0;
    }

    friend constexpr bool operator==(DocumentID lhs, DocumentID rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(DocumentID lhs, DocumentID rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(DocumentID lhs, DocumentID rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }

private:
    uint64_t m_value = 0;
};

} // namespace mir

namespace std {
template <>
struct hash<mir::DocumentID> {
    std::size_t operator()(const mir::DocumentID& id) const noexcept {
        return hash<uint64_t>{}(id.value());
    }
};
} // namespace std