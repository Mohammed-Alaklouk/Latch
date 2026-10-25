#pragma once
#include <vector>
#include "raylib.h"
#include "Pin.h"
#include <nlohmann/json_fwd.hpp>

struct componentInfo {
	enum Type
	{
		AND,
		NAND,
		OR,
		NOR,
		XOR,
		XNOR,
		NOT,
		LED,
		TOGGLE,

		COMPONENT_COUNT
	};

	// Helper functions to convert between componentInfo and JSON using nlohmann::json
	friend void to_json(nlohmann::json& j, const componentInfo& c);
	friend void from_json(const nlohmann::json& j, componentInfo& c);

	Type type;
	int id;
	Vector2 position;
	std::vector<int> input_wireIDs;
	std::vector<int> output_wireIDs;
	std::vector<LogicLevel> output_value;
};