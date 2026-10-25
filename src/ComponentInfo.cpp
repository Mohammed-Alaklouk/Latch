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
	c.type = static_cast<componentInfo::Type>(j.at("type").get<int>());
	c.id = j.at("id").get<int>();
	c.position.x = j.at("position").at(0).get<float>();
	c.position.y = j.at("position").at(1).get<float>();
	c.input_wireIDs = j.at("inputWireIds").get<std::vector<int>>();
	c.output_wireIDs = j.at("outputWireIds").get<std::vector<int>>();
	c.output_value = j.at("outputValues").get<std::vector<LogicLevel>>();
}
