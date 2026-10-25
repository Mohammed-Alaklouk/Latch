#pragma once
#include <nlohmann/json_fwd.hpp>
#include <filesystem>
#include "Circuit.h"
#include "Settings.h"

namespace Serializer {
	namespace {
	}

	bool saveSheet(const std::filesystem::path& path, const Circuit& circuit, const SimulationSettings& settings);
	bool loadSheet(const std::filesystem::path& path, Circuit& circuit, SimulationSettings& settings);

	struct SheetData {
		Circuit::CircuitSnapshot circuit;
		SimulationSettings settings;
	};
	
	void to_json(nlohmann::json& j, const SheetData& sheet);
	void from_json(const nlohmann::json& j, SheetData& sheet);
	
	bool writeJsonToFile(const std::filesystem::path& path, const nlohmann::json& j);
	bool readJsonFromFile(const std::filesystem::path& path, nlohmann::json& j);
};
