#pragma once

#include "../Profile/Profile.hpp"
#include "../Solid/FacetedSolid.hpp"
#include "../Tessellation/TriangleMesh.hpp"

#include <utility>

namespace mir
{

/// Unified geometric model object.
///
/// Profile3 is the construction source, Solid3 is the exact CAD boundary
/// representation when available, and TriangleMesh3 is the render/import
/// representation. Mesh-only objects are valid for imported assets such as STL.
class Model3
{
public:
    Model3() = default;

    explicit Model3(Profile3 profile)
        : profile_(std::move(profile))
    {
    }

    [[nodiscard]] const Profile3& profile() const noexcept { return profile_; }
    [[nodiscard]] Profile3& profile() noexcept { return profile_; }

    [[nodiscard]] const Solid3& solid() const noexcept { return solid_; }
    [[nodiscard]] Solid3& solid() noexcept { return solid_; }

    [[nodiscard]] const TriangleMesh3& mesh() const noexcept { return mesh_; }
    [[nodiscard]] TriangleMesh3& mesh() noexcept { return mesh_; }

    void setProfile(Profile3 profile) { profile_ = std::move(profile); }
    void setSolid(Solid3 solid) { solid_ = std::move(solid); }
    void setMesh(TriangleMesh3 mesh) {
        mesh_ = std::move(mesh);
        mesh_.markGeometryChanged();
    }

    [[nodiscard]] bool hasProfile() const noexcept { return profile_.isValid(); }
    [[nodiscard]] bool hasSolid() const noexcept { return solid_.isValid(); }
    [[nodiscard]] bool hasMesh() const noexcept { return mesh_.isValid(); }

    [[nodiscard]] bool isValid() const noexcept
    {
        return hasSolid() || hasMesh() || hasProfile();
    }

    [[nodiscard]] Point3 boundsMin() const noexcept
    {
        if (hasMesh()) return mesh_.boundsMin();
        if (hasSolid()) return solid_.boundsMin();
        if (hasProfile()) return profile_.boundsMin();
        return Point3::origin();
    }

    [[nodiscard]] Point3 boundsMax() const noexcept
    {
        if (hasMesh()) return mesh_.boundsMax();
        if (hasSolid()) return solid_.boundsMax();
        if (hasProfile()) return profile_.boundsMax();
        return Point3::origin();
    }

private:
    Profile3 profile_;
    Solid3 solid_;
    TriangleMesh3 mesh_;
};

/// Canonical document-level name. Geometry remains spatial; time belongs to Document.
using Model = Model3;

} // namespace mir
