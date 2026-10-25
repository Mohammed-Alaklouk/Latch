#pragma once
#include "raylib.h"
#include <nlohmann/json_fwd.hpp>

struct SimulationSettings {
	float TickRate = 0.5f;
};

namespace Settings {
	namespace View {
		inline constexpr float MinZoom = 0.3f;
		inline constexpr float MaxZoom = 3.0f;
		inline constexpr float ZoomSensitivity = 0.1f;
	}

	namespace Grid {
		inline constexpr int CellSize = 10;  
		inline constexpr int   MajorStep = 10;
		inline constexpr float MinorThickness = 0.8f;
		inline constexpr float MajorThickness = 1.5f;
	}

	struct GateColors {
		Color BodyTop;
		Color BodyBot;
		Color Accent;
		Color Border;
		Color BorderHighlight;
		Color Lead; // short stubs joining pins to the body
	};

	struct UITheme {
		Color Background;
		Color GridMinor;
		Color GridMajor;
		Color Selection;
		Color Label;
		Color SelectionRectangleFill;
		Color SelectionRectangleOutline;
		Color UndefinedHalo;
		Color LogicLevel[3];
		GateColors Gates;
	};

	inline constexpr UITheme DarkTheme { 
		.Background					= {18,18,18,255},			
		.GridMinor					= {30,30,30,255},			
		.GridMajor					= {50,50,50,255},			
		.Selection				= {0, 150, 255, 255},		
		.Label					= WHITE,
		.SelectionRectangleFill		= {255,0,0,50},
		.SelectionRectangleOutline	= RED,
		.UndefinedHalo			= {230, 40, 55, 100 },
		.LogicLevel = {
			{60, 60, 60, 255},		// LOW
			{0, 220, 80, 255},		// HIGH
			{255, 150, 0, 255}		// UNDEFINED
		},
		.Gates = {
			.BodyTop				= { 45, 50, 62, 255 },		
			.BodyBot				= { 32, 36, 46, 255 },		
			.Accent			= { 0, 180, 120, 255 },		
			.Border			= { 80, 85, 95, 255 },		
			.BorderHighlight	= { 100, 160, 255, 255 },	
			.Lead				= { 90, 95, 110, 255 },		
		}
	};

	inline const UITheme& CurrentTheme() { return DarkTheme; } 

	void to_json(nlohmann::json& j, const GateColors& gates);
	void from_json(const nlohmann::json& j, GateColors& gates);
	void to_json(nlohmann::json& j, const UITheme& theme);
	void from_json(const nlohmann::json& j, UITheme& theme);
}

void to_json(nlohmann::json& j, const SimulationSettings& settings);
void from_json(const nlohmann::json& j, SimulationSettings& settings);
void to_json(nlohmann::json& j, const Color& color);
void from_json(const nlohmann::json& j, Color& color);
