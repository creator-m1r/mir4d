#pragma once

#include <cstdint>
#include <string>

namespace mir
{

class OpenGLShader
{
public:
    OpenGLShader() noexcept = default;
    ~OpenGLShader() noexcept;

    OpenGLShader(const OpenGLShader&) = delete;
    OpenGLShader& operator=(const OpenGLShader&) = delete;

    [[nodiscard]] bool build(const std::string& vertexSource,
                             const std::string& fragmentSource) noexcept;
    void destroy() noexcept;
    void bind() const noexcept;

    [[nodiscard]] bool valid() const noexcept { return program_ != 0; }
    [[nodiscard]] std::uint32_t program() const noexcept { return program_; }

    [[nodiscard]] int uniformLocation(const char* name) const noexcept;
    void setMat4(int location, const double* matrix) const noexcept;

private:
    std::uint32_t program_{0};
};

} // namespace mir
