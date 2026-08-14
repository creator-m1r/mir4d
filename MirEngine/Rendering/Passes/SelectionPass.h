// MirEngine/Rendering/Passes/SelectionPass.h
// =================================================================================
// Проход выделения (Selection Pass).
//
// Отвечает за отрисовку визуального выделения объектов сцены (например,
// подсветка контура, изменение цвета или обводка). На данном этапе создаёт
// простой тестовый контур, демонстрирующий работу механизма.
//
// В будущем будет взаимодействовать с Selection (MirEngine/Scene/Selection),
// получая набор выделенных Node и применяя соответствующие визуальные эффекты.
//
// Архитектура:
//   - Не зависит от API, использует RenderDevice и Shader.
//   - Создаёт собственный VAO для графики выделения (линии/меш обводки).
//   - Материал выделения может иметь особые свойства (например, стробоскопический
//     эффект, свечение), но сейчас просто яркий цвет.
// =================================================================================

#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/ShaderLibrary.h"
#include <memory>
#include <vector>

namespace MirEngine {
namespace Rendering {

class RenderDevice;
class RenderContext;
class VertexArray;
class VertexBuffer;

class SelectionPass : public RenderPass {
public:
    explicit SelectionPass(ShaderLibrary& shaderLibrary);

    // --------------------------------------------------------------------------
    // Инициализация: создаёт тестовый контур выделения.
    // --------------------------------------------------------------------------
    bool initialize(RenderDevice& device);

    // --------------------------------------------------------------------------
    // Основной метод: рисует выделение для выбранных объектов.
    // Пока просто рисует тестовый контур.
    // --------------------------------------------------------------------------
    void execute(RenderContext& context,
                 Scene& scene,
                 Camera& camera,
                 RenderDevice& device) override;

private:
    ShaderLibrary& m_shaderLibrary;

    // Дескрипторы
    MaterialHandle m_materialHandle = 0;
    MeshHandle     m_meshHandle = 0;

    // Ресурсы
    std::shared_ptr<VertexArray>  m_vao;
    std::shared_ptr<VertexBuffer> m_vb;

    // Временный список вершин для тестового контура
    std::vector<Vertex> m_vertices;
};

} // namespace Rendering
} // namespace MirEngine