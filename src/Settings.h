#pragma once
#include "raylib.h"

struct SimulationSettings {
	float tickRate = 0.5f;
};

namespace Settings {
	namespace view {
		inline constexpr float MinZoom = 0.3f;
		inline constexpr float MaxZoom = 3.0f;
		inline constexpr float ZoomSensitivity = 0.1f;
	}

	namespace grid {
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
		Color background;
		Color gridMinor;
		Color gridMajor;
		Color selectionColor;
		Color logicLevelColors[3];
		GateColors Gates;
	};

	inline constexpr UITheme DarkTheme { 
		.background					= {18,18,18,255},			// background
		.gridMinor					= {30,30,30,255},			// gridMinor
		.gridMajor					= {50,50,50,255},			// gridMajor 
		.selectionColor				= {0, 150, 255, 255},		// selection color
		.logicLevelColors = {
			{60, 60, 60, 255},		// LOW
			{0, 220, 80, 255},		// HIGH
			{255, 150, 0, 255}		// UNDEFINED
		},
		.Gates = {
			.BodyTop				= { 45, 50, 62, 255 },		// body_top
			.BodyBot				= { 32, 36, 46, 255 },		// body_bot
			.AccentColor			= { 0, 180, 120, 255 },		// AccentColor
			.BorderColor			= { 80, 85, 95, 255 },		// BorderColor
			.BorderHighlightColor	= { 100, 160, 255, 255 },	// BorderHighlightColor
			.LeadColor				= { 90, 95, 110, 255 },		// LeadColor	
		}
	};

	inline const UITheme& CurrentTheme() { return DarkTheme; }   
}