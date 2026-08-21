#include "GeometryPass.h"

#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Resources/Shader.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"

#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Model/ModelNode.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Math/Transform.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <vector>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine::Rendering
{

static const char* kDefaultVert = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
out vec3 v_normal;
out vec3 v_worldPos;
out vec3 v_viewPos;
void main() {
    vec4 worldPos = u_model * vec4(aPos, 1.0);
    v_worldPos = worldPos.xyz;
    v_normal = mat3(u_model) * aNormal;
    vec4 viewPos = u_view * worldPos;
    v_viewPos = viewPos.xyz;
    gl_Position = u_projection * viewPos;
}
)";

static const char* kDefaultFrag = R"(
#version 410 core
in vec3 v_normal;
in vec3 v_worldPos;
in vec3 v_viewPos;
out vec4 FragColor;
uniform vec3 u_keyDir;
uniform vec3 u_keyColor;
uniform float u_keyIntensity;
uniform vec3 u_fillDir;
uniform vec3 u_fillColor;
uniform float u_fillIntensity;
uniform vec3 u_ambientColor;
uniform float u_ambientIntensity;
uniform mat3 u_worldToView;
uniform vec3 u_baseColor;
uniform float u_roughness;
uniform float u_metallic;
uniform float u_specular;
uniform vec3 u_emission;
uniform float u_opacity;
uniform int u_selected;
uniform int u_hover;
const float PI = 3.14159265359;
float D_GGX(float NoH, float a) { float a2=a*a; float d=NoH*NoH*(a2-1.0)+1.0; return a2/(PI*d*d); }
float V_Smith(float NoV,float NoL,float a) { float a2=a*a; float gv=NoL*sqrt(NoV*NoV*(1.0-a2)+a2); float gl=NoV*sqrt(NoL*NoL*(1.0-a2)+a2); return 0.5/(gv+gl+1e-5); }
vec3 F_Schlick(float u,vec3 F0) { return F0+(1.0-F0)*pow(1.0-u,5.0); }
vec3 shadeDirectional(vec3 N,vec3 V,vec3 Ldir,vec3 lightColor,float lightIntensity,vec3 baseColor,float metallic,float roughness,vec3 F0) {
    vec3 L=normalize(u_worldToView*Ldir); float NoL=clamp(dot(N,L),0.0,1.0); if(NoL<=0.0)return vec3(0.0);
    vec3 H=normalize(L+V); float NoH=clamp(dot(N,H),0.0,1.0); float NoV=clamp(dot(N,V),0.0,1.0);
    float a=max(roughness*roughness,0.02); float D=D_GGX(NoH,a); float Vis=V_Smith(NoV,NoL,a); vec3 F=F_Schlick(clamp(dot(V,H),0.0,1.0),F0);
    vec3 spec=D*Vis*F; float diffuseStrength=mix(1.0,0.45,metallic); vec3 kd=(1.0-F)*diffuseStrength;
    return (kd*baseColor/PI+spec)*lightColor*lightIntensity*NoL;
}
void main() {
    vec3 N=normalize(u_worldToView*v_normal); vec3 V=normalize(-v_viewPos); float NoV=clamp(dot(N,V),0.0,1.0);
    vec3 F0=mix(vec3(0.04*u_specular),u_baseColor,u_metallic); vec3 Lo=vec3(0.0);
    Lo+=shadeDirectional(N,V,u_keyDir,u_keyColor,u_keyIntensity,u_baseColor,u_metallic,u_roughness,F0);
    Lo+=shadeDirectional(N,V,u_fillDir,u_fillColor,u_fillIntensity,u_baseColor,u_metallic,u_roughness,F0);
    float ambientFactor=0.40+0.60*NoV; vec3 ambient=u_ambientColor*u_ambientIntensity*ambientFactor*u_baseColor;
    float rim=pow(1.0-NoV,3.0); vec3 rimLight=u_ambientColor*0.35*rim*u_baseColor;
    vec3 color=ambient+rimLight+Lo+u_emission; color=max(color,vec3(0.0)); color=color/(color+vec3(1.0)); color=pow(color,vec3(1.0/2.2));
    if(u_selected!=0){ const vec3 selectionColor=vec3(1.0,0.62,0.16); color=mix(color,selectionColor,0.42); color=clamp(color,vec3(0.0),vec3(1.0)); }
    else if(u_hover!=0){ const vec3 hoverColor=vec3(0.35,0.85,1.0); color=mix(color,hoverColor,0.22); color=clamp(color,vec3(0.0),vec3(1.0)); }
    FragColor=vec4(color,u_opacity);
}
)";

