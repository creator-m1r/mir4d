#pragma once

#include "ProcessMaterial.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mir
{

using ProcessNodeId = std::uint64_t;
using ProcessConnectionId = std::uint64_t;

struct ProcessPort
{
    std::string id;
    std::string name;
    bool input{true};
    std::string materialId;
};

struct ProcessNode
{
    ProcessNodeId id{0};
    WorldObject::Id equipmentId{0};
    std::string name;
    std::vector<ProcessPort> ports;
    bool enabled{true};
};

struct ProcessConnection
{
    ProcessConnectionId id{0};
    ProcessNodeId sourceNode{0};
    std::string sourcePort;
    ProcessNodeId targetNode{0};
    std::string targetPort;
    bool enabled{true};
    ProcessMaterial material{};
};

class SimulationProcessGraph
{
public:
    ProcessNodeId addNode(WorldObject::Id equipmentId, std::string name)
    {
        const ProcessNodeId id = nextNodeId_++;
        nodes_.emplace(id, ProcessNode{id, equipmentId, std::move(name), {}, true});
        return id;
    }

    bool removeNode(ProcessNodeId id)
    {
        if (nodes_.erase(id) == 0)
            return false;

        for (auto it = connections_.begin(); it != connections_.end(); )
        {
            if (it->second.sourceNode == id || it->second.targetNode == id)
                it = connections_.erase(it);
            else
                ++it;
        }
        return true;
    }

    bool addPort(ProcessNodeId nodeId, std::string id, std::string name, bool input)
    {
        auto* node = nodeFor(nodeId);
        if (!node)
            return false;

        node->ports.push_back(ProcessPort{std::move(id), std::move(name), input, {}});
        return true;
    }

    ProcessConnectionId connect(
        ProcessNodeId sourceNode,
        const std::string& sourcePort,
        ProcessNodeId targetNode,
        const std::string& targetPort)
    {
        if (!validPort(sourceNode, sourcePort, false) ||
            !validPort(targetNode, targetPort, true))
            return 0;

        const ProcessConnectionId id = nextConnectionId_++;
        connections_.emplace(id, ProcessConnection{
            id, sourceNode, sourcePort, targetNode, targetPort, true, {}});
        return id;
    }

    ProcessNode* node(ProcessNodeId id) noexcept { return nodeFor(id); }
    const ProcessNode* node(ProcessNodeId id) const noexcept { return nodeFor(id); }

    ProcessConnection* connection(ProcessConnectionId id) noexcept
    {
        const auto it = connections_.find(id);
        return it == connections_.end() ? nullptr : &it->second;
    }

    const ProcessConnection* connection(ProcessConnectionId id) const noexcept
    {
        const auto it = connections_.find(id);
        return it == connections_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const auto& nodes() const noexcept { return nodes_; }
    [[nodiscard]] const auto& connections() const noexcept { return connections_; }

private:
    ProcessNode* nodeFor(ProcessNodeId id) noexcept
    {
        const auto it = nodes_.find(id);
        return it == nodes_.end() ? nullptr : &it->second;
    }

    const ProcessNode* nodeFor(ProcessNodeId id) const noexcept
    {
        const auto it = nodes_.find(id);
        return it == nodes_.end() ? nullptr : &it->second;
    }

    bool validPort(ProcessNodeId nodeId, const std::string& portId, bool input) const
    {
        const auto* node = nodeFor(nodeId);
        if (!node)
            return false;

        for (const auto& port : node->ports)
        {
            if (port.id == portId && port.input == input)
                return true;
        }
        return false;
    }

    ProcessNodeId nextNodeId_{1};
    ProcessConnectionId nextConnectionId_{1};
    std::unordered_map<ProcessNodeId, ProcessNode> nodes_{};
    std::unordered_map<ProcessConnectionId, ProcessConnection> connections_{};
};

}
