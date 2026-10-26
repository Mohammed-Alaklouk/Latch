#include <cstdio>
#include <nlohmann/json.hpp>
#include <fstream>
#include "Serializer.h"


bool Serializer::saveSheet(const std::filesystem::path& path, const Circuit& circuit, const SimulationSettings& settings)
{
	nlohmann::json j{
		{"version", 1},
		{"circuit", circuit.GetSnapshot()},
		{"settings", settings}
	};

	return writeJsonToFile(path, j);
}

// For deserialization we read the json and create a snapshot of the circuit's state and restore it to the circuit.
// If the deserialization was unsuccessful, it returnsf alse writes the issue to the console. 
// Later a logging system will be added and the function will return a log instead.
bool Serializer::loadSheet(const std::filesystem::path& path, Circuit& circuit, SimulationSettings& settings)
{
	nlohmann::json j;
	if (!readJsonFromFile(path, j)) {
		return false;
	}

	SheetData tempSheet;
	try {
		if (j.value("version", 0) != 1) {
			printf("Error: %s has an unsupported version\n", path.string().c_str());
			return false;
		}

		tempSheet = j.get<SheetData>();

		validateCircuitSnapshot(tempSheet.circuit);
	}
	catch (const std::exception& e) {
		printf("Error: Failed to load %s. %s\n", path.string().c_str(), e.what());
		return false;
	}

	circuit.RestoreSnapshot(tempSheet.circuit);
	settings = tempSheet.settings;
	return true;
}

void Serializer::to_json(nlohmann::json& j, const SheetData& sheet)
{
	j = nlohmann::json{
		{"version", 1},
		{"circuit", sheet.circuit},
		{"settings", sheet.settings}
	};
}

void Serializer::from_json(const nlohmann::json& j, SheetData& sheet)
{
	sheet.circuit = j.at("circuit").get<Circuit::CircuitSnapshot>();
	sheet.settings = j.at("settings").get<SimulationSettings>();
}

bool Serializer::writeJsonToFile(const std::filesystem::path& path, const nlohmann::json& j)
{
	std::filesystem::path tmp = path;
	tmp += ".tmp";

	std::ofstream out(tmp, std::ios::binary);
	if (!out) {
		printf("Error: Could not open %s for writing\n", tmp.string().c_str());
		return false;
	}
	out << j.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
	out.close();
	if (out.fail()) {
		printf("Error: Failed while writing %s\n", tmp.string().c_str());
		return false;
	}

	std::error_code ec;
	std::filesystem::rename(tmp, path, ec);
	if (ec) {
		printf("Error: Could not replace %s: %s\n", path.string().c_str(), ec.message().c_str());
		std::filesystem::remove(tmp, ec);
		return false;
	}
	return true;
}

bool Serializer::readJsonFromFile(const std::filesystem::path& path, nlohmann::json& j)
{
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		printf("Error: Could not open %s\n", path.string().c_str());
		return false;
	}
	try {
		j = nlohmann::json::parse(in);
	}
	catch (const nlohmann::json::parse_error& e) {
		printf("Error: %s is not valid JSON. %s\n", path.string().c_str(), e.what());
		return false;
	}
	return true;
}

