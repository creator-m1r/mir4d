// MirEngine/Geometry/Topology/Assembly.hpp
// 🧩 Сборка (Assembly) — иерархическая структура, объединяющая компоненты в единый объект.
//
// В САПР сборка — это способ собрать из отдельных деталей (компонентов)
// одно целое: двигатель из поршней и блока, здание из стен и крыши,
// робот из манипуляторов и контроллера. Assembly хранит дерево компонентов,
// каждый из которых имеет свою позицию, поворот и масштаб (Transform)
// относительно родительского компонента или всей сборки.
//
// Как устроена сборка:
//   • Корень сборки — это фиктивный компонент с Transform::identity(),
//     который представляет саму сборку в мировом пространстве.
//   • Каждый компонент может иметь дочерние компоненты, образуя дерево.
//   • Transform каждого компонента задаёт его положение относительно родителя.
//     Чтобы получить мировую позицию, Transform умножается на мировую позицию родителя.
//   • Компонент может ссылаться на отдельный документ или деталь (по ObjectID).
//     Это позволяет одной детали быть использованной в нескольких местах сборки
//     (instancing).
//
// Основные операции:
//   • addComponent(id, parent, transform) — добавить компонент в сборку.
//   • removeComponent(id) — удалить компонент.
//   • findComponent(id) — найти компонент по ID.
//   • worldTransform(id) — получить мировую трансформацию компонента.
//   • childComponents(parent) — список детей родителя.
//   • rootComponents() — компоненты верхнего уровня (дети корня).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "../../Core/IDs/ComponentID.hpp"   // mir::ComponentID
#include "../../Core/IDs/ObjectID.hpp"      // mir::ObjectID
#include "../../Math/Transform.hpp"         // mir::Transform

namespace mir {

// ── Описание одного компонента сборки ───────────────────────
struct AssemblyComponent {
    ComponentID id;               // уникальный идентификатор компонента
    std::string name;             // человекочитаемое имя ("Поршень #3")
    ObjectID    objectId;         // ссылка на объект документа (может быть невалидным)
    Transform   localTransform;   // трансформация относительно родителя
    ComponentID parentId;         // ID родительского компонента (невалидный для корня)
    std::vector<ComponentID> children; // список ID дочерних компонентов
};

// ── Сборка ──────────────────────────────────────────────────
class Assembly {
public:
    Assembly() {
        // Создаём корневой компонент (сама сборка).
        m_rootId = generateComponentId();
        AssemblyComponent root;
        root.id = m_rootId;
        root.name = "Assembly Root";
        root.localTransform = Transform::identity();
        root.parentId = ComponentID{};   // невалидный = корень
        m_components[m_rootId] = std::move(root);
    }

    // ── Добавление компонента ────────────────────────────────
    // Создаёт новый компонент с заданным родителем и трансформацией.
    // Если parentId невалидный — добавляет в корень сборки.
    // Возвращает ID созданного компонента.
    ComponentID addComponent(const std::string& name,
                             const Transform& localTransform,
                             ComponentID parentId = ComponentID{}) {
        // Если родитель не указан или не существует — используем корень.
        if (!parentId.valid() || !m_components.contains(parentId)) {
            parentId = m_rootId;
        }

        ComponentID newId = generateComponentId();
        AssemblyComponent comp;
        comp.id = newId;
        comp.name = name;
        comp.localTransform = localTransform;
        comp.parentId = parentId;

        // Добавляем в словарь компонентов.
        m_components[newId] = comp;
        // Добавляем в список детей родителя.
        m_components[parentId].children.push_back(newId);

        return newId;
    }

    // ── Удаление компонента ──────────────────────────────────
    // Удаляет компонент и всех его потомков рекурсивно.
    // Возвращает true, если компонент был найден и удалён.
    bool removeComponent(ComponentID id) {
        if (!m_components.contains(id) || id == m_rootId) {
            return false;   // нельзя удалить корень или несуществующий компонент
        }

        // Рекурсивно удаляем детей.
        for (const auto& childId : m_components[id].children) {
            removeComponent(childId);   // рекурсивно удаляем потомков
        }

        // Убираем из списка детей родителя.
        ComponentID parentId = m_components[id].parentId;
        if (parentId.valid() && m_components.contains(parentId)) {
            auto& siblings = m_components[parentId].children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), id), siblings.end());
        }

        // Удаляем из словаря.
        m_components.erase(id);
        return true;
    }

    // ── Поиск компонента ─────────────────────────────────────
    // Возвращает указатель на компонент или nullptr, если не найден.
    [[nodiscard]] const AssemblyComponent* findComponent(ComponentID id) const {
        auto it = m_components.find(id);
        return (it != m_components.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] AssemblyComponent* findComponent(ComponentID id) {
        auto it = m_components.find(id);
        return (it != m_components.end()) ? &it->second : nullptr;
    }

    // ── Получение мировой трансформации компонента ───────────
    // Рекурсивно умножает локальные трансформации от корня до компонента.
    [[nodiscard]] Transform worldTransform(ComponentID id) const {
        if (!m_components.contains(id)) {
            throw std::runtime_error("Assembly::worldTransform: компонент не найден");
        }

        const AssemblyComponent* comp = &m_components.at(id);
        Transform world = comp->localTransform;

        // Поднимаемся по цепочке родителей до корня.
        while (comp->parentId.valid() && comp->parentId != m_rootId) {
            comp = &m_components.at(comp->parentId);
            world = comp->localTransform.combine(world);
        }

        return world;
    }

    // ── Доступ к корню ──────────────────────────────────────
    [[nodiscard]] ComponentID rootId() const noexcept { return m_rootId; }

    // ── Дети родителя (или корня, если parentId невалидный) ──
    [[nodiscard]] std::vector<ComponentID> childComponents(ComponentID parentId = ComponentID{}) const {
        if (!parentId.valid()) {
            parentId = m_rootId;
        }
        auto it = m_components.find(parentId);
        if (it == m_components.end()) {
            return {};
        }
        return it->second.children;
    }

    // ── Количество компонентов (включая корень) ─────────────
    [[nodiscard]] size_t componentCount() const noexcept {
        return m_components.size();
    }

    // ── Итерация по всем компонентам ─────────────────────────
    template <typename Func>
    void forEachComponent(Func&& func) const {
        for (const auto& [id, comp] : m_components) {
            func(id, comp);
        }
    }

private:
    // Словарь всех компонентов сборки (ключ — ComponentID).
    std::unordered_map<ComponentID, AssemblyComponent> m_components;

    // Простой счётчик для генерации уникальных ID компонентов.
    uint64_t m_nextComponentId = 1;

    // Генерация нового уникального ComponentID.
    ComponentID generateComponentId() {
        // Предполагаем, что ComponentID можно создать из целого числа.
        // Если это не так, замените на соответствующий фабричный метод (например, ComponentID::create(...)).
        return ComponentID{m_nextComponentId++};
    }

    // ID корневого компонента (фиктивного).
    ComponentID m_rootId;
};

} // namespace mir