namespace
{
struct EngineeringStudioLights
{
    float keyDir[3] = {0.5f, 1.0f, 0.3f};
    float keyColor[3] = {1.0f, 0.98f, 0.94f};
    float keyIntensity = 1.35f;
    float fillDir[3] = {-0.6f, -0.2f, -0.8f};
    float fillColor[3] = {0.62f, 0.70f, 0.85f};
    float fillIntensity = 0.70f;
    float ambientColor[3] = {0.46f, 0.49f, 0.54f};
    float ambientIntensity = 0.82f;
};

void normalize3(float v[3]) noexcept
{
    const float length=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(length>1e-9f){v[0]/=length;v[1]/=length;v[2]/=length;}
}

void applyMaterialUniforms(Shader* shader,const MaterialData& material)
{
    if (!shader) return;
    shader->setVec3("u_baseColor",material.baseColor[0],material.baseColor[1],material.baseColor[2]);
    shader->setFloat("u_roughness",material.roughness);
    shader->setFloat("u_metallic",material.metallic);
    shader->setFloat("u_specular",material.specular);
    shader->setVec3("u_emission",material.emission[0],material.emission[1],material.emission[2]);
    shader->setFloat("u_opacity",material.opacity);
}
}

GeometryPass::GeometryPass(ShaderLibrary& shaderLibrary):m_shaderLibrary(shaderLibrary){}

bool GeometryPass::initialize(RenderDevice& device)
{
    const ShaderHandle handle=m_shaderLibrary.load("DefaultLit",kDefaultVert,kDefaultFrag);
    if(handle.empty()){std::cerr<<"[GeometryPass] Failed to load default shader.\n";return false;}
    device.registerMaterial(kDefaultMaterialHandle,m_shaderLibrary.get(handle)); return true;
}

void GeometryPass::invalidateSceneCache() noexcept
{
    m_objectToHandle.clear(); m_vaos.clear(); m_nextMeshHandle=1; m_cachedScene=nullptr; m_cachedRevision=0;
    m_highlightVAO.reset(); m_highlightObject=mir4d::InvalidObjectId; m_highlightFace=0;
}

void GeometryPass::execute(RenderContext& context,mir::Scene& scene,RenderDevice& device)
{
    auto shader=m_shaderLibrary.get("DefaultLit"); if(!shader){std::cerr<<"[GeometryPass] DefaultLit shader not found.\n";return;}
    const std::uint64_t revision=scene.contentRevision();
    if(m_cachedRevision!=revision){rebuildSceneCache(scene,device);m_cachedScene=&scene;m_cachedRevision=revision;}
    Matrix4Raw viewRelative=context.viewMatrix; viewRelative[12]=0.0f; viewRelative[13]=0.0f; viewRelative[14]=0.0f;
    EngineeringStudioLights lights; normalize3(lights.keyDir); normalize3(lights.fillDir);
    const float worldToView[9]={viewRelative[0],viewRelative[1],viewRelative[2],viewRelative[4],viewRelative[5],viewRelative[6],viewRelative[8],viewRelative[9],viewRelative[10]};
    shader->bind(); shader->setMatrix("u_view",viewRelative); shader->setMatrix("u_projection",context.projectionMatrix);
    shader->setVec3("u_keyDir",lights.keyDir[0],lights.keyDir[1],lights.keyDir[2]); shader->setVec3("u_keyColor",lights.keyColor[0],lights.keyColor[1],lights.keyColor[2]); shader->setFloat("u_keyIntensity",lights.keyIntensity);
    shader->setVec3("u_fillDir",lights.fillDir[0],lights.fillDir[1],lights.fillDir[2]); shader->setVec3("u_fillColor",lights.fillColor[0],lights.fillColor[1],lights.fillColor[2]); shader->setFloat("u_fillIntensity",lights.fillIntensity);
    shader->setVec3("u_ambientColor",lights.ambientColor[0],lights.ambientColor[1],lights.ambientColor[2]); shader->setFloat("u_ambientIntensity",lights.ambientIntensity); shader->setMatrix3("u_worldToView",worldToView);
    const auto* selection=context.selectionIds; const bool faceSelectionActive=context.selectionObjectId!=0&&context.selectionFaceId!=0;
    for(const auto& node:scene.nodes()){
        if(!node||!node->model()||!node->model()->hasMesh()||node->id()==mir4d::InvalidObjectId)continue;
        bool selected=false;
        if(selection!=nullptr&&!(faceSelectionActive&&node->id()==context.selectionObjectId))for(const std::uint64_t id:*selection)if(id==node->id()){selected=true;break;}
        const bool hovered=!selected&&context.hoverObjectId==node->id(); processNode(*node,context,device,shader.get(),selected,hovered);
    }
    if(faceSelectionActive)drawHighlightFace(scene,context,device,shader.get());
    drawHighlightEdge(scene,context,device,shader.get());
    drawHighlightVertex(scene,context,device,shader.get());
    shader->unbind();
}

