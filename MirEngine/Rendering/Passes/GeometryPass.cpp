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
    // Brighter model-only key light. Scene/background lighting is untouched.
    float keyIntensity = 1.35f;
    float fillDir[3] = {-0.6f, -0.2f, -0.8f};
    float fillColor[3] = {0.62f, 0.70f, 0.85f};
    // Lift side faces without flattening the model.
    float fillIntensity = 0.70f;
    float ambientColor[3] = {0.46f, 0.49f, 0.54f};
    // Keeps recessed engineering geometry readable.
    float ambientIntensity = 0.82f;
};

void normalize3(float v[3]) noexcept
{
    const float length=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    if(length>1e-9f){v[0]/=length;v[1]/=length;v[2]/=length;}
}

void applyMaterialUniforms(Shader* shader,const MaterialData& material)
{
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
    if(faceSelectionActive)drawHighlightFace(scene,context,device,shader.get()); shader->unbind();
}

void GeometryPass::rebuildSceneCache(mir::Scene& scene,RenderDevice& device)
{
    m_objectToHandle.clear();m_vaos.clear();m_nextMeshHandle=1;m_highlightVAO.reset();m_highlightObject=mir4d::InvalidObjectId;m_highlightFace=0;
    for(const auto& node:scene.nodes()){if(!node||!node->model()||!node->model()->hasMesh()||node->id()==mir4d::InvalidObjectId)continue;uploadMesh(*node,device);}
}

void GeometryPass::uploadMesh(const mir::ModelNode& node,RenderDevice& device)
{
    const mir::TriangleMesh3& mesh=node.model()->mesh(); if(!mesh.isValid())return;
    std::vector<Vertex> vertices; std::vector<std::uint32_t> indices; vertices.reserve(mesh.vertices.size()); indices.reserve(mesh.triangles.size()*3);
    std::vector<mir::Vector3> normals;
    if(mesh.hasVertexNormals())normals=mesh.normals;else{normals.assign(mesh.vertices.size(),mir::Vector3::zero());for(const auto& triangle:mesh.triangles){const mir::Vector3 a{mesh.vertices[triangle.a]};const mir::Vector3 b{mesh.vertices[triangle.b]};const mir::Vector3 c{mesh.vertices[triangle.c]};const auto n=mir::Vector3::cross(b-a,c-a).normalized();normals[triangle.a]+=n;normals[triangle.b]+=n;normals[triangle.c]+=n;}for(auto& n:normals)n=n.normalized();}
    for(std::size_t i=0;i<mesh.vertices.size();++i){const auto&p=mesh.vertices[i];const auto&n=normals[i];vertices.push_back(Vertex{{static_cast<float>(p.x),static_cast<float>(p.y),static_cast<float>(p.z)},{static_cast<float>(n.x),static_cast<float>(n.y),static_cast<float>(n.z)}});}
    for(const auto&t:mesh.triangles){indices.push_back(t.a);indices.push_back(t.b);indices.push_back(t.c);}
    const auto vao=std::make_shared<VertexArray>(); const auto vbo=std::make_shared<VertexBuffer>(vertices); const auto ibo=std::make_shared<IndexBuffer>(indices);
    vao->addVertexBuffer(vbo);vao->setIndexBuffer(ibo);m_vaos[node.id()]=vao;m_objectToHandle[node.id()]=m_nextMeshHandle++;
}

} // namespace MirEngine::Rendering
