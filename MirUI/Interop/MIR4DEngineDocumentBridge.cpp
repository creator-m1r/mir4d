#include "MIR4DEngineDocumentBridge.hpp"
#include "../../MirEngine/Document/CreateBoxCommandHandler.hpp"
#include "../../MirEngine/Document/Document.hpp"
#include <string>

namespace MirUI
{
struct MIR4DEngineDocumentBridge::Impl
{
    mir4d::Document document{"Новый проект"};
    mir4d::CreateBoxCommandHandler createBoxHandler{};
    std::string name{"Новый проект"};
};

MIR4DEngineDocumentBridge::MIR4DEngineDocumentBridge() : impl_(new Impl{}) {}
MIR4DEngineDocumentBridge::~MIR4DEngineDocumentBridge() { delete impl_; }

void MIR4DEngineDocumentBridge::reset(const char* projectName)
{
    const std::string name = (projectName != nullptr && *projectName != '\0') ? projectName : "Новый проект";
    // A Document owns its registry and ObjectStore, so reset both ownership
    // and history explicitly instead of replacing the non-assignable Document.
    impl_->document.scene().clear();
    impl_->document.objectRegistry().clear();
    impl_->document.history().clear();
    impl_->document.setTime(mir4d::Time{});
    impl_->document.setName(name);
    impl_->document.clearModified();
    impl_->name = name;
}

bool MIR4DEngineDocumentBridge::createBox(double width, double depth, double height, std::uint64_t* objectId)
{
    if (objectId != nullptr) *objectId = mir4d::InvalidObjectId;
    if (!(width > 0.0) || !(depth > 0.0) || !(height > 0.0)) return false;
    const auto command = mir4d::Command::make(0, impl_->document.time(), mir4d::CommandType::CreateBox,
                                               mir4d::InvalidObjectId,
                                               {std::to_string(width), std::to_string(depth), std::to_string(height)});
    const auto result = impl_->document.execute(command, impl_->createBoxHandler);
    if (!result.success) return false;
    if (objectId != nullptr) *objectId = result.objectId;
    return true;
}

bool MIR4DEngineDocumentBridge::advanceTime(double seconds)
{
    if (!(seconds >= 0.0)) return false;
    impl_->document.advanceTime(seconds);
    return true;
}
const char* MIR4DEngineDocumentBridge::projectName() const noexcept { return impl_->name.c_str(); }
std::size_t MIR4DEngineDocumentBridge::objectCount() const noexcept { return impl_->document.objects().size(); }
std::size_t MIR4DEngineDocumentBridge::commandCount() const noexcept { return impl_->document.history().size(); }
double MIR4DEngineDocumentBridge::currentTime() const noexcept { return impl_->document.time().seconds(); }
std::uint64_t MIR4DEngineDocumentBridge::revision() const noexcept { return impl_->document.revision(); }
bool MIR4DEngineDocumentBridge::isModified() const noexcept { return impl_->document.isModified(); }
bool MIR4DEngineDocumentBridge::isValid() const noexcept { return impl_->document.isValid(); }
std::size_t MIR4DEngineDocumentBridge::meshVertexCount(std::uint64_t objectId) const noexcept
{
    const auto node = impl_->document.objects().find(objectId);
    if (!node || !node->model() || !node->model()->hasMesh()) return 0;
    return node->model()->mesh().vertices.size();
}
std::size_t MIR4DEngineDocumentBridge::meshTriangleCount(std::uint64_t objectId) const noexcept
{
    const auto node = impl_->document.objects().find(objectId);
    if (!node || !node->model() || !node->model()->hasMesh()) return 0;
    return node->model()->mesh().triangles.size();
}
} // namespace MirUI

extern "C"
{
void* MIR4DDocumentCreate() { return new MirUI::MIR4DEngineDocumentBridge(); }
void MIR4DDocumentDestroy(void* handle) { delete static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle); }
void MIR4DDocumentReset(void* handle, const char* projectName) { if (handle) static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->reset(projectName); }
bool MIR4DDocumentCreateBox(void* handle, double w, double d, double h, std::uint64_t* id) { return handle && static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->createBox(w, d, h, id); }
bool MIR4DDocumentAdvanceTime(void* handle, double seconds) { return handle && static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->advanceTime(seconds); }
std::size_t MIR4DDocumentObjectCount(void* handle) { return handle ? static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->objectCount() : 0; }
std::size_t MIR4DDocumentCommandCount(void* handle) { return handle ? static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->commandCount() : 0; }
double MIR4DDocumentCurrentTime(void* handle) { return handle ? static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->currentTime() : 0.0; }
std::uint64_t MIR4DDocumentRevision(void* handle) { return handle ? static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->revision() : 0; }
bool MIR4DDocumentIsModified(void* handle) { return handle && static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->isModified(); }
bool MIR4DDocumentIsValid(void* handle) { return handle && static_cast<MirUI::MIR4DEngineDocumentBridge*>(handle)->isValid(); }
}
