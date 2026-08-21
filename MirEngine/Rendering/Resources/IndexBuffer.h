
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "GPUBuffer.h"

namespace MirEngine {
namespace Rendering {

class IndexBuffer : public GPUBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void uploadIndices(const std::vector<uint32_t>& indices,
                               BufferUsage usage = BufferUsage::Static) = 0;

    virtual void uploadIndices(const uint32_t* data, size_t count,
                               BufferUsage usage = BufferUsage::Static) = 0;

    [[nodiscard]] virtual size_t getIndexCount() const = 0;
    [[nodiscard]] virtual size_t getSize()       const override = 0;

protected:
    IndexBuffer() = default;
};

}
}