void GeometryPass::processNode(const mir::ModelNode& node,
                               RenderContext& context,
                               RenderDevice& device,
                               Shader* shader,
                               bool selected,
                               bool hovered)
{
    (void)device;
    if (!shader || !node.model() || !node.model()->hasMesh()) return;

    const auto handleIt = m_objectToHandle.find(node.id());
    if (handleIt == m_objectToHandle.end()) return;

    const auto vaoIt = m_vaos.find(handleIt->second);
    if (vaoIt == m_vaos.end() || !vaoIt->second || !vaoIt->second->isValid()) return;

    const double cameraPos[3] = {
        static_cast<double>(context.cameraPosition[0]),
        static_cast<double>(context.cameraPosition[1]),
        static_cast<double>(context.cameraPosition[2])
    };
    shader->setMatrix("u_model", makeModelMatrix(node.transform(), cameraPos));
    shader->setInt("u_selected", selected ? 1 : 0);
    shader->setInt("u_hover", hovered ? 1 : 0);

    MaterialId materialId = MaterialLibrary::defaultMaterial();
    if (const auto materialIt = m_objectMaterials.find(node.id()); materialIt != m_objectMaterials.end()) {
        materialId = materialIt->second;
    }
    applyMaterialUniforms(shader, MaterialLibrary::material(materialId));

    const auto& vao = vaoIt->second;
    vao->bind();
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(vao->getElementCount()),
                   GL_UNSIGNED_INT,
                   nullptr);
    vao->unbind();
}

void GeometryPass::rebuildSceneCache(mir::Scene& scene,RenderDevice& device)
{
    m_objectToHandle.clear();m_vaos.clear();m_nextMeshHandle=1;m_highlightVAO.reset();m_highlightObject=mir4d::InvalidObjectId;m_highlightFace=0;
    for(const auto& node:scene.nodes()){if(!node||!node->model()||!node->model()->hasMesh()||node->id()==mir4d::InvalidObjectId)continue;uploadMesh(*node,device);}
}

void GeometryPass::uploadMesh(const mir::ModelNode& node,RenderDevice& device)
{
    const mir::TriangleMesh3& mesh=node.model()->mesh(); if(!mesh.isValid())return;
    std::vector<Vertex> vertices; std::vector<std::uint32_t> indices;
    vertices.reserve(mesh.vertices.size()); indices.reserve(mesh.triangles.size()*3);
    std::vector<mir::Vector3> normals;
    if(mesh.hasVertexNormals()){
        normals=mesh.normals;
    } else {
        normals.assign(mesh.vertices.size(),mir::Vector3::zero());
        for(const auto& triangle:mesh.triangles){
            const auto ab=mesh.vertices[triangle.b]-mesh.vertices[triangle.a];
            const auto ac=mesh.vertices[triangle.c]-mesh.vertices[triangle.a];
            const auto n=mir::Vector3::cross(ab,ac).normalized();
            normals[triangle.a]+=n; normals[triangle.b]+=n; normals[triangle.c]+=n;
        }
        for(auto& nrm:normals)nrm=nrm.normalized();
    }
    for(std::size_t i=0;i<mesh.vertices.size();++i){
        const auto&p=mesh.vertices[i]; const auto&nrm=normals[i];
        vertices.push_back(Vertex(
            Vector3(static_cast<float>(p.x),static_cast<float>(p.y),static_cast<float>(p.z)),
            Vector3(static_cast<float>(nrm.x),static_cast<float>(nrm.y),static_cast<float>(nrm.z)),
            Vector2{}));
    }
    for(const auto&t:mesh.triangles){
        indices.push_back(static_cast<std::uint32_t>(t.a));
        indices.push_back(static_cast<std::uint32_t>(t.b));
        indices.push_back(static_cast<std::uint32_t>(t.c));
    }
    const auto vao=device.createVertexArray();
    const auto vbo=device.createVertexBuffer();
    const auto ibo=device.createIndexBuffer();
    vbo->uploadVertices(vertices);
    ibo->uploadIndices(indices);
    vao->setVertexBuffer(vbo);
    vao->setIndexBuffer(ibo);
    const MeshHandle handle=m_nextMeshHandle++;
    m_vaos[handle]=vao;
    m_objectToHandle[node.id()]=handle;
}

