#include "App.h"
#include "imgui_internal.h"

App::App()
{
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_HIDDEN | FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "Latch");
    MaximizeWindow();
    ClearWindowState(FLAG_WINDOW_HIDDEN);

	sheets.push_back(std::make_unique<Sheet>());
	GetCurrentSheet().SetViewport({ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() });
        
    SetTargetFPS(60);
    rlImGuiSetup(true);
}

App::~App()
{
    rlImGuiShutdown();
    CloseWindow();
}

void App::GetMouseInputs(MouseInputs& mouse_inputs)
{
    mouse_inputs.leftButtonDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    mouse_inputs.rightButtonDown = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    mouse_inputs.mousePositionScreen = GetMousePosition();
    mouse_inputs.mousePositionWorld = GetScreenToWorld2D(GetMousePosition(), GetCurrentSheet().GetCamera());
    mouse_inputs.mouseWheelDelta = GetMouseWheelMove();
}

inline void App::CreateSaveFolder()
{
	std::error_code ec;
    if (!std::filesystem::exists(saveFolder, ec)) {
        std::filesystem::create_directory(saveFolder, ec);
	}

    if (ec) {
        printf("Error creating save folder: %s\n", ec.message().c_str());
	}
}

void App::HandleInput()
{
    if (IsWindowResized()) {
        for (auto& sheet : sheets) {
		    sheet->SetViewport({ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() });
        }
    }

	Sheet& sheet = GetCurrentSheet();
	MouseInputs mouse_inputs;
	GetMouseInputs(mouse_inputs);
	sheet.SetInputs(mouse_inputs, IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT), selected_gate_type);
	sheet.HandleInput();

    if (ImGui::GetIO().WantCaptureKeyboard || !CheckCollisionPointRec(mouse_inputs.mousePositionScreen, sheet.GetViewport()))
        return;

    if (IsKeyPressed(KEY_SPACE))
        sheet.StepSimulation();

    if (IsKeyPressed(KEY_DELETE))
        sheet.DeleteSelected();

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
    {
        if (IsKeyPressed(KEY_C))        sheet.CopySelected(clipboard);
        else if (IsKeyPressed(KEY_V))   sheet.Paste(clipboard);

        if (IsKeyPressed(KEY_Z))        sheet.Undo();
        else if (IsKeyPressed(KEY_Y))   sheet.Redo();

		if (IsKeyPressed(KEY_S))        SaveCurrentSheet();
    }
}

void App::Update(float deltaTime)
{
	GetCurrentSheet().Update(deltaTime);
}

void App::UI()
{
    rlImGuiBegin();

	MenuBar();
	TabBar();

    ImGui::Begin("Components");

    if (ImGui::Button("AND"))
        selected_gate_type = componentInfo::Type::AND;
    if (ImGui::Button("NAND"))
        selected_gate_type = componentInfo::Type::NAND;
    if (ImGui::Button("OR"))
        selected_gate_type = componentInfo::Type::OR;
    if (ImGui::Button("NOR"))
        selected_gate_type = componentInfo::Type::NOR;
    if (ImGui::Button("XOR"))
        selected_gate_type = componentInfo::Type::XOR;
    if (ImGui::Button("XNOR"))
        selected_gate_type = componentInfo::Type::XNOR;
    if (ImGui::Button("NOT"))
        selected_gate_type = componentInfo::Type::NOT;
    if (ImGui::Button("LED"))
        selected_gate_type = componentInfo::Type::LED;
    if (ImGui::Button("TOGGLE"))
        selected_gate_type = componentInfo::Type::TOGGLE;
    ImGui::End();

    GetCurrentSheet().UI();
    rlImGuiEnd();
}

void App::Draw() 
{ 
    Sheet& sheet = GetCurrentSheet();
    BeginDrawing();

    BeginMode2D(sheet.GetCamera());
    ClearBackground(Settings::CurrentTheme().Background);
	sheet.Draw();

    EndMode2D();

    UI();
    EndDrawing();
}

void App::SaveCurrentSheet()
{
	CreateSaveFolder();
    if (GetCurrentSheet().GetFilePath().empty()) {
        SaveCurrentSheetAs();
        return;
	}

	auto& sheet = GetCurrentSheet();
    bool success = Serializer::saveSheet(sheet.GetFilePath(), sheet.GetCircuit(), sheet.GetSimulationSettings());
    if (success) 
        sheet.SetSaved();
}

