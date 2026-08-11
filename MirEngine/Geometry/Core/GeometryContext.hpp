// ─────────────────────────────────────────────────────────────
//  MirEngine/Geometry/Core/GeometryContext.hpp
// ─────────────────────────────────────────────────────────────
//  ГЕОМЕТРИЧЕСКИЙ КОНТЕКСТ — «рабочий стол» для всех фигур
//
// В системе Мир 4D геометрические объекты (точки, линии, тела)
// не существуют сами по себе. Они хранятся в едином реестре —
// GeometryContext, который:
//   • Создаёт новые объекты и выдаёт им уникальный номер (ID).
//   • Удаляет объекты, когда они больше не нужны.
//   • Быстро находит объект по его номеру.
//   • Следит за порядком и не теряет объекты.
//   • Сообщает другим частям программы о появлении/исчезновении
//     объектов через систему событий (EventBus).
//
// Представь, что это большая картотека в библиотеке.
// Каждая книга (геометрический объект) получает свой номер.
// Библиотекарь (GeometryContext) кладёт книгу на полку,
// а если книгу списывают — убирает. И сразу делает запись
// в журнале событий: «Книга №42 добавлена», «Книга №7 удалена».
//
// Все методы безопасны для использования из нескольких потоков
// благодаря внутреннему замку (std::mutex).
//
// Чистый C++23, без внешних зависимостей.
// ─────────────────────────────────────────────────────────────

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>   // <-- ВОТ ЭТО БЫЛО ПРОПУЩЕНО! std::function живёт здесь

// ─────────────────────────────────────────────────────────────
// Вспомогательные типы (обычно они лежат в отдельных файлах,
// но для самодостаточности мы определяем их прямо здесь)
// ─────────────────────────────────────────────────────────────

namespace mir {

// -------------------------------------------------------------------
// EntityID – уникальный номер геометрической сущности
// -------------------------------------------------------------------
struct EntityID {
    uint64_t value = 0;
    bool operator==(const EntityID& other) const { return value == other.value; }
    bool operator!=(const EntityID& other) const { return value != other.value; }
    // Для использования в unordered_map
    struct Hash {
        size_t operator()(EntityID id) const noexcept {
            return std::hash<uint64_t>{}(id.value);
        }
    };
};

// -------------------------------------------------------------------
// Transform – положение, поворот и масштаб объекта
// (заглушка, реальная версия в Math/Transform.hpp)
// -------------------------------------------------------------------
struct Transform {
    double position[3] = {0,0,0};
    double rotation[4] = {0,0,0,1}; // кватернион (x,y,z,w)
    double scale[3]    = {1,1,1};

    static Transform identity() { return Transform{}; }
};

// -------------------------------------------------------------------
// GeometryObject – сам объект сцены (заглушка)
// -------------------------------------------------------------------
struct GeometryObject {
    EntityID id;
    Transform transform;
    std::string name = "Unnamed";
};

// -------------------------------------------------------------------
// Простейшая шина событий (EventBus)
// -------------------------------------------------------------------
class EventBus {
public:
    // Синглтон: единственный экземпляр на всё приложение
    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    // Подписка на события определённого типа
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler) {
        // Упрощённая версия: храним все обработчики в одном списке
        // В реальном проекте здесь фильтрация по типу.
        m_handlers.push_back(
            [handler = std::move(handler)](const void* event) {
                handler(*static_cast<const EventType*>(event));
            }
        );
    }

    // Публикация события
    template<typename EventType>
    void publish(const EventType& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& h : m_handlers) {
            h(&event);
        }
    }

private:
    EventBus() = default;
    std::vector<std::function<void(const void*)>> m_handlers;
    std::mutex m_mutex;
};

// -------------------------------------------------------------------
// События геометрического контекста
// -------------------------------------------------------------------
struct ObjectAddedEvent {
    EntityID entityId;
};

struct ObjectRemovedEvent {
    EntityID entityId;
};