void GeometryPass::rebuildHighlightFace(mir::Scene& scene, RenderDevice& device)
{
    m_highlightVAO.reset();

    const auto objectId = m_highlightObject;
    const auto faceId = m_highlightFace;
    if (objectId == mir4d::InvalidObjectId || faceId == 0) return;

    const mir::ModelNode* selectedNode = nullptr;
    for (const auto& node : scene.nodes()) {
        if (node && node->id() == objectId && node->model() && node->model()->hasMesh()) {
            selectedNode = node.get();
            break;
        }
    }
    if (!selectedNode) return;

    const auto& mesh = selectedNode->model()->mesh();
    if (!mesh.isValid()) return;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    for (const auto& triangle : mesh.triangles) {
        if (triangle.sourceFaceId != faceId) continue;

        const std::size_t base = vertices.size();
        const std::size_t ids[3] = {triangle.a, triangle.b, triangle.c};

        mir::Vector3 normal = mir::Vector3::zero();
        const auto ab = mesh.vertices[triangle.b] - mesh.vertices[triangle.a];
        const auto ac = mesh.vertices[triangle.c] - mesh.vertices[triangle.a];
        normal = mir::Vector3::cross(ab, ac).normalized();

        for (const std::size_t index : ids) {
            const auto& p = mesh.vertices[index];
            vertices.emplace_back(
                Vector3(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z)),
                Vector3(static_cast<float>(normal.x), static_cast<float>(normal.y), static_cast<float>(normal.z)),
                Vector2{});
        }

        indices.push_back(static_cast<std::uint32_t>(base));
        indices.push_back(static_cast<std::uint32_t>(base + 1));
        indices.push_back(static_cast<std::uint32_t>(base + 2));
    }

    if (indices.empty()) return;

    const auto vao = device.createVertexArray();
    const auto vbo = device.createVertexBuffer();
    const auto ibo = device.createIndexBuffer();
    vbo->uploadVertices(vertices);
    ibo->uploadIndices(indices);
    vao->setVertexBuffer(vbo);
    vao->setIndexBuffer(ibo);
    m_highlightVAO = vao;
}

void GeometryPass::drawHighlightFace(mir::Scene& scene,
                                     RenderContext& context,
                                     RenderDevice& device,
                                     Shader* shader)
{
    if (!shader) return;

    if (m_highlightObject != context.selectionObjectId ||
        m_highlightFace != context.selectionFaceId ||
        !m_highlightVAO) {
        m_highlightObject = context.selectionObjectId;
        m_highlightFace = context.selectionFaceId;
        rebuildHighlightFace(scene, device);
    }

    if (!m_highlightVAO || !m_highlightVAO->isValid()) return;

    const mir::ModelNode* selectedNode = nullptr;
    for (const auto& node : scene.nodes()) {
        if (node && node->id() == context.selectionObjectId) {
            selectedNode = node.get();
            break;
        }
    }
    if (!selectedNode) return;

    const double cameraPos[3] = {
        static_cast<double>(context.cameraPosition[0]),
        static_cast<double>(context.cameraPosition[1]),
        static_cast<double>(context.cameraPosition[2])
    };
    shader->setMatrix("u_model", makeModelMatrix(selectedNode->transform(), cameraPos));
    shader->setInt("u_selected", 1);
    shader->setInt("u_hover", 0);
    shader->setVec3("u_baseColor", 1.0f, 0.62f, 0.16f);
    shader->setFloat("u_roughness", 0.42f);
    shader->setFloat("u_metallic", 0.0f);
    shader->setFloat("u_specular", 0.55f);
    shader->setVec3("u_emission", 0.08f, 0.03f, 0.0f);
    shader->setFloat("u_opacity", 0.92f);

    device.setDepthFunc(RenderDevice::DepthFunc::LessEqual);
    m_highlightVAO->bind();
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(m_highlightVAO->getElementCount()),
                   GL_UNSIGNED_INT,
                   nullptr);
    m_highlightVAO->unbind();
    device.setDepthFunc(RenderDevice::DepthFunc::Less);
}