void Serializer::validateCircuitSnapshot(const Circuit::CircuitSnapshot& snapshot)
{
	// validate IdManagers
	std::vector<int> componentIds;
	for (const auto& component : snapshot.components) {
		componentIds.push_back(component.id);
	}

	std::vector<int> wireIds;
	for (const auto& wire : snapshot.wires) {
		wireIds.push_back(wire.ID);
	}

	validateIdManager(snapshot.component_ids, componentIds, "Snapshot Component IDs");
	validateIdManager(snapshot.wire_ids, wireIds, "Snapshot Wire IDs");

	// validate the inside of each wire before anything else looks up its nodes
	for (const auto& wire : snapshot.wires) {
		std::vector<int> nodeIds;
		for (const auto& node : wire.Nodes) {
			nodeIds.push_back(node.ID);
		}

		validateIdManager(wire.node_id_manager, nodeIds, "Wire " + std::to_string(wire.ID) + " Node IDs");

		// Check if the source node exists and is a pin
		int sourceIndex = wire.node_id_manager.getIndex(wire.SourceNodeID);
		if (sourceIndex == -1 || wire.Nodes[sourceIndex].Type != WireNode::PIN) {
			throw std::runtime_error("Wire " + std::to_string(wire.ID) + " has no source pin");
		}

		// Check if the destination nodes exist and are pins
		for (const auto& destNodeID : wire.DestinationNodeIDs) {
			int destIndex = wire.node_id_manager.getIndex(destNodeID);
			if (destIndex == -1 || wire.Nodes[destIndex].Type != WireNode::PIN) {
				throw std::runtime_error("Wire " + std::to_string(wire.ID) + " has a destination node " + std::to_string(destNodeID) + " that is not a pin");
			}
		}

		// Check if the segments connect existing nodes
		for (const auto& segment : wire.Segments) {
			if (wire.node_id_manager.getIndex(segment.first) == -1 || wire.node_id_manager.getIndex(segment.second) == -1) {
				throw std::runtime_error("Wire " + std::to_string(wire.ID) + " has a segment with a non-existent node");
			}
		}
	}

	// validate components
	for (const auto& component : snapshot.components) {
		// Check if the component has the same number of pins as its gate type
		componentInfo expected = GateFactories[component.type](component.id, component.position)->getComponentInfo();
		if (component.input_wireIDs.size() != expected.input_wireIDs.size() ||
			component.output_wireIDs.size() != expected.output_wireIDs.size() ||
			component.output_value.size() != expected.output_value.size()) {
			throw std::runtime_error("Component " + std::to_string(component.id) + " has the wrong number of pins for its type");
		}

		int index = 0;
		for (const auto& input_wire_id : component.input_wireIDs) {
			if (input_wire_id == -1) {  // Skip if no wire is connected
				index++;
				continue;
			}

			if (snapshot.wire_ids.getIndex(input_wire_id) == -1) {
				throw std::runtime_error("Component " + std::to_string(component.id) + " has a non-existent input wire ID " + std::to_string(input_wire_id));
			}

			// Check if the input wire has a destination node on this component at the correct pin index
			// The same wire can feed more than one pin of the component, so only a node matching both counts
			bool found = false;
			const auto& wire = snapshot.wires.at(snapshot.wire_ids.getIndex(input_wire_id));
			for (const auto& destNodeID : wire.DestinationNodeIDs) {
				const auto& node = wire.Nodes.at(wire.node_id_manager.getIndex(destNodeID));
				if (node.Pin.ComponentID == component.id && node.Pin.PinIndex == index) {
					found = true;
					break;
				}
			}

			if (!found) {
				throw std::runtime_error("Wire " + std::to_string(input_wire_id) + " is not connected to input pin " + std::to_string(index) + " of component " + std::to_string(component.id));
			}

			index++;
		}

		index = 0;
		// Check output wires
		for (const auto& output_wire_id : component.output_wireIDs) {
			if (output_wire_id == -1) {  // Skip if no wire is connected
				index++;
				continue;
			}

			if (snapshot.wire_ids.getIndex(output_wire_id) == -1) {
				throw std::runtime_error("Component " + std::to_string(component.id) + " has a non-existent output wire ID " + std::to_string(output_wire_id));
			}

			auto sourcePin = snapshot.wires.at(snapshot.wire_ids.getIndex(output_wire_id)).getSourcePin();
			// Check if the output wire is connected to the correct component
			if (sourcePin.ComponentID != component.id) {
				throw std::runtime_error("Wire " + std::to_string(output_wire_id) + " is not connected to the output of component " + std::to_string(component.id));
			}

			// Check if the output wire is connected to the correct pin index
			if (sourcePin.PinIndex != index) {
				throw std::runtime_error("Wire " + std::to_string(output_wire_id) + " is not connected to the correct output pin of component " + std::to_string(component.id));
			}

			index++;
		}
	}

	// validate wires
	for (const auto& wire : snapshot.wires) {
		// Check if the source pin's component ID exists in the snapshot
		for (const auto& node : wire.Nodes) {
			if (node.Pin.ComponentID != -1 && snapshot.component_ids.getIndex(node.Pin.ComponentID) == -1) {
				throw std::runtime_error("Wire " + std::to_string(wire.ID) + " has a node with a non-existent component ID " + std::to_string(node.Pin.ComponentID));
			}
		}

		// Check if each destination node's component has this wire on the same input pin
		for (const auto& destNodeID : wire.DestinationNodeIDs) {
			PinRef pin = wire.Nodes.at(wire.node_id_manager.getIndex(destNodeID)).Pin;
			auto componentIndex = snapshot.component_ids.getIndex(pin.ComponentID);
			if (componentIndex == -1) {
				throw std::runtime_error("Wire " + std::to_string(wire.ID) + " has a destination pin with a non-existent component ID " + std::to_string(pin.ComponentID));
			}

			const auto& inputs = snapshot.components.at(componentIndex).input_wireIDs;
			if (pin.PinIndex < 0 || pin.PinIndex >= (int)inputs.size() || inputs[pin.PinIndex] != wire.ID) {
				throw std::runtime_error("Wire " + std::to_string(wire.ID) + " is not connected to input pin " + std::to_string(pin.PinIndex) + " of component " + std::to_string(pin.ComponentID));
			}
		}

		// Check if the source pin's component exists in the snapshot and has this wire on the same output pin
		PinRef sourcePin = wire.getSourcePin();
		auto sourceComponentIndex = snapshot.component_ids.getIndex(sourcePin.ComponentID);
		if (sourceComponentIndex == -1) {
			throw std::runtime_error("Wire " + std::to_string(wire.ID) + " has a source pin with a non-existent component ID " + std::to_string(sourcePin.ComponentID));
		}

		const auto& outputs = snapshot.components.at(sourceComponentIndex).output_wireIDs;
		if (sourcePin.PinIndex < 0 || sourcePin.PinIndex >= (int)outputs.size() || outputs[sourcePin.PinIndex] != wire.ID) {
			throw std::runtime_error("Wire " + std::to_string(wire.ID) + " is not connected to output pin " + std::to_string(sourcePin.PinIndex) + " of component " + std::to_string(sourcePin.ComponentID));
		}
	}
}