// -------------------------------------------------------------------
// Простейший генератор идентификаторов (IDGenerator)
// -------------------------------------------------------------------
class IDGenerator {
public:
    EntityID createEntity() {
        return EntityID{++m_nextId};
    }
private:
    uint64_t m_nextId = 0;
};


// ╔══════════════════════════════════════════════════════════╗
// ║            ГЛАВНЫЙ КЛАСС: GeometryContext                ║
// ╚══════════════════════════════════════════════════════════╝

class GeometryContext {
public:
    // ── Конструктор и деструктор ────────────────────────────
    GeometryContext() = default;
    ~GeometryContext() = default;

    // Запрещаем копирование контекста (он должен быть единственным)
    GeometryContext(const GeometryContext&) = delete;
    GeometryContext& operator=(const GeometryContext&) = delete;

    // ─────────────────────────────────────────────────────────
    //  Создать новый геометрический объект
    //    Возвращает его уникальный EntityID.
    //    Объект размещается в куче и сохраняется в картотеке.
    //    После создания отправляет событие ObjectAddedEvent.
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] EntityID createEntity() {
        // Генерируем новый уникальный номер для сущности
        EntityID id = m_idGenerator.createEntity();

        // Создаём сам объект (пока с пустой трансформацией)
        auto object = std::make_unique<GeometryObject>();
        object->id = id;                    // запоминаем номер внутри объекта
        object->transform = Transform::identity();  // начальное положение

        // Кладём в хранилище под замком (чтобы никто не мешал)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_objects.emplace(id, std::move(object));
        }

        // Сообщаем всем подписчикам: «Появился новый объект!»
        EventBus::instance().publish(ObjectAddedEvent{id});

        return id;
    }

    // ─────────────────────────────────────────────────────────
    //  Удалить геометрический объект по его ID.
    //    Возвращает true, если объект существовал и был удалён.
    //    Отправляет событие ObjectRemovedEvent.
    // ─────────────────────────────────────────────────────────
    bool removeEntity(EntityID id) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_objects.find(id);
        if (it == m_objects.end()) {
            return false;   // такой книги в картотеке нет
        }

        // Удаляем объект (unique_ptr сам освободит память)
        m_objects.erase(it);

        // Оповещаем систему: «Объект удалён»
        EventBus::instance().publish(ObjectRemovedEvent{id});

        return true;
    }

    // ─────────────────────────────────────────────────────────
    //  Найти объект по ID.
    //    Возвращает "голый" указатель (не владеющий).
    //    Если объект не найден — возвращает nullptr.
    //    Указатель действителен до тех пор, пока объект
    //    не удалён из контекста.
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] GeometryObject* findEntity(EntityID id) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_objects.find(id);
        return (it != m_objects.end()) ? it->second.get() : nullptr;
    }

    // Константная версия для чтения
    [[nodiscard]] const GeometryObject* findEntity(EntityID id) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_objects.find(id);
        return (it != m_objects.end()) ? it->second.get() : nullptr;
    }

    // ─────────────────────────────────────────────────────────
    //  Количество объектов в контексте
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] size_t getEntityCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_objects.size();
    }

    // ─────────────────────────────────────────────────────────
    //  Очистить весь контекст (удалить все объекты)
    //    Отправляет по событию на каждый удаляемый объект.
    // ─────────────────────────────────────────────────────────
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Для каждого объекта отправляем событие удаления
        for (const auto& [id, _] : m_objects) {
            EventBus::instance().publish(ObjectRemovedEvent{id});
        }

        // Очищаем хранилище (все unique_ptr автоматически удалятся)
        m_objects.clear();
    }

private:
    //  Картотека: ID → указатель на объект
    std::unordered_map<EntityID, std::unique_ptr<GeometryObject>, EntityID::Hash> m_objects;

    //  Замок для безопасной работы из нескольких потоков
    mutable std::mutex m_mutex;

    //  Генератор уникальных идентификаторов
    IDGenerator m_idGenerator;
};

} // namespace mir