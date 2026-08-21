
#pragma once

#include <cstddef>
#include <vector>

#include "GPUBuffer.h"
#include "Vertex.h"

namespace MirEngine {
namespace Rendering {

class VertexBuffer : public GPUBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void uploadVertices(const std::vector<Vertex>& vertices,
                                BufferUsage usage = BufferUsage::Static) = 0;

    virtual void uploadVertices(const Vertex* data, size_t count,
                                BufferUsage usage = BufferUsage::Static) = 0;

    [[nodiscard]] virtual size_t getVertexCount() const = 0;
    [[nodiscard]] virtual size_t getVertexSize()  const = 0;
    [[nodiscard]] virtual size_t getSize()        const override = 0;

protected:
    VertexBuffer() = default;
};

}
}