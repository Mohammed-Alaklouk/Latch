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

	struct UITheme {
		Color background;
		Color gridMinor;
		Color gridMajor;
		Color LogicLevelColors[3];
	};

	inline constexpr UITheme DarkTheme { 
		{18,18,18,255}, 
		{30,30,30,255}, 
		{50,50,50,255},
		{
			{60, 60, 60, 255}, // LOW
			{0, 220, 80, 255}, // HIGH
			{255, 150, 0, 255} // UNDEFINED
		}
	};

	inline const UITheme& CurrentTheme() { return DarkTheme; }   
}