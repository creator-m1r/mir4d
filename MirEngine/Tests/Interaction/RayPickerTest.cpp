#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Interaction/BoundingVolumeHierarchy.hpp"
#include "MirEngine/Viewport/Camera.hpp"
#include "MirEngine/Viewport/ViewportState.hpp"
#include "MirEngine/Math/Transform.hpp"
#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepPrimAPI_MakeBox.hpp"
#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"

#include <random>
#include <set>
#include <unordered_map>

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

namespace
{

mir::TriangleMesh3 makeBox()
{
    // Unit cube centered at the origin, spanning [-1, 1] on every axis.
    mir::TriangleMesh3 mesh;
    mesh.vertices = {
        {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0},
        {-1.0, 1.0, -1.0},  {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0},
        {1.0, 1.0, 1.0},    {-1.0, 1.0, 1.0}};
    mesh.triangles = {
        {0, 1, 2}, {0, 2, 3}, {4, 6, 5}, {4, 7, 6},
        {0, 4, 5}, {0, 5, 1}, {1, 5, 6}, {1, 6, 2},
        {2, 6, 7}, {2, 7, 3}, {3, 7, 4}, {3, 4, 0}};
    assert(mesh.isValid());
    return mesh;
}

std::shared_ptr<mir::Model> makeBoxModel()
{
    auto model = std::make_shared<mir::Model>();
    model->setMesh(makeBox());
    return model;
}

void runHierarchicalTests()
{
    // Single unit cube at the origin: deterministic geometry for sub-object
    // picking. Vertex indices follow makeBox()'s ordering.
    mir4d::Document doc("hierarchical");
    auto model = makeBoxModel();
    auto node = doc.scene().createNode(model);
    const auto id = node->id();

    const mir::Point3 v0{-1.0, -1.0, -1.0};                  // vertex index 0
    const mir::Point3 edgeMid{0.0, -1.0, -1.0};             // midpoint of edge (v0,v1): tri 0, edge 0
    const mir::Point3 faceMid{1.0 / 3.0, -1.0 / 3.0, -1.0}; // centroid of triangle 0

    auto rayThrough = [](const mir::Point3& p) {
        return mir::PickRay{mir::Point3{p.x, p.y, p.z + 0.05}, mir::Vector3{0.0, 0.0, -1.0}};
    };

    // Vertex mode (filter index 4) through vertex 0 -> Vertex, elementId 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(v0), mir::makePickFilter(4));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Vertex);
        assert(r.objectId == id);
        assert(r.elementId == 0);
    }

    // Edge mode (filter index 3) through edge midpoint -> Edge, elementId 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(edgeMid), mir::makePickFilter(3));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Edge);
        assert(r.elementId == 0);
    }

    // Face mode (filter index 2) through face centroid -> Face.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(faceMid), mir::makePickFilter(2));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Face);
        assert(r.objectId == id);
    }

    // Body mode (filter index 1) through centroid -> Body, elementId 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(faceMid), mir::makePickFilter(1));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Body);
        assert(r.elementId == 0);
    }

    // Vertex mode (filter 4) but cursor far from any vertex -> strict miss.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(faceMid), mir::makePickFilter(4));
        assert(!r.hit());
        assert(r.kind == mir::PickKind::None);
    }
}

} // namespace

