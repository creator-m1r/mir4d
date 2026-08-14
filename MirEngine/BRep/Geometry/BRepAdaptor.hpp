#pragma once

// MirEngine/BRep/Geometry/BRepAdaptor.hpp

#include "MirEngine/BRep/Core/BRepModel.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace mir
{

class BRepAdaptor_Curve
{
public:
    BRepAdaptor_Curve() = default;
    BRepAdaptor_Curve(const BRepModel& model, BRepEdgeHandle edgeHandle) noexcept
    {
        (void)bind(model, edgeHandle);
    }

    [[nodiscard]] bool bind(const BRepModel& model, BRepEdgeHandle edgeHandle) noexcept
    {
        model_ = &model;
        edge_ = model.topology().edge(edgeHandle);
        curve_ = edge_ && !edge_->degenerated ? model.geometry().curve(edge_->curve) : nullptr;
        return curve_ != nullptr && curve_->isValid();
    }

    [[nodiscard]] bool isBound() const noexcept { return model_ && edge_ && curve_; }
    [[nodiscard]] BRepRange range() const noexcept { return edge_ ? edge_->range : BRepRange{}; }
    [[nodiscard]] BRepCurveType type() const noexcept { return curve_ ? curve_->type : BRepCurveType::Unknown; }
    [[nodiscard]] Vector3 value(Scalar u) const noexcept { return isBound() ? curve_->valueAt(u) : Vector3::zero(); }
    [[nodiscard]] Vector3 startPoint() const noexcept { return value(range().first); }
    [[nodiscard]] Vector3 endPoint() const noexcept { return value(range().last); }

    [[nodiscard]] Vector3 tangent(Scalar u) const noexcept
    {
        if (!isBound()) return Vector3::zero();
        const Scalar h = Scalar(1.0e-6);
        const BRepRange r = range();
        const Scalar u0 = std::max(r.first, u - h);
        const Scalar u1 = std::min(r.last, u + h);
        const Scalar den = u1 - u0;
        return den > Scalar(0.0) ? (value(u1) - value(u0)) / den : Vector3::zero();
    }

    [[nodiscard]] Scalar lengthEstimate(std::size_t samples = 32) const noexcept
    {
        if (!isBound() || samples < 2) return Scalar(0.0);
        const BRepRange r = range();
        Scalar total = 0.0;
        Vector3 prev = value(r.first);
        for (std::size_t i = 1; i < samples; ++i)
        {
            const Scalar t = static_cast<Scalar>(i) / static_cast<Scalar>(samples - 1);
            const Vector3 curr = value(r.first + (r.last - r.first) * t);
            total += (curr - prev).length();
            prev = curr;
        }
        return total;
    }

private:
    const BRepModel* model_{nullptr};
    const BRepEdge* edge_{nullptr};
    const BRepCurveGeometry* curve_{nullptr};
};

class BRepAdaptor_Surface
{
public:
    BRepAdaptor_Surface() = default;
    BRepAdaptor_Surface(const BRepModel& model, BRepFaceHandle faceHandle) noexcept
    {
        (void)bind(model, faceHandle);
    }

    [[nodiscard]] bool bind(const BRepModel& model, BRepFaceHandle faceHandle) noexcept
    {
        model_ = &model;
        face_ = model.topology().face(faceHandle);
        surface_ = face_ ? model.geometry().surface(face_->surface) : nullptr;
        return surface_ != nullptr && surface_->isValid();
    }

    [[nodiscard]] bool isBound() const noexcept { return model_ && face_ && surface_; }
    [[nodiscard]] BRepSurfaceType type() const noexcept { return surface_ ? surface_->type : BRepSurfaceType::Unknown; }
    [[nodiscard]] Vector3 value(Scalar u, Scalar v) const noexcept { return isBound() ? surface_->valueAt(u, v) : Vector3::zero(); }

    [[nodiscard]] Vector3 normal(Scalar u, Scalar v) const noexcept
    {
        if (!isBound()) return Vector3::unitZ();
        if (surface_->type == BRepSurfaceType::Plane)
            return surface_->plane.normal.normalized();
        const Scalar h = Scalar(1.0e-6);
        const Vector3 pu = value(u + h, v) - value(u - h, v);
        const Vector3 pv = value(u, v + h) - value(u, v - h);
        const Vector3 n = Vector3::cross(pu, pv);
        return n.isZero() ? Vector3::unitZ() : n.normalized();
    }

private:
    const BRepModel* model_{nullptr};
    const BRepFace* face_{nullptr};
    const BRepSurfaceGeometry* surface_{nullptr};
};

} // namespace mir
