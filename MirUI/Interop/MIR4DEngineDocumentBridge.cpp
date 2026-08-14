#include "MIR4DEngineDocumentBridge.hpp"

#include "../../MirEngine/Document/CreateBoxCommandHandler.hpp"
#include "../../MirEngine/Document/Document.hpp"

#include <memory>
#include <string>
#include <utility>

namespace MirUI
{

struct MIR4DEngineDocumentBridge::Impl
{
    mir4d::Document document{"Новый проект"};
    mir4d::CreateBoxCommandHandler createBoxHandler{};
    std::string name{"Новый проект"};
};

MIR4DEngineDocumentBridge::MIR4DEngineDocumentBridge()
    : impl_(new Impl{})
{
}

MIR4DEngineDocumentBridge::~MIR4DEngineDocumentBridge()
{
    delete impl_;
}

void MIR4DEngineDocumentBridge::reset(const char* projectName)
{
    const std::string name =
        (projectName != nullptr && *projectName != '\0') ? projectName : "Новый проект";

    impl_->document = mir4d::Document(name);
    impl_->name = name;
}

bool MIR4DEngineDocumentBridge::createBox(
    double width,
    double depth,
    double height,
    std::uint64_t* objectId)
{
    if (objectId != nullptr)
        *objectId = mir4d::InvalidObjectId;

    if (!(width > 0.0) || !(depth > 0.0) || !(height > 0.0))
        return false;

    const mir4d::Command command = mir4d::Command::make(
        0,
        impl_->document.time(),
        mir4d::CommandType::CreateBox,
        mir4d::InvalidObjectId,
        {std::to_string(width), std::to_string(depth), std::to_string(height)});

    const mir4d::CommandResult result =
        impl_->document.execute(command, impl_->createBoxHandler);

    if (!result.success)
        return false;

    if (objectId != nullptr)
        *objectId = result.objectId;

    return true;
}

bool MIR4DEngineDocumentBridge::advanceTime(double seconds)
{
    if (!(seconds >= 0.0))
        return false;

    impl_->document.advanceTime(seconds);
    return true;
}

const char* MIR4DEngineDocumentBridge::projectName() const noexcept
{
    return impl_->name.c_str();
}

std::size_t MIR4DEngineDocumentBridge::objectCount() const noexcept
{
    return impl_->document.objects().size();
}

std::size_t MIR4DEngineDocumentBridge::commandCount() const noexcept
{
    return impl_->document.history().size();
}

double MIR4DEngineDocumentBridge::currentTime() const noexcept
{
    return impl_->document.time().seconds();
}

std::uint64_t MIR4DEngineDocumentBridge::revision() const noexcept
{
    return impl_->document.revision();
}

bool MIR4DEngineDocumentBridge::isModified() const noexcept
{
    return impl_->document.isModified();
}

bool MIR4DEngineDocumentBridge::isValid() const noexcept
{
    return impl_->document.isValid();
}

std::size_t MIR4DEngineDocumentBridge::meshVertexCount(std::uint64_t objectId) const noexcept
{
    const auto node = impl_->document.objects().find(objectId);
    if (!node || !node->model() || !node->model()->hasMesh())
        return 0;
    return node->model()->mesh().vertices.size();
}

std::size_t MIR4DEngineDocumentBridge::meshTriangleCount(std::uint64_t objectId) const noexcept
{
    const auto node = impl_->document.objects().find(objectId);
    if (!node || !node->model() || !node->model()->hasMesh())
        return 0;
    return node->model()->mesh().triangles.size();
}

} // namespace MirUI