namespace
{

void runBVHTests()
{
    std::mt19937 rng(12345);
    auto rnd = [&](double lo, double hi) {
        return lo + (hi - lo) * (static_cast<double>(rng()) /
                                 static_cast<double>(rng.max()));
    };

    constexpr std::size_t N = 800;
    std::vector<mir::Point3> points;
    points.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        points.push_back(mir::Point3{rnd(-50, 50), rnd(-50, 50), rnd(-50, 50)});

    mir::BoundingVolumeHierarchy bvh;
    bvh.buildPoints(points);

    auto safeInv = [](double d) {
        const double e = 1e-12;
        if (std::abs(d) < e)
            return d >= 0 ? std::numeric_limits<double>::max()
                          : -std::numeric_limits<double>::max();
        return 1.0 / d;
    };

    // Brute-force replica of the BVH's ray/AABB test.
    auto rayHitsPoint = [&](const mir::Point3& p, const mir::Point3& o,
                            const mir::Vector3& invDir, double r) {
        const double mn[3] = {p.x - r, p.y - r, p.z - r};
        const double mx[3] = {p.x + r, p.y + r, p.z + r};
        double t0 = -std::numeric_limits<double>::max();
        double t1 = std::numeric_limits<double>::max();
        const double org[3] = {o.x, o.y, o.z};
        const double iv[3] = {invDir.x, invDir.y, invDir.z};
        for (int i = 0; i < 3; ++i)
        {
            const double tN = (mn[i] - org[i]) * iv[i];
            const double tF = (mx[i] - org[i]) * iv[i];
            const double lo = std::min(tN, tF);
            const double hi = std::max(tN, tF);
            t0 = std::max(t0, lo);
            t1 = std::min(t1, hi);
            if (t0 > t1)
                return false;
        }
        return t1 >= std::max(0.0, t0);
    };

    for (int trial = 0; trial < 200; ++trial)
    {
        const mir::Point3 origin{rnd(-60, 60), rnd(-60, 60), rnd(-60, 60)};
        mir::Vector3 dir{rnd(-1, 1), rnd(-1, 1), rnd(-1, 1)};
        if (dir.length() < 1e-6)
            dir = mir::Vector3{0.0, 0.0, 1.0};
        dir = dir.normalized();
        const mir::Vector3 invDir{safeInv(dir.x), safeInv(dir.y), safeInv(dir.z)};
        const double radius = rnd(0.25, 6.0);

        std::set<std::size_t> brute;
        for (std::size_t i = 0; i < points.size(); ++i)
            if (rayHitsPoint(points[i], origin, invDir, radius))
                brute.insert(i);

        std::vector<std::size_t> q;
        bvh.queryRay(origin, dir, radius, q);
        std::set<std::size_t> got(q.begin(), q.end());

        assert(brute == got);
    }
}

void runDeformBVHTests()
{
    // Verifies the BVH cache is rebuilt when mesh geometry changes in place,
    // so picking always reflects the current vertices (not the stale tree).
    mir4d::Document doc("deform-bvh");
    auto model = makeBoxModel();
    auto node = doc.scene().createNode(model);
    const auto id = node->id();

    const mir::Point3 v0{-1.0, -1.0, -1.0};
    const mir::Point3 moved{10.0, 10.0, 10.0};

    auto rayThrough = [](const mir::Point3& p) {
        return mir::PickRay{mir::Point3{p.x, p.y, p.z + 0.05}, mir::Vector3{0.0, 0.0, -1.0}};
    };

    // Before deform: a vertex ray through v0 selects vertex 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(v0), mir::makePickFilter(4));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Vertex);
        assert(r.objectId == id);
        assert(r.elementId == 0);
    }

    // Mutate vertex 0 in place and signal the geometry change.
    node->model()->mesh().vertices[0] = moved;
    node->model()->mesh().markGeometryChanged();

    // After deform: the SAME ray must now MISS (original v0 is gone) and a ray
    // through the moved position must select vertex 0 again — proving the BVH
    // was rebuilt from the new geometry.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(v0), mir::makePickFilter(4));
        assert(!r.hit() || r.elementId != 0);
    }
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(moved), mir::makePickFilter(4));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Vertex);
        assert(r.objectId == id);
        assert(r.elementId == 0);
    }
}