// objectIds holds the id of each object in the container, in storage order
// Every id has to be either used or free, and every used id has to point at the object with that id
void Serializer::validateIdManager(const IdManager& idManager, const std::vector<int>& objectIds, const std::string& name)
{
	const auto& indices = idManager.getIndices();

	std::unordered_set<int> freeIds;
	for (const auto& freeId : idManager.getFreeIds()) {
		if (freeId < 0 || freeId >= (int)indices.size()) {
			throw std::runtime_error("IdManager " + name + " has an out of range free id: " + std::to_string(freeId));
		}

		if (!freeIds.insert(freeId).second) {
			throw std::runtime_error("IdManager " + name + " has a duplicate free id: " + std::to_string(freeId));
		}
	}

	std::unordered_set<int> usedIndices;
	int i = 0;
	for (const auto& index : indices) {
		bool inUnused = freeIds.find(i) != freeIds.end();
		bool isUsed = index != -1;
		if (isUsed && inUnused) {
			throw std::runtime_error("IdManager " + name + " has a duplicate index: " + std::to_string(i));
		}

		if (!isUsed && !inUnused) {
			throw std::runtime_error("IdManager " + name + " has an index that is neither used nor unused: " + std::to_string(i));
		}

		if (isUsed) {
			if (index < 0 || index >= (int)objectIds.size()) {
				throw std::runtime_error("IdManager " + name + " has an invalid index: " + std::to_string(index));
			}

			// Check if the object at the index actually has this id
			if (objectIds[index] != i) {
				throw std::runtime_error("IdManager " + name + " maps id " + std::to_string(i) + " to an object with id " + std::to_string(objectIds[index]));
			}

			if (!usedIndices.insert(index).second) {
				throw std::runtime_error("IdManager " + name + " maps more than one id to index " + std::to_string(index));
			}
		}

		i++;
	}

	// Check if every object can be reached through an id
	if (usedIndices.size() != objectIds.size()) {
		throw std::runtime_error("IdManager " + name + " does not have an id for every object");
	}
}