void App::SaveCurrentSheetAs()
{
	CreateSaveFolder();

    auto& sheet = GetCurrentSheet();
    nfdu8filteritem_t filters[] = { { "Latch circuit", "json" } };
    std::u8string defaultPath = saveFolder.u8string();

    NFD::UniquePathU8 outPath;
    nfdresult_t result = NFD::SaveDialog(outPath, filters, 1,
        reinterpret_cast<const char*>(defaultPath.c_str()), (sheet.GetTitle() + ".json").c_str());

    if (result == NFD_CANCEL)
        return;
    if (result == NFD_ERROR) {
        printf("Error: %s\n", NFD::GetError());
        return;
    }

    std::filesystem::path path(reinterpret_cast<const char8_t*>(outPath.get()));
    if (Serializer::saveSheet(path, sheet.GetCircuit(), sheet.GetSimulationSettings())) {
        std::u8string name = path.stem().u8string();
        sheet.SetFilePath(path);
        sheet.SetTitle(std::string(name.begin(), name.end()));
        sheet.SetSaved();
    }
}

void App::LoadSheetFromFile()
{
    nfdu8filteritem_t filters[] = { { "Latch circuit", "json" } };
    std::u8string defaultPath = saveFolder.u8string();

    NFD::UniquePathU8 outPath;
    nfdresult_t result = NFD::OpenDialog(outPath, filters, 1,
        reinterpret_cast<const char*>(defaultPath.c_str()));

    if (result == NFD_CANCEL)
        return;
    if (result == NFD_ERROR) {
        printf("Error: %s\n", NFD::GetError());
        return;
    }

    std::filesystem::path path(reinterpret_cast<const char8_t*>(outPath.get()));
	int index = 0;
    for (auto& sheet : sheets) {
        if (sheet->GetFilePath() == path) {
            requested_tab = index;
            return;
        }
		index++;
	}
    auto newSheet = std::make_unique<Sheet>();
    if (!Serializer::loadSheet(path, newSheet->GetCircuit(), newSheet->GetSimulationSettings())) {
        return;
    }

    std::u8string name = path.stem().u8string();
	newSheet->SetViewport({ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() });
    newSheet->SetTitle(std::string(name.begin(), name.end()));
	newSheet->SetFilePath(path);
	newSheet->ResetHistory();
    sheets.push_back(std::move(newSheet));
    requested_tab = sheets.size() - 1;
}

void App::MenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save", "Ctrl+S"))
                SaveCurrentSheet();
            if (ImGui::MenuItem("Save as"))
                SaveCurrentSheetAs();
            if (ImGui::MenuItem("Load file"))
                LoadSheetFromFile();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
	}
}

void App::TabBar()
{
    float height = ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 2;
    if (ImGui::BeginViewportSideBar("##SheetTabs", ImGui::GetMainViewport(), ImGuiDir_Up, height,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGuiTabBarFlags flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll;
        if (ImGui::BeginTabBar("Sheets", flags)) { 
            int index = 0;
            for (auto sheet = sheets.begin(); sheet != sheets.end();)
            {
                auto title = sheet->get()->GetTitle();
                ImGuiTabItemFlags tabFlags = 0;
                if (!sheet->get()->IsSaved())
                    tabFlags |= ImGuiTabItemFlags_UnsavedDocument;

                if (requested_tab == index) {
					tabFlags |= ImGuiTabItemFlags_SetSelected;
                    requested_tab = -1;
				}

                bool tab_open = true;
                ImGui::PushID(sheet->get());
				// I pass nullptr for the open state if there's only one tab so it can not be closed
                if (ImGui::BeginTabItem((title + "###tab").c_str(), sheets.size() > 1 ? &tab_open : nullptr, tabFlags)) {
                    current_sheet_index = index;
                    ImGui::EndTabItem();
                }
                ImGui::PopID();

                if (!tab_open && sheets.size() > 1) {
                    sheet = sheets.erase(sheet);
                    if (current_sheet_index >= index && current_sheet_index > 0)
                        current_sheet_index--;
                    continue;
                }
                else {
                    sheet++;
                    index++;
                }

            }

            std::string new_sheet_title = "Untitled Sheet" + std::to_string(sheets.size() + 1);
            if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
            {
                sheets.push_back(std::make_unique<Sheet>());
                sheets.back()->SetViewport({ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() });
                sheets.back()->SetTitle(new_sheet_title);

                requested_tab = sheets.size() - 1;
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();   
}

void App::Run()
{
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        HandleInput();
        Update(deltaTime);
        Draw();
    }
}