void GeometryPass::drawHighlightEdge(mir::Scene& scene,
                                     RenderContext& context,
                                     RenderDevice& device,
                                     Shader* shader)
{
    if (!shader) return;

    auto drawOne = [&](std::uint64_t objectId, std::uint64_t edgeId, bool hover)
    {
        if (objectId == mir4d::InvalidObjectId) return;

        const mir::ModelNode* node = nullptr;
        for (const auto& n : scene.nodes()) {
            if (n && n->id() == objectId && n->model() && n->model()->hasMesh()) {
                node = n.get();
                break;
            }
        }
        if (!node) return;

        const auto& mesh = node->model()->mesh();
        if (!mesh.isValid()) return;

        const std::size_t ti = static_cast<std::size_t>(edgeId / 3);
        const int e = static_cast<int>(edgeId % 3);
        if (ti >= mesh.triangles.size()) return;
        const auto& tri = mesh.triangles[ti];
        const std::size_t ends[2] = {
            (e == 0) ? tri.a : (e == 1) ? tri.b : tri.c,
            (e == 0) ? tri.b : (e == 1) ? tri.c : tri.a};

        std::vector<Vertex> verts;
        for (const std::size_t idx : ends) {
            const auto& p = mesh.vertices[idx];
            verts.emplace_back(
                Vector3{static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z)},
                Vector3{}, Vector2{});
        }
        std::vector<std::uint32_t> indices = {0, 1};

        auto vao = device.createVertexArray();
        auto vbo = device.createVertexBuffer();
        auto ibo = device.createIndexBuffer();
        if (!vao || !vbo || !ibo) return;
        vbo->uploadVertices(verts);
        ibo->uploadIndices(indices);
        vao->setVertexBuffer(vbo);
        vao->setIndexBuffer(ibo);

        const double cameraPos[3] = {
            static_cast<double>(context.cameraPosition[0]),
            static_cast<double>(context.cameraPosition[1]),
            static_cast<double>(context.cameraPosition[2]) };
        shader->setMatrix("u_model", makeModelMatrix(node->transform(), cameraPos));
        shader->setInt("u_selected", 1);
        shader->setInt("u_hover", hover ? 1 : 0);
        if (hover) {
            shader->setVec3("u_baseColor", 0.5f, 0.9f, 1.0f);
            shader->setVec3("u_emission", 0.10f, 0.40f, 0.50f);
            shader->setFloat("u_opacity", 0.7f);
        } else {
            shader->setVec3("u_baseColor", 0.20f, 0.90f, 1.0f);
            shader->setVec3("u_emission", 0.15f, 0.50f, 0.55f);
            shader->setFloat("u_opacity", 1.0f);
        }
        shader->setFloat("u_roughness", 0.4f);
        shader->setFloat("u_metallic", 0.0f);
        shader->setFloat("u_specular", 0.5f);

        device.setDepthFunc(RenderDevice::DepthFunc::LessEqual);
        vao->bind();
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);
        vao->unbind();
        device.setDepthFunc(RenderDevice::DepthFunc::Less);
    };

    if (context.selectionEdgeActive)
        drawOne(context.selectionObjectId, context.selectionEdgeId, false);
    if (context.hoverEdgeActive && context.hoverEdgeId != context.selectionEdgeId)
        drawOne(context.hoverObjectId, context.hoverEdgeId, true);
}

