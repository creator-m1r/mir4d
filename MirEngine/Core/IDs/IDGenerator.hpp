// MirEngine/Core/IDs/IDGenerator.hpp
// 🏭 Генератор уникальных идентификаторов для всех типов сущностей.
//
// В большом проекте сотни и тысячи объектов: детали, компоненты,
// документы, эскизы, элементы геометрии... Каждый должен иметь
// уникальный номер, чтобы его можно было найти, выделить, изменить.
// IDGenerator — это центральная "фабрика" таких номеров.
//
// Как это работает:
//   • Внутри живёт несколько счётчиков — по одному на каждый тип ID.
//   • При вызове createEntity() счётчик Entity увеличивается на 1,
//     и возвращается новый EntityID с этим номером.
//   • Номера никогда не повторяются в рамках одного типа.
//   • Все методы потокобезопасны (atomic), чтобы генератор можно было
//     использовать из разных потоков без блокировок.
//
// Почему не один счётчик на всех?
//   Потому что тогда EntityID, ObjectID и ComponentID будут иметь
//   одинаковые номера, и по числу 42 будет непонятно, кто это —
//   сущность, объект или компонент. Разные счётчики гарантируют,
//   что каждый тип ID уникален в своём пространстве имён.
//
// Использование:
//   IDGenerator gen;
//   EntityID entity = gen.createEntity();       // EntityID{1}
//   ObjectID object = gen.createObject();        // ObjectID{1}
//   EntityID entity2 = gen.createEntity();       // EntityID{2}
//   // entity != object, хотя оба имеют номер 1 — это разные ТИПЫ.
//
// В будущем внутренний механизм можно заменить на UUID или
// распределённую генерацию, не меняя публичный API.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "EntityID.hpp"        // mir::EntityID
#include "ObjectID.hpp"        // mir::ObjectID
#include "ComponentID.hpp"     // mir::ComponentID
#include "FeatureID.hpp"       // mir::FeatureID
#include "DocumentID.hpp"      // mir::DocumentID
#include "ProjectID.hpp"       // mir::ProjectID
#include <atomic>              // std::atomic для потокобезопасности

namespace mir {

class IDGenerator {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Все счётчики начинаются с 0. Первый созданный ID будет иметь номер 1.
    IDGenerator() noexcept = default;

    // ── Генерация ID для каждого типа ────────────────────────

    // Создать новый уникальный идентификатор сущности (Entity).
    // Используется для всех объектов в сцене: детали, камеры, источники света...
    [[nodiscard]] EntityID createEntity() noexcept {
        uint64_t id = m_entityCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        return EntityID{id};
    }

    // Создать новый уникальный идентификатор объекта документа (DocumentObject).
    // Используется для объектов внутри документа CAD: тела, эскизы, элементы...
    [[nodiscard]] ObjectID createObject() noexcept {
        uint64_t id = m_objectCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        return ObjectID{id};
    }

    // Создать новый уникальный идентификатор компонента (Component).
    // Используется для компонентов в сборке (Assembly).
    [[nodiscard]] ComponentID createComponent() noexcept {
        uint64_t id = m_componentCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        return ComponentID{id};
    }

    // Создать новый уникальный идентификатор конструктивного элемента (Feature).
    // Используется для операций моделирования: выдавливание, вращение, отверстие...
    [[nodiscard]] FeatureID createFeature() noexcept {
        uint64_t id = m_featureCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        return FeatureID{id};
    }

    // Создать новый уникальный идентификатор документа.
    // Каждый открытый/созданный документ получает свой номер.
    [[nodiscard]] DocumentID createDocument() noexcept {
        uint64_t id = m_documentCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        return DocumentID{id};
    }

    // Создать новый уникальный идентификатор проекта.
    // Проект верхнего уровня, содержащий документы и настройки.
    [[nodiscard]] ProjectID createProject() noexcept {
        uint64_t id = m_projectCounter.fetch_add(1, std::memory_order_relaxed) + 1;
        return ProjectID{id};
    }

    // ── Сброс всех счётчиков (для тестов) ────────────────────
    // Возвращает генератор в начальное состояние.
    // Не используй в продакшене без крайней необходимости!
    void reset() noexcept {
        m_entityCounter.store(0, std::memory_order_relaxed);
        m_objectCounter.store(0, std::memory_order_relaxed);
        m_componentCounter.store(0, std::memory_order_relaxed);
        m_featureCounter.store(0, std::memory_order_relaxed);
        m_documentCounter.store(0, std::memory_order_relaxed);
        m_projectCounter.store(0, std::memory_order_relaxed);
    }

private:
    std::atomic<uint64_t> m_entityCounter    {0};   // счётчик EntityID
    std::atomic<uint64_t> m_objectCounter    {0};   // счётчик ObjectID
    std::atomic<uint64_t> m_componentCounter {0};   // счётчик ComponentID
    std::atomic<uint64_t> m_featureCounter   {0};   // счётчик FeatureID
    std::atomic<uint64_t> m_documentCounter  {0};   // счётчик DocumentID
    std::atomic<uint64_t> m_projectCounter   {0};   // счётчик ProjectID
};

} // namespace mir