
#pragma once

#include <cstddef>

namespace MirEngine {
namespace Rendering {

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    Vector2() = default;
    Vector2(float x_, float y_) : x(x_), y(y_) {}
};

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3() = default;
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct Vector4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vector4() = default;
    Vector4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;

    Vertex() = default;
    Vertex(const Vector3& pos, const Vector3& norm, const Vector2& texcoord)
        : position(pos), normal(norm), uv(texcoord) {}
};

static_assert(sizeof(Vertex) == 32, "Vertex size must be 32 bytes (12+12+8)");
static_assert(offsetof(Vertex, position) == 0,  "position must be at offset 0");
static_assert(offsetof(Vertex, normal)   == 12, "normal must be at offset 12");
static_assert(offsetof(Vertex, uv)       == 24, "uv must be at offset 24");

}
}