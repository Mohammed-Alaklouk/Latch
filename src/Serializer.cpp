#include "Serializer.h"
#include <cstdio>

 
// For serialization we create a snapshot of the circuit's current state and use it as a referece for the json
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
		circuitJson["wires"].push_back(wire);
	}


	return circuitJson;
}

// For deserialization we read the json and create a snapshot of the circuit's state and restore it to the circuit.
// The serializer currently trusts the json to be valid and does not perform any validation on the data. 
// It returns true if the deserialization was successful, false otherwise and it writes the issue to the console. 
// Later a logging system will be added and the function will return a log instead.
bool Serializer::deserializeCircuit(const nlohmann::json& circuitJson, Circuit& circuit)
{
	Circuit::CircuitSnapshot snapshot;

	try
	{
		int version = circuitJson.value("version", 0);
		if (version != 1) {
			printf("Error: Unsupported circuit JSON version %d\n", version);
			return false;
		}

		snapshot.component_ids.setIndices(circuitJson.at("component-ids").at("indices").get<std::vector<int>>());
		snapshot.component_ids.setFreeIds(circuitJson.at("component-ids").at("free_ids").get<std::vector<int>>());
		snapshot.wire_ids.setIndices(circuitJson.at("wire-ids").at("indices").get<std::vector<int>>());
		snapshot.wire_ids.setFreeIds(circuitJson.at("wire-ids").at("free_ids").get<std::vector<int>>());
		snapshot.note = circuitJson.at("note").get<std::string>();

		for (const auto& component: circuitJson.at("components")) {
			snapshot.components.push_back({
				static_cast<componentInfo::Type>(component.at("type").get<int>()),
				component.at("id").get<int>(),
				{component.at("position").at(0).get<float>(), component.at("position").at(1).get<float>()},
				component.at("inputWireIds").get<std::vector<int>>(),
				component.at("outputWireIds").get<std::vector<int>>(),
				component.at("outputValues").get<std::vector<LogicLevel>>()
				});
		}

		for (const auto& wire: circuitJson.at("wires")) {
			snapshot.wires.push_back(wire.get<Wire>());
		}
	}
	catch (const std::exception& e)
	{
		printf("Error: Invalid circuit JSON format. %s\n", e.what());
		return false;
	}

	circuit.RestoreSnapshot(snapshot);
	return true;
}
/*

nlohmann::json Serializer::serializeSettings(const SimulationSettings& settings)
{
	nlohmann::json settingsJson = {
		{"ticksPerSecond", settings.ticks_per_second},
	};
	return settingsJson;
}

SimulationSettings Serializer::deserializeSettings(const nlohmann::json& settingsJson) {
	SimulationSettings settings;
	try {
		settings.ticks_per_second = settingsJson.at("ticksPerSecond").get<float>();
	}
	catch (const std::exception& e) {
		printf("Error: Invalid settings JSON format. %s\n", e.what());
	}

	return settings;
}

*/