void runBoxSelectionTests()
{
    mir4d::Document doc("box-selection");

    auto leftNode = doc.scene().createNode(makeBoxModel());
    mir::Transform leftT;
    leftT.position = {-18.0, 0.0, 0.0};
    leftNode->setTransform(leftT);

    auto rightNode = doc.scene().createNode(makeBoxModel());
    mir::Transform rightT;
    rightT.position = {18.0, 0.0, 0.0};
    rightNode->setTransform(rightT);

    mir::Camera camera;
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(0.0, 0.0, 20.0);
    camera.setProjection(mir::CameraProjection::Orthographic);
    camera.setAspect(800.0 / 600.0);

    const std::uint32_t width = 800;
    const std::uint32_t height = 600;

    mir::ViewportState state;
    state.camera = camera;
    state.resize(width, height);

    mir::Scene& scene = doc.scene();

    // Right-half rectangle intersects only the right node.
    state.selectInRect(scene, width * 0.55f, 0.0f, width * 0.95f,
                       static_cast<float>(height), false);
    assert(state.multiSelectionCount() == 1);
    assert(state.multiSelectionAt(0) == rightNode->id());
    assert(state.selection.primary() == rightNode->id());

    // Whole-viewport rectangle selects both cubes.
    state.selectInRect(scene, 0.0f, 0.0f, static_cast<float>(width),
                       static_cast<float>(height), false);
    assert(state.multiSelectionCount() == 2);

    // Additive selection over the right half unions with the existing set.
    state.selectInRect(scene, width * 0.55f, 0.0f, width * 0.95f,
                       static_cast<float>(height), true);
    assert(state.multiSelectionCount() == 2);

    // A rectangle in empty space clears the selection (non-additive).
    state.selectInRect(scene, 1.0f, 1.0f, 2.0f, 2.0f, false);
    assert(state.multiSelectionCount() == 0);
    assert(state.selection.primary() == mir4d::InvalidObjectId);
}

} // namespace

void runEdgeSourceIdTests()
{
    // Tessellating a real B-Rep solid must tag every boundary triangle edge
    // with the stable source B-Rep edge id, while internal tessellation seams
    // (ear-clip diagonals) remain kInvalidSourceEdge.
    mir::BRepModel model;
    const auto box = mir::BRepPrimAPI_MakeBox::build(model, 10.0, 10.0, 10.0, mir::Vector3::zero());
    assert(box.success);

    const mir::TriangleMesh3 mesh = mir::BRepTessellator::tessellateModel(model);
    assert(!mesh.empty());

    const std::uint64_t edgeCount = model.topology().edgeCount();
    std::uint64_t validEdges = 0;
    std::unordered_map<std::uint64_t, std::uint64_t> chordsPerEdge;
    for (const auto& tri : mesh.triangles)
    {
        for (int k = 0; k < 3; ++k)
        {
            const std::uint64_t sid = tri.sourceEdgeId[k];
            if (sid == mir::kInvalidSourceEdge)
                continue;
            ++validEdges;
            ++chordsPerEdge[sid];
            // Every valid id must index a real B-Rep edge.
            assert(sid < edgeCount);
        }
    }

    // The box has 12 edges; each appears as a boundary chord on two adjacent
    // faces, so at least 12 distinct boundary edges must be tagged.
    assert(validEdges >= 12);

    // Each B-Rep edge of the box is shared by exactly two faces, each face
    // contributing one boundary chord, so every valid source id must map to
    // exactly two triangle chords (this is what whole-edge highlight draws).
    assert(chordsPerEdge.size() == edgeCount);
    for (const auto& [sid, count] : chordsPerEdge)
    {
        assert(count == 2);
        (void)sid;
    }
}

