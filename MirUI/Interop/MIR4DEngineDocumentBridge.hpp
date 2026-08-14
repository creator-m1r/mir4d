#pragma once

#include <cstddef>
#include <cstdint>

namespace MirUI
{

/// Owns the live C++ CAD document used by the SwiftUI workspace.
/// SwiftUI talks to this small, stable boundary rather than constructing
/// MirEngine::Document directly.
class MIR4DEngineDocumentBridge
{
public:
    MIR4DEngineDocumentBridge();
    ~MIR4DEngineDocumentBridge();

    MIR4DEngineDocumentBridge(const MIR4DEngineDocumentBridge&) = delete;
    MIR4DEngineDocumentBridge& operator=(const MIR4DEngineDocumentBridge&) = delete;

    void reset(const char* projectName);
    bool createBox(double width, double depth, double height, std::uint64_t* objectId);
    bool advanceTime(double seconds);

    [[nodiscard]] const char* projectName() const noexcept;
    [[nodiscard]] std::size_t objectCount() const noexcept;
    [[nodiscard]] std::size_t commandCount() const noexcept;
    [[nodiscard]] double currentTime() const noexcept;
    [[nodiscard]] std::uint64_t revision() const noexcept;
    [[nodiscard]] bool isModified() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;

    [[nodiscard]] std::size_t meshVertexCount(std::uint64_t objectId) const noexcept;
    [[nodiscard]] std::size_t meshTriangleCount(std::uint64_t objectId) const noexcept;

private:
    struct Impl;
    Impl* impl_;
};

} // namespace MirUI
