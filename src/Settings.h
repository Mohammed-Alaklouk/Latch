#pragma once
#include "raylib.h"

struct SimulationSettings {
	float tickRate = 0.5f;
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
		Color AccentColor;
		Color BorderColor;
		Color BorderHighlightColor;
		Color LeadColor; // short stubs joining pins to the body
	};

	struct UITheme {
		Color Background;
		Color GridMinor;
		Color GridMajor;
		Color SelectionColor;
		Color LabelColor;
		Color SelectionRectangleFill;
		Color SelectionRectangleOutline;
		Color UndefinedHaloColor;
		Color LogicLevelColors[3];
		GateColors Gates;
	};

	inline constexpr UITheme DarkTheme { 
		.Background					= {18,18,18,255},			
		.GridMinor					= {30,30,30,255},			
		.GridMajor					= {50,50,50,255},			
		.SelectionColor				= {0, 150, 255, 255},		
		.LabelColor					= WHITE,
		.SelectionRectangleFill		= {255,0,0,50},
		.SelectionRectangleOutline	= RED,
		.UndefinedHaloColor			= {230, 40, 55, 100 },
		.LogicLevelColors = {
			{60, 60, 60, 255},		// LOW
			{0, 220, 80, 255},		// HIGH
			{255, 150, 0, 255}		// UNDEFINED
		},
		.Gates = {
			.BodyTop				= { 45, 50, 62, 255 },		
			.BodyBot				= { 32, 36, 46, 255 },		
			.AccentColor			= { 0, 180, 120, 255 },		
			.BorderColor			= { 80, 85, 95, 255 },		
			.BorderHighlightColor	= { 100, 160, 255, 255 },	
			.LeadColor				= { 90, 95, 110, 255 },		
		}
	};

	inline const UITheme& CurrentTheme() { return DarkTheme; }   
}