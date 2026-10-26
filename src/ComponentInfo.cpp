#include "ComponentInfo.h"
#include <nlohmann/json.hpp>

void to_json(nlohmann::json& j, const componentInfo& c)
{
	j = {
		{"type", static_cast<int>(c.type)},
		{"id", c.id},
		{"position", {c.position.x, c.position.y}},
		{"inputWireIds", c.input_wireIDs},
		{"outputWireIds", c.output_wireIDs},
		{"outputValues", c.output_value}
	};
}

void from_json(const nlohmann::json& j, componentInfo& c)
{
	int typeInt = j.at("type").get<int>();
	if (typeInt < 0 || typeInt >= static_cast<int>(componentInfo::COMPONENT_COUNT)) {
		throw std::runtime_error("Failed to load component: invalid type id: " + std::to_string(typeInt));
	}

	c.type = static_cast<componentInfo::Type>(typeInt);
	c.id = j.at("id").get<int>();
	c.position.x = j.at("position").at(0).get<float>();
	c.position.y = j.at("position").at(1).get<float>();
	c.input_wireIDs = j.at("inputWireIds").get<std::vector<int>>();
	c.output_wireIDs = j.at("outputWireIds").get<std::vector<int>>();
	// The values are read as ints and range checked before casting so a bad file can't produce an invalid LogicLevel
	c.output_value.clear();
	for (int valueInt : j.at("outputValues").get<std::vector<int>>()) {
		if (valueInt < 0 || valueInt > 2) {
			throw std::runtime_error("Failed to load component " + std::to_string(c.id) + ": invalid output value: " + std::to_string(valueInt));
		}

		c.output_value.push_back(static_cast<LogicLevel>(valueInt));
	}
}