void GeometryPass::drawHighlightVertex(mir::Scene& scene,
                                       RenderContext& context,
                                       RenderDevice& device,
                                       Shader* shader)
{
    if (!shader) return;

    auto drawOne = [&](std::uint64_t objectId, std::uint64_t vertexId, bool hover)
    {
        if (objectId == mir4d::InvalidObjectId) return;

        const mir::ModelNode* node = nullptr;
        for (const auto& n : scene.nodes()) {
            if (n && n->id() == objectId && n->model() && n->model()->hasMesh()) {
                node = n.get();
                break;
            }
        }
        if (!node) return;

        const auto& mesh = node->model()->mesh();
        if (!mesh.isValid()) return;
        if (vertexId >= mesh.vertices.size()) return;

        const auto& vref = mesh.vertices[vertexId];
        const mir::Point3 worldP = node->transform().transformPoint(vref);
        // Marker is built in camera-relative space so it stays screen-aligned
        // (does not rotate with the body) and uses an identity model matrix.
        const mir::Vector3 camRel{
            worldP.x - context.cameraPosition[0],
            worldP.y - context.cameraPosition[1],
            worldP.z - context.cameraPosition[2]};
        const float dist = static_cast<float>(camRel.length());

        float marker = 1e-4f;
        const float fy = context.projectionMatrix[5];
        if (context.viewportHeight > 0 && fy > 1e-5f)
        {
            // Perspective: derive a constant on-screen marker size
            // (~6px radius) from the vertical FOV and vertex distance.
            const float tanHalfFovY = 1.0f / fy;
            const float worldPerPixel =
                (2.0f * dist * tanHalfFovY) / static_cast<float>(context.viewportHeight);
            marker = std::max(worldPerPixel * 6.0f, 1e-4f);
        }
        else
        {
            // Non-perspective fallback: scale with the mesh extent.
            mir::Vector3 mn = mir::Vector3::zero();
            mir::Vector3 mx = mir::Vector3::zero();
            bool first = true;
            for (const auto& v : mesh.vertices) {
                const mir::Vector3 p{v.x, v.y, v.z};
                if (first) { mn = p; mx = p; first = false; }
                else {
                    mn.x = std::min(mn.x, p.x); mn.y = std::min(mn.y, p.y); mn.z = std::min(mn.z, p.z);
                    mx.x = std::max(mx.x, p.x); mx.y = std::max(mx.y, p.y); mx.z = std::max(mx.z, p.z);
                }
            }
            const float extent = static_cast<float>(
                mir::Vector3{mx.x - mn.x, mx.y - mn.y, mx.z - mn.z}.length());
            marker = std::max(extent * 0.04f, 1e-4f);
        }

        std::vector<Vertex> verts;
        const float cx = static_cast<float>(camRel.x);
        const float cy = static_cast<float>(camRel.y);
        const float cz = static_cast<float>(camRel.z);
        verts.emplace_back(Vector3{cx - marker, cy, cz}, Vector3{}, Vector2{});
        verts.emplace_back(Vector3{cx + marker, cy, cz}, Vector3{}, Vector2{});
        verts.emplace_back(Vector3{cx, cy - marker, cz}, Vector3{}, Vector2{});
        verts.emplace_back(Vector3{cx, cy + marker, cz}, Vector3{}, Vector2{});
        std::vector<std::uint32_t> indices = {0, 1, 2, 3};

        auto vao = device.createVertexArray();
        auto vbo = device.createVertexBuffer();
        auto ibo = device.createIndexBuffer();
        if (!vao || !vbo || !ibo) return;
        vbo->uploadVertices(verts);
        ibo->uploadIndices(indices);
        vao->setVertexBuffer(vbo);
        vao->setIndexBuffer(ibo);

        shader->setMatrix("u_model", IdentityMatrix4());
        shader->setInt("u_selected", 1);
        shader->setInt("u_hover", hover ? 1 : 0);
        if (hover) {
            shader->setVec3("u_baseColor", 1.0f, 0.9f, 0.4f);
            shader->setVec3("u_emission", 0.45f, 0.40f, 0.12f);
            shader->setFloat("u_opacity", 0.8f);
        } else {
            shader->setVec3("u_baseColor", 1.0f, 0.85f, 0.10f);
            shader->setVec3("u_emission", 0.55f, 0.45f, 0.05f);
            shader->setFloat("u_opacity", 1.0f);
        }
        shader->setFloat("u_roughness", 0.4f);
        shader->setFloat("u_metallic", 0.0f);
        shader->setFloat("u_specular", 0.5f);

        device.setDepthFunc(RenderDevice::DepthFunc::LessEqual);
        vao->bind();
        glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, nullptr);
        vao->unbind();
        device.setDepthFunc(RenderDevice::DepthFunc::Less);
    };

    if (context.selectionVertexActive)
        drawOne(context.selectionObjectId, context.selectionVertexId, false);
    if (context.hoverVertexActive && context.hoverVertexId != context.selectionVertexId)
        drawOne(context.hoverObjectId, context.hoverVertexId, true);
}

Matrix4Raw GeometryPass::makeModelMatrix(const mir::Transform& transform,
                                          const double cameraPos[3]) noexcept
{
    const mir::Matrix4 matrix = transform.matrix();
    Matrix4Raw result{};

    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            result[column * 4 + row] = static_cast<float>(matrix(row, column));
        }
    }

    result[12] = static_cast<float>(transform.position.x - cameraPos[0]);
    result[13] = static_cast<float>(transform.position.y - cameraPos[1]);
    result[14] = static_cast<float>(transform.position.z - cameraPos[2]);

    return result;
}

} // namespace MirEngine::Rendering
