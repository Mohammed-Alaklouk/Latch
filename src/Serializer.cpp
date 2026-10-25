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
// The serializer currently trusts the json to be valid and does not perform any validation on the data. 
// It returns true if the deserialization was successful, false otherwise and it writes the issue to the console. 
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
	}
	catch (const std::exception&) {
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
