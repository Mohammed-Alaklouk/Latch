#include "Serializer.h"
#include <cstdio>

 
// For serialization we create a snapshot of the circuit's current state and use it as a reference for the json
nlohmann::json Serializer::serializeCircuit(const Circuit& circuit)
{
	nlohmann::json circuitJson = circuit.GetSnapshot();
	circuitJson["version"] = 1; 
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

		snapshot = circuitJson.get<Circuit::CircuitSnapshot>();
	}
	catch (const std::exception& e)
	{
		printf("Error: Invalid circuit JSON format. %s\n", e.what());
		return false;
	}

	circuit.RestoreSnapshot(snapshot);
	return true;
}
