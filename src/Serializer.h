#pragma once
#include "Circuit.h"
#include <nlohmann/json.hpp>

struct Serializer {
	static nlohmann::json serializeCircuit(const Circuit& circuit);
};
