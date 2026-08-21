
#pragma once

#include <memory>
#include <cstdint>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace MirEngine {
namespace Rendering {

class VertexArray {
public:
    virtual ~VertexArray() = default;

    virtual void bind()   = 0;
    virtual void unbind() = 0;

    virtual void setVertexBuffer(std::shared_ptr<VertexBuffer> vb) = 0;
    virtual void setIndexBuffer (std::shared_ptr<IndexBuffer>  ib) = 0;

    [[nodiscard]] virtual std::shared_ptr<VertexBuffer> getVertexBuffer() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IndexBuffer>  getIndexBuffer()  const = 0;

    [[nodiscard]] virtual uint32_t getElementCount() const = 0;

    [[nodiscard]] virtual bool hasIndexBuffer() const = 0;

    [[nodiscard]] virtual bool isValid() const = 0;

protected:
    VertexArray() = default;
};

}
}