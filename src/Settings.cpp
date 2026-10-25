#include "Settings.h"
#include <nlohmann/json.hpp>
#include <algorithm>

void to_json(nlohmann::json& j, const SimulationSettings& settings)
{
	j["TickRate"] = settings.TickRate;
}

void from_json(const nlohmann::json& j, SimulationSettings& settings)
{
	settings.TickRate = j.value("TickRate", settings.TickRate);
}

void to_json(nlohmann::json& j, const Color& color)
{
	j = nlohmann::json::array({ color.r, color.g, color.b, color.a });
}

void from_json(const nlohmann::json& j, Color& color)
{
	color.r = j.at(0).get<decltype(color.r)>();
	color.g = j.at(1).get<decltype(color.g)>();
	color.b = j.at(2).get<decltype(color.b)>();
	color.a = j.at(3).get<decltype(color.a)>();
}

void Settings::to_json(nlohmann::json& j, const GateColors& gates)
{
	j["BodyTop"] = gates.BodyTop;
	j["BodyBot"] = gates.BodyBot;
	j["Accent"] = gates.Accent;
	j["Border"] = gates.Border;
	j["BorderHighlight"] = gates.BorderHighlight;
	j["Lead"] = gates.Lead;
}

void Settings::from_json(const nlohmann::json& j, GateColors& gates)
{
	gates.BodyTop = j.value("BodyTop", DarkTheme.Gates.BodyTop);
	gates.BodyBot = j.value("BodyBot", DarkTheme.Gates.BodyBot);
	gates.Accent = j.value("Accent", DarkTheme.Gates.Accent);
	gates.Border = j.value("Border", DarkTheme.Gates.Border);
	gates.BorderHighlight = j.value("BorderHighlight", DarkTheme.Gates.BorderHighlight);
	gates.Lead = j.value("Lead", DarkTheme.Gates.Lead);
}

void Settings::to_json(nlohmann::json& j, const UITheme& theme)
{
	j["Background"] = theme.Background;
	j["GridMinor"] = theme.GridMinor;
	j["GridMajor"] = theme.GridMajor;
	j["Selection"] = theme.Selection;
	j["Label"] = theme.Label;
	j["SelectionRectangleFill"] = theme.SelectionRectangleFill;
	j["SelectionRectangleOutline"] = theme.SelectionRectangleOutline;
	j["UndefinedHalo"] = theme.UndefinedHalo;
	j["LogicLevel"] = theme.LogicLevel;
	j["Gates"] = theme.Gates;

}

void Settings::from_json(const nlohmann::json& j, UITheme& theme)
{
	theme.Background = j.value("Background", DarkTheme.Background);
	theme.GridMinor = j.value("GridMinor", DarkTheme.GridMinor);
	theme.GridMajor = j.value("GridMajor", DarkTheme.GridMajor);
	theme.Selection = j.value("Selection", DarkTheme.Selection);
	theme.Label = j.value("Label", DarkTheme.Label);
	theme.SelectionRectangleFill = j.value("SelectionRectangleFill", DarkTheme.SelectionRectangleFill);
	theme.SelectionRectangleOutline = j.value("SelectionRectangleOutline", DarkTheme.SelectionRectangleOutline);
	theme.UndefinedHalo = j.value("UndefinedHalo", DarkTheme.UndefinedHalo);
	theme.Gates = j.value("Gates", DarkTheme.Gates);
	if (j.contains("LogicLevel"))
		j.at("LogicLevel").get_to(theme.LogicLevel);
	else
		std::copy(std::begin(DarkTheme.LogicLevel), std::end(DarkTheme.LogicLevel), theme.LogicLevel);

}
