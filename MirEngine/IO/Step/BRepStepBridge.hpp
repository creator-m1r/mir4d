#pragma once

// MirEngine/IO/Step/BRepStepBridge.hpp
//
// Нативный (без OpenCASCADE) мост между ISO 10303-21 STEP и ядром B-Rep MIR 4D.
// Читает точную B-Rep геометрию (FACE_SURFACE: PLANE, CYLINDRICAL_SURFACE;
// EDGE_CURVE: LINE, CIRCLE) в MirEngine::BRepModel и сериализует BRepModel
// обратно в STEP (ADVANCED_BREP). Тесселяция BRepModel в render mesh выполняется
// отдельно через MirEngine::BRepTessellator.

#include "MirEngine/BRep/Core/BRepModel.hpp"

#include <memory>
#include <string>

namespace mir::io::step
{

class BRepStepBridge
{
public:
    /// Читает STEP-файл и строит точную B-Rep модель MIR 4D.
    /// Возвращает nullptr и заполняет error при неудаче.
    [[nodiscard]] static std::shared_ptr<mir::BRepModel> read(
        const std::string& path,
        std::string& error);

    /// Сериализует BRepModel в STEP-файл (ADVANCED_BREP).
    [[nodiscard]] static bool write(
        const std::string& path,
        const mir::BRepModel& model,
        std::string& error);

    /// Читает STEP из текста (удобно для тестов и in-memory обработки).
    [[nodiscard]] static std::shared_ptr<mir::BRepModel> readFromText(
        const std::string& text,
        std::string& error);

    /// Сериализует BRepModel в STEP-текст.
    [[nodiscard]] static bool writeToText(
        std::string& out,
        const mir::BRepModel& model,
        std::string& error);
};

} // namespace mir::io::step