void runElementBoxSelectionTests()
{
    // Rectangle selection must capture sub-objects (faces / edges) when the
    // active pick filter is an element mode, and keep whole-object selection
    // in body mode.
    mir::BRepModel brep;
    const auto box = mir::BRepPrimAPI_MakeBox::build(brep, 10.0, 10.0, 10.0, mir::Vector3::zero());
    assert(box.success);

    auto model = std::make_shared<mir::Model>();
    model->setMesh(mir::BRepTessellator::tessellateModel(brep));
    assert(!model->mesh().empty());

    mir4d::Document doc("element-box-selection");
    doc.scene().createNode(model);

    mir::Camera camera;
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(0.0, 0.0, 40.0);
    camera.setProjection(mir::CameraProjection::Orthographic);
    camera.setAspect(800.0 / 600.0);

    const std::uint32_t width = 800;
    const std::uint32_t height = 600;

    mir::ViewportState state;
    state.camera = camera;
    state.resize(width, height);
    mir::Scene& scene = doc.scene();

    const auto& mesh = model->mesh();
    std::unordered_set<std::uint64_t> faceIds;
    for (const auto& tri : mesh.triangles)
        faceIds.insert(tri.sourceFaceId);
    std::unordered_set<std::uint64_t> edgeIds;
    for (const auto& tri : mesh.triangles)
        for (int k = 0; k < 3; ++k)
            if (tri.sourceEdgeId[k] != mir::kInvalidSourceEdge)
                edgeIds.insert(tri.sourceEdgeId[k]);

    // ---- Face mode: every B-Rep face whose centroid projects in the rect ----
    state.setPickFilter(mir::makePickFilter(2));
    state.selectInRect(scene, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), false);
    assert(state.multiSelectionCount() == faceIds.size());
    for (std::size_t i = 0; i < state.multiSelectionCount(); ++i)
    {
        assert(state.multiSelectionKindAt(i) == mir::PickKind::Face);
        assert(faceIds.count(state.multiSelectionElementIdAt(i)) == 1);
    }

    // ---- Edge mode: every B-Rep edge whose chord midpoint projects in rect --
    state.setPickFilter(mir::makePickFilter(3));
    state.selectInRect(scene, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), false);
    assert(state.multiSelectionCount() == edgeIds.size());
    for (std::size_t i = 0; i < state.multiSelectionCount(); ++i)
    {
        assert(state.multiSelectionKindAt(i) == mir::PickKind::Edge);
        const std::uint64_t eid = state.multiSelectionElementIdAt(i);
        const std::size_t ti = static_cast<std::size_t>(eid / 3);
        const int k = static_cast<int>(eid % 3);
        assert(ti < mesh.triangles.size());
        assert(mesh.triangles[ti].sourceEdgeId[k] != mir::kInvalidSourceEdge);
    }

    // ---- Body mode: whole-object selection is unchanged ----
    state.setPickFilter(mir::makePickFilter(1));
    state.selectInRect(scene, 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), false);
    assert(state.multiSelectionCount() == 1);
    assert(state.multiSelectionKindAt(0) == mir::PickKind::Body);
}

void runBRepEdgeLengthTests()
{
    // The exact B-Rep edge length (via BRepAdaptor_Curve) must agree with the
    // tessellated chord length for a straight box edge, and the source edge id
    // stored on the mesh must resolve to a bound B-Rep edge.
    mir::BRepModel brep;
    const auto box = mir::BRepPrimAPI_MakeBox::build(brep, 4.0, 6.0, 8.0, mir::Vector3::zero());
    assert(box.success);

    const mir::TriangleMesh3 mesh = mir::BRepTessellator::tessellateModel(brep);
    assert(!mesh.empty());

    bool tested = false;
    for (std::size_t ti = 0; ti < mesh.triangles.size() && !tested; ++ti)
    {
        for (int k = 0; k < 3; ++k)
        {
            const std::uint64_t sid = mesh.triangles[ti].sourceEdgeId[k];
            if (sid == mir::kInvalidSourceEdge)
                continue;

            mir::BRepEdgeHandle eh;
            eh.index = sid;
            mir::BRepAdaptor_Curve curve(brep, eh);
            assert(curve.isBound());

            const double exact = curve.lengthEstimate();

            const auto& tri = mesh.triangles[ti];
            const std::size_t ends[2] = {
                (k == 0) ? tri.a : (k == 1) ? tri.b : tri.c,
                (k == 0) ? tri.b : (k == 1) ? tri.c : tri.a};
            const double chord =
                (mesh.vertices[ends[0]] - mesh.vertices[ends[1]]).length();

            // A straight box edge: exact arc length equals the chord length.
            assert(std::fabs(exact - chord) < 1e-6);
            tested = true;
            break;
        }
    }
    assert(tested);
}

