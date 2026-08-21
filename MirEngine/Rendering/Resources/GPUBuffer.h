
#pragma once

#include <cstddef>
#include <cstdint>

namespace MirEngine {
namespace Rendering {

enum class BufferUsage : uint8_t {
    Static  = 0,
    Dynamic = 1,
    Stream  = 2
};

class GPUBuffer {
public:
    virtual ~GPUBuffer() = default;

    virtual void bind()   = 0;
    virtual void unbind() = 0;

    virtual void upload(const void* data, size_t size,
                        BufferUsage usage = BufferUsage::Static) = 0;

    [[nodiscard]] virtual size_t getSize()  const = 0;
    [[nodiscard]] virtual bool   isValid()  const = 0;

protected:
    GPUBuffer() = default;
};

}
}