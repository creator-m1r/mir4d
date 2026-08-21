#include "StepExporter.hpp"

#include "../../Document/Document.hpp"
#include "../../Geometry/Model/ModelNode.hpp"
#include "../../Geometry/Tessellation/TriangleMesh.hpp"
#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mir::io::step
{

namespace
{

struct TriangleData
{
    Point3 a;
    Point3 b;
    Point3 c;
};

Vector3 triangleNormal(const Point3& a, const Point3& b, const Point3& c)
{
    const Vector3 ab{b.x - a.x, b.y - a.y, b.z - a.z};
    const Vector3 ac{c.x - a.x, c.y - a.y, c.z - a.z};
    return Vector3::cross(ab, ac).normalized();
}

Vector3 arbitraryPerpendicular(const Vector3& n)
{
    const Vector3 candidate =
        std::abs(n.x) < 0.9 ? Vector3{1.0, 0.0, 0.0} : Vector3{0.0, 1.0, 0.0};
    return Vector3::cross(n, candidate).normalized();
}

[[nodiscard]] std::vector<TriangleData> collectTriangles(
    const mir4d::Document& document,
    const ExportOptions& options)
{
    std::vector<TriangleData> triangles;

    const auto include = [&](mir4d::ObjectId id)
    {
        if (!options.selectionOnly || options.selection.empty())
            return true;
        for (const mir4d::ObjectId selected : options.selection)
            if (selected == id)
                return true;
        return false;
    };

    for (const auto& node : document.scene().nodes())
    {
        if (!node || !include(node->id()) || !node->model() || !node->model()->hasMesh())
            continue;

        const TriangleMesh3& mesh = node->model()->mesh();
        const Transform& transform = node->transform();

        triangles.reserve(triangles.size() + mesh.triangles.size());
        for (const auto& triangle : mesh.triangles)
        {
            if (triangle.a >= mesh.vertices.size() ||
                triangle.b >= mesh.vertices.size() ||
                triangle.c >= mesh.vertices.size())
                continue;

            const Point3 a = transform.transformPoint(mesh.vertices[triangle.a]);
            const Point3 b = transform.transformPoint(mesh.vertices[triangle.b]);
            const Point3 c = transform.transformPoint(mesh.vertices[triangle.c]);
            triangles.push_back({a, b, c});
        }
    }

    return triangles;
}

std::string formatCoord(double value)
{
    std::ostringstream out;
    out << std::setprecision(12);
    if (value == 0.0)
        out << "0.";
    else
        out << value;
    return out.str();
}

const char* kStepHeader =
    "ISO-10303-21;\n"
    "HEADER;\n"
    "FILE_DESCRIPTION(('MIR4D STEP export'),'2;1');\n";

const char* kStepContext[] = {
    "#1 = APPLICATION_CONTEXT('automotive design');",
    "#2 = PRODUCT_CONTEXT('',#1,'mechanical');",
    "#3 = PRODUCT_DEFINITION_CONTEXT('',#2,'design');",
    "#4 = APPLICATION_PROTOCOL_DEFINITION('','automotive_design',2000,#1);",
    "#5 = PRODUCT('MIR4D_PART','MIR4D exported part','',(#2));",
    "#6 = PRODUCT_DEFINITION_FORMATION('','',#5);",
    "#7 = PRODUCT_DEFINITION('design',$,#6,#3);",
    "#8 = PRODUCT_DEFINITION_SHAPE('','',#7);",
    "#9 = ( LENGTH_UNIT ( ) NAMED_UNIT ( * ) SI_UNIT ( $.MILLI.,.METRE. ) ) ;",
    "#10 = ( NAMED_UNIT ( * ) PLANE_ANGLE_UNIT ( ) SI_UNIT ( $,.RADIAN. ) ) ;",
    "#11 = ( CURVATURE_MEASURE_UNIT ( ) NAMED_UNIT ( * ) SI_UNIT ( $,.STERADIAN. ) ) ;",
    "#12 = ( AREA_UNIT ( ) NAMED_UNIT ( * ) SI_UNIT ( $,.SQUARE_METRE. ) ) ;",
    "#13 = ( VOLUME_UNIT ( ) NAMED_UNIT ( * ) SI_UNIT ( $,.CUBIC_METRE. ) ) ;",
    "#14 = ( SI_UNIT ( $,.METRE. ) LENGTH_UNIT ( ) NAMED_UNIT ( * ) ) ;",
    "#15 = ( NAMED_UNIT ( * ) SI_UNIT ( $,.RADIAN. ) PLANE_ANGLE_UNIT ( ) ) ;",
    "#16 = ( AREA_UNIT ( ) NAMED_UNIT ( * ) SI_UNIT ( $,.SQUARE_METRE. ) ) ;",
    "#17 = ( NAMED_UNIT ( * ) SI_UNIT ( $,.CUBIC_METRE. ) VOLUME_UNIT ( ) ) ;",
    "#18 = ( GEOMETRIC_REPRESENTATION_CONTEXT ( 3 ) GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT ( ( #19 ) ) GLOBAL_UNIT_ASSIGNED_CONTEXT ( ( #14,#15,#16,#17 ) ) REPRESENTATION_CONTEXT ( 'NONE','3D' ) ) ;",
    "#19 = UNCERTAINTY_MEASURE_WITH_UNIT ( LENGTH_MEASURE ( 1.0E-06 ) ,  #14,  'distance_accuracy_value','' ) ;"};

}

ExportResult StepExporter::exportTo(
    const std::string& path,
    const mir4d::Document& document,
    const ExportOptions& options)
{
    ExportResult result;
    result.format = Format::Step;
    result.targetPath = path;

    const std::vector<TriangleData> triangles = collectTriangles(document, options);
    result.triangleCount = triangles.size();

    if (triangles.empty())
    {
        result.error = options.selectionOnly
            ? "No selected mesh triangles to export to STEP"
            : "Document contains no mesh triangles to export to STEP";
        return result;
    }

    std::ofstream output(path);
    if (!output)
    {
        result.error = "Cannot write STEP file: " + path;
        return result;
    }

    int nextId = 20;
    auto allocate = [&]() -> int { return nextId++; };

    std::vector<std::string> geometry;
    std::unordered_map<std::uint64_t, int> pointIds;

    auto cartesianPointId = [&](const Point3& p) -> int
    {
        std::uint64_t bx, by, bz;
        std::memcpy(&bx, &p.x, sizeof(bx));
        std::memcpy(&by, &p.y, sizeof(by));
        std::memcpy(&bz, &p.z, sizeof(bz));
        const std::uint64_t key = bx ^ (by << 1) ^ (bz << 2);
        auto found = pointIds.find(key);
        if (found != pointIds.end())
            return found->second;
        const int id = allocate();
        std::ostringstream line;
        line << "#" << id << " = CARTESIAN_POINT('',("
             << formatCoord(p.x * options.unitScale) << ','
             << formatCoord(p.y * options.unitScale) << ','
             << formatCoord(p.z * options.unitScale) << "));";
        geometry.push_back(line.str());
        pointIds.emplace(key, id);
        return id;
    };

    std::vector<int> faces;
    faces.reserve(triangles.size());

    for (const TriangleData& t : triangles)
    {
        const int pa = cartesianPointId(t.a);
        const int pb = cartesianPointId(t.b);
        const int pc = cartesianPointId(t.c);

        const int loopId = allocate();
        {
            std::ostringstream line;
            line << "#" << loopId << " = POLY_LOOP('',(#" << pa << ",#" << pb << ",#" << pc << "));";
            geometry.push_back(line.str());
        }

        const int fbId = allocate();
        {
            std::ostringstream line;
            line << "#" << fbId << " = FACE_BOUND(#" << loopId << ",.T.);";
            geometry.push_back(line.str());
        }

        const Point3 centroid{
            (t.a.x + t.b.x + t.c.x) / 3.0,
            (t.a.y + t.b.y + t.c.y) / 3.0,
            (t.a.z + t.b.z + t.c.z) / 3.0};

        Vector3 n = triangleNormal(t.a, t.b, t.c);
        if (n.length() < 1e-12)
            n = Vector3{0.0, 0.0, 1.0};
        const Vector3 u = arbitraryPerpendicular(n);

        const int cId = allocate();
        {
            std::ostringstream line;
            line << "#" << cId << " = CARTESIAN_POINT('',("
                 << formatCoord(centroid.x * options.unitScale) << ','
                 << formatCoord(centroid.y * options.unitScale) << ','
                 << formatCoord(centroid.z * options.unitScale) << "));";
            geometry.push_back(line.str());
        }
        const int dzId = allocate();
        {
            std::ostringstream line;
            line << "#" << dzId << " = DIRECTION('',("
                 << formatCoord(n.x) << ',' << formatCoord(n.y) << ',' << formatCoord(n.z) << "));";
            geometry.push_back(line.str());
        }
        const int dxId = allocate();
        {
            std::ostringstream line;
            line << "#" << dxId << " = DIRECTION('',("
                 << formatCoord(u.x) << ',' << formatCoord(u.y) << ',' << formatCoord(u.z) << "));";
            geometry.push_back(line.str());
        }
        const int axId = allocate();
        {
            std::ostringstream line;
            line << "#" << axId << " = AXIS2_PLACEMENT_3D('',#" << cId << ",#" << dzId << ",#" << dxId << ");";
            geometry.push_back(line.str());
        }
        const int plId = allocate();
        {
            std::ostringstream line;
            line << "#" << plId << " = PLANE('',#" << axId << ");";
            geometry.push_back(line.str());
        }
        const int fId = allocate();
        {
            std::ostringstream line;
            line << "#" << fId << " = FACE((#" << fbId << "),#" << plId << ");";
            geometry.push_back(line.str());
        }
        faces.push_back(fId);
    }

    const int shellId = allocate();
    {
        std::ostringstream line;
        line << "#" << shellId << " = CLOSED_SHELL('',(";
        for (std::size_t i = 0; i < faces.size(); ++i)
        {
            if (i != 0)
                line << ',';
            line << '#' << faces[i];
        }
        line << "));";
        geometry.push_back(line.str());
    }

    const int brepId = allocate();
    {
        std::ostringstream line;
        line << "#" << brepId << " = FACETED_BREP('',#" << shellId << ");";
        geometry.push_back(line.str());
    }

    const int repId = allocate();
    {
        std::ostringstream line;
        line << "#" << repId << " = ( REPRESENTATION ( 'MIR4D', ( #" << brepId
             << " ), #18 ) SHAPE_REPRESENTATION ( ) ) ;";
        geometry.push_back(line.str());
    }

    const int sdrId = allocate();
    {
        std::ostringstream line;
        line << "#" << sdrId << " = SHAPE_DEFINITION_REPRESENTATION ( #8, #" << repId << " ) ;";
        geometry.push_back(line.str());
    }

    output << kStepHeader;
    output << "FILE_NAME('MIR4D','',('MIR4D'),(''),'','','');\n";
    output << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN { 1 0 10303 214 1 1 1 1 }'));\n";
    output << "ENDSEC;\n";
    output << "DATA;\n";

    for (const char* line : kStepContext)
        output << line << '\n';

    for (const std::string& line : geometry)
        output << line << '\n';

    output << "ENDSEC;\n";
    output << "END-ISO-10303-21;\n";

    if (!output)
        result.error = "Failed to write STEP file: " + path;

    return result;
}

}
