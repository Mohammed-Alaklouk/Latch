#include "Serializer.h"

nlohmann::json Serializer::serializeCircuit(const Circuit& circuit)
{
	auto snapshot = circuit.GetSnapshot();

	nlohmann::json circuitJson = {
		{"version", 1},
		{"components", nlohmann::json::array()},
		{"wires", nlohmann::json::array()},
		{
			"component-ids", {
				{"indices", snapshot.component_ids.getIndices()},
				{"free_ids", snapshot.component_ids.getFreeIds()}
			}
		},
		{
			"wire-ids", {
				{"indices", snapshot.wire_ids.getIndices()},
				{"free_ids", snapshot.wire_ids.getFreeIds()}
			}
		},
		{"note", snapshot.note}
	};

	for (const auto& component : snapshot.components) {
		nlohmann::json componentJson = {
			{"type", component.type},
			{"id", component.id},
			{"position", {component.position.x, component.position.y}},
			{"inputWireIds", component.input_wireIDs},
			{"outputWireIds", component.output_wireIDs},
			{"outputValues", component.output_value}
		};

		circuitJson["components"].push_back(componentJson);
	}

	for (const auto& wire : snapshot.wires) {
		nlohmann::json wireJson = {
			{"id", wire.ID},
			{"value", static_cast<int>(wire.Value)},

			{
				"nodeIDManager", {
					{"indices", wire.node_id_manager.getIndices()},
					{"free_ids", wire.node_id_manager.getFreeIds()}
				}
			},

			{"sourceNodeID", wire.SourceNodeID},
			{"destinationNodeIDs", wire.DestinationNodeIDs},
			{"nodes", nlohmann::json::array()},
			{"segments", nlohmann::json::array()},
		};

		for (const auto& node : wire.Nodes) {
			wireJson["nodes"].push_back({
				{"pin", {node.Pin.ComponentID, node.Pin.PinIndex}},
				{"position", {node.Position.x, node.Position.y}},
				{"type", static_cast<int>(node.Type)},
				{"id", node.ID}
			});
		}

		for (const auto& segment : wire.Segments) {
			wireJson["segments"].push_back({segment.first, segment.second});
		}
		
		circuitJson["wires"].push_back(wireJson);
	}


	return circuitJson;
}
