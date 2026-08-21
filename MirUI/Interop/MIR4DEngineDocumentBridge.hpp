#pragma once

#include <cstddef>
#include <cstdint>

namespace MirUI
{
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
}

extern "C"
{
    void* MIR4DDocumentCreate();
    void MIR4DDocumentDestroy(void* handle);
    void MIR4DDocumentReset(void* handle, const char* projectName);
    bool MIR4DDocumentCreateBox(void* handle, double width, double depth, double height, std::uint64_t* objectId);
    bool MIR4DDocumentAdvanceTime(void* handle, double seconds);
    std::size_t MIR4DDocumentObjectCount(void* handle);
    std::size_t MIR4DDocumentCommandCount(void* handle);
    double MIR4DDocumentCurrentTime(void* handle);
    std::uint64_t MIR4DDocumentRevision(void* handle);
    bool MIR4DDocumentIsModified(void* handle);
    bool MIR4DDocumentIsValid(void* handle);
}