void runCylinderTessellationTests()
{
    // Build a minimal lateral cylinder solid manually: one cylindrical face
    // bounded by two circular edges (bottom/top) and two seam edges.
    const double R = 2.0;
    const double H = 5.0;
    const double PI = 3.14159265358979323846;

    mir::BRepModel model;
    auto& geo = model.geometry();
    auto& topo = model.topology();

    mir::BRepSurfaceGeometry surf;
    surf.type = mir::BRepSurfaceType::Cylinder;
    surf.uRange = {0.0, 2.0 * PI};
    surf.vRange = {0.0, H};
    surf.cylinder.location = {0.0, 0.0, 0.0};
    surf.cylinder.axis = mir::Vector3::unitZ();
    surf.cylinder.xDir = mir::Vector3::unitX();
    surf.cylinder.radius = R;
    const mir::BRepSurfaceHandle sH = geo.addSurface(surf);

    auto makeCircle = [&](double z) {
        mir::BRepCurveGeometry c;
        c.type = mir::BRepCurveType::Circle;
        c.range = {0.0, 2.0 * PI};
        c.circle.center = {0.0, 0.0, z};
        c.circle.normal = mir::Vector3::unitZ();
        c.circle.xDir = mir::Vector3::unitX();
        c.circle.radius = R;
        return geo.addCurve(c);
    };
    const mir::BRepCurveHandle cBottomH = makeCircle(0.0);
    const mir::BRepCurveHandle cTopH = makeCircle(H);

    mir::BRepCurveGeometry cSeam;
    cSeam.type = mir::BRepCurveType::Line;
    cSeam.range = {0.0, H};
    cSeam.line.location = {R, 0.0, 0.0};
    cSeam.line.direction = mir::Vector3::unitZ();
    const mir::BRepCurveHandle cSeamAH = geo.addCurve(cSeam);
    const mir::BRepCurveHandle cSeamBH = geo.addCurve(cSeam);

    mir::BRepPointGeometry p0; p0.point = {R, 0.0, 0.0};
    mir::BRepPointGeometry p1; p1.point = {R, 0.0, H};
    const mir::BRepPointHandle p0H = geo.addPoint(p0);
    const mir::BRepPointHandle p1H = geo.addPoint(p1);

    mir::BRepVertex v0; v0.point = p0H; v0.free = false;
    mir::BRepVertex v1; v1.point = p1H; v1.free = false;
    const mir::BRepVertexHandle v0H = topo.addVertex(v0);
    const mir::BRepVertexHandle v1H = topo.addVertex(v1);

    auto makeEdge = [&](mir::BRepCurveHandle ch, mir::BRepVertexHandle s, mir::BRepVertexHandle e) {
        mir::BRepEdge edge;
        edge.curve = ch;
        edge.range = geo.curve(ch)->range;
        edge.start = s;
        edge.end = e;
        edge.free = false;
        return topo.addEdge(edge);
    };
    const mir::BRepEdgeHandle eBottomH = makeEdge(cBottomH, v0H, v0H);
    const mir::BRepEdgeHandle eTopH = makeEdge(cTopH, v1H, v1H);
    const mir::BRepEdgeHandle eSeamAH = makeEdge(cSeamAH, v0H, v1H);
    const mir::BRepEdgeHandle eSeamBH = makeEdge(cSeamBH, v0H, v1H);

    mir::BRepWire wire;
    auto addOE = [&](mir::BRepEdgeHandle eh, mir::BRepOrientation o) {
        mir::BRepOrientedEdge oe; oe.edge = eh; oe.orientation = o; wire.edges.push_back(oe);
    };
    addOE(eBottomH, mir::BRepOrientation::Forward);
    addOE(eSeamBH, mir::BRepOrientation::Forward);
    addOE(eTopH, mir::BRepOrientation::Reversed);
    addOE(eSeamAH, mir::BRepOrientation::Reversed);
    wire.closed = true;
    const mir::BRepWireHandle wireH = topo.addWire(wire);

    mir::BRepFace face;
    face.surface = sH;
    face.outer.wire = wireH;
    face.outer.orientation = mir::BRepOrientation::Forward;
    face.free = false;
    const mir::BRepFaceHandle faceH = topo.addFace(face);

    mir::BRepShell shell;
    mir::BRepOrientedFace of; of.face = faceH; of.orientation = mir::BRepOrientation::Forward;
    shell.faces.push_back(of);
    const mir::BRepShellHandle shellH = topo.addShell(shell);

    mir::BRepSolid solid;
    solid.shells.push_back(shellH);
    const mir::BRepSolidHandle solidH = topo.addSolid(solid);

    model.addRootSolid(solidH);

    // Tessellate and validate.
    const mir::TriangleMesh3 mesh = mir::BRepTessellator::tessellateModel(model);
    assert(!mesh.empty());
    assert(mesh.isValid());

    // Every vertex lies on the cylinder of radius R about the Z axis, so the
    // face is genuinely curved (not planar).
    for (const mir::Point3& p : mesh.vertices)
    {
        const double r = std::sqrt(p.x * p.x + p.y * p.y);
        assert(std::fabs(r - R) < 1e-6);
        assert(p.z >= -1e-6 && p.z <= H + 1e-6);
    }

    // Every triangle carries the single face id.
    for (const auto& t : mesh.triangles)
        assert(t.sourceFaceId == static_cast<std::uint64_t>(faceH.index));

    // Boundary chords must be tagged with real source edge ids, and each such
    // edge's exact length must match either the circumference (circle) or the
    // height (seam).
    std::set<std::uint64_t> tagged;
    for (const auto& t : mesh.triangles)
        for (int k = 0; k < 3; ++k)
            if (t.sourceEdgeId[k] != mir::kInvalidSourceEdge)
                tagged.insert(t.sourceEdgeId[k]);
    assert(tagged.size() >= 2);

    for (std::uint64_t sid : tagged)
    {
        mir::BRepEdgeHandle eh;
        eh.index = sid;
        assert(topo.edge(eh) != nullptr);
        mir::BRepAdaptor_Curve curve(model, eh);
        assert(curve.isBound());
        const double L = curve.lengthEstimate();
        const bool isCircle = std::fabs(L - 2.0 * PI * R) < 0.1;
        const bool isSeam = std::fabs(L - H) < 1e-3;
        assert(isCircle || isSeam);
    }
}

