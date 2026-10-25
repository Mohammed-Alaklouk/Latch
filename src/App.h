#pragma once
#include <memory>
#include <vector>
#include <nfd.hpp>

#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

#include "Sheet.h"
#include "Settings.h"
#include "Serializer.h"

class App
{
public:
	App();
	~App();
	void Run();
	
private:
	void HandleInput();
	void Update(float deltaTime);
	void UI();
	void Draw();
	void SaveCurrentSheet();
	void LoadSheetFromFile();

	void GetMouseInputs(MouseInputs& mouse_inputs);
	Sheet& GetCurrentSheet() { return *sheets[current_sheet_index]; }

	int window_width = 800;
	int window_height = 450;

	std::vector<std::unique_ptr<Sheet>> sheets;
	int current_sheet_index = 0;
	Clipboard clipboard;

	char new_sheet_title[64] = ""; 

	componentInfo::Type selected_gate_type = componentInfo::Type::AND;
	NFD::Guard nfd_guard;
};
