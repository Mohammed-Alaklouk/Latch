#pragma once
#include <vector>
#include "raylib.h"
#include "Pin.h"

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

	Type type;
	int id;
	Vector2 position;
	std::vector<int> input_wireIDs;
	std::vector<int> output_wireIDs;
	std::vector<LogicLevel> output_value;
};