int main()
{
    mir4d::Document document("RayPicker Y-orientation test");

    // Upper cube sits at world +Y (renders at the top of the viewport).
    // It is placed near the top edge of the orthographic frustum so the
    // top-edge picking ray passes through its center.
    auto topModel = makeBoxModel();
    auto topNode = document.scene().createNode(topModel);
    mir::Transform topTransform;
    topTransform.position = {0.0, 18.0, 0.0};
    topNode->setTransform(topTransform);

    // Lower cube sits at world -Y (renders at the bottom of the viewport).
    auto bottomModel = makeBoxModel();
    auto bottomNode = document.scene().createNode(bottomModel);
    mir::Transform bottomTransform;
    bottomTransform.position = {0.0, -18.0, 0.0};
    bottomNode->setTransform(bottomTransform);

    // Orthographic camera looking straight down -Z with up = +Y, so world +Y
    // maps linearly to screen-up. With the Z-up convention phi is measured from
    // +Z, so a top-down view (eye on +Z, looking -Z) is phi = 0. theta = 0.
    // distance = 20 => ortho half-height = 20, so world Y at z=0 equals
    // ndcY * 20. A ray at ndcY = 0.9 (screenY = 0.95*H) hits world Y = 18.
    mir::Camera camera;
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(0.0, 0.0, 20.0);
    camera.setProjection(mir::CameraProjection::Orthographic);
    camera.setAspect(800.0 / 600.0);

    const std::uint32_t width = 800;
    const std::uint32_t height = 600;
    const float cx = width * 0.5f;

    // Top of the viewport (screenY measured from the bottom in the engine's
    // view-local convention) must resolve to the upper cube.
    const auto topHit =
        mir::RayPicker::pick(document.scene(), camera, cx, height * 0.95f, width, height);
    assert(topHit.hit());
    assert(topHit.objectId == topNode->id());

    // Bottom of the viewport must resolve to the lower cube.
    const auto bottomHit =
        mir::RayPicker::pick(document.scene(), camera, cx, height * 0.05f, width, height);
    assert(bottomHit.hit());
    assert(bottomHit.objectId == bottomNode->id());

    // The two hits must be different objects.
    assert(topHit.objectId != bottomHit.objectId);

    runHierarchicalTests();
    runBoxSelectionTests();
    runBVHTests();
    runDeformBVHTests();
    runEdgeSourceIdTests();
    runElementBoxSelectionTests();
    runBRepEdgeLengthTests();
    runCylinderTessellationTests();

    std::cout << "MIR4D RAYPICKER: OK\n";
    return 0;
}
