#pragma once
#include <filesystem>
#include "raylib.h"
#include "imgui.h"

#include "Action.h"
#include "Circuit.h"
#include "ComponentInfo.h"
#include "Settings.h"

struct MouseInputs {
	bool leftButtonDown			= false;
	bool rightButtonDown		= false;
	Vector2 mousePositionScreen = { 0, 0 };
	Vector2 mousePositionWorld	= { 0, 0 };
	float mouseWheelDelta		= 0.0f;
};

struct Clipboard {
	std::vector<componentInfo> components;
	std::vector<Wire> wires;
	Vector2 center{ 0, 0 };
};

struct PanningContext {
	Vector2 initial_pos;
	Vector2 initial_camera_target;
};

struct DraggingContext {
	Vector2 initial_mouse_pos;
	Vector2 snapped_delta;
	Wire::Elbow elbow = Wire::Elbow::HorizontalFirst; // reroute bend, chosen from the drag direction
	bool elbowLocked = false;                         // held once committed; re-armed near the start
};

struct ConnectingContext {
	enum ConnectionType { PIN, JUNCTION, INPUT } type; // PIN = dragging from an output pin, INPUT = from a free input pin
	int sourceNodeID;
	int wireID;
	Vector2 sourcePos;
	WireSegment sourceSegment;

	int sourceComponentID;
	PinRef targetPin;
	int sourceInputIndex = -1;                        // which input pin the drag started from (type == INPUT)
	Wire::Elbow elbow = Wire::Elbow::HorizontalFirst; // which way the L-route bends
	bool elbowLocked = false;                         // held once set; re-armed by returning to the source
};

struct SelectionContext {
	Vector2 selectionStart;
	Vector2 selectionEnd;
	Rectangle selectionRect;
};

class Sheet

{
public:
	Sheet();
	
	void HandleInput();
	void Update(float deltaTime);
	void UI();
	void Draw();

	void DeleteSelected();
	void CopySelected(Clipboard& clipboard);
	void Paste(const Clipboard& clipboard);
	void Undo();
	void Redo();
	void StepSimulation();
	void ResetHistory();

	void SetInputs(MouseInputs input, bool shift_down, componentInfo::Type selected_gate_type) {
		mouse_inputs = input;
		this->shift_down = shift_down;
		this->selected_gate_type = selected_gate_type;
	}

	void SetViewport(Rectangle viewport);
	void SetTitle(const std::string& title) { this->title = title; }
	void SetFilePath(const std::filesystem::path& path) { this->file_path = path; }
	void SetSaved() { lastSavedSnapshotID = action_manager.getCurrentSnapshotID(); }

	const std::filesystem::path GetFilePath() const { return file_path; }
	std::string GetTitle() const { return title; }
	Rectangle GetViewport() const { return viewport; }
	const Camera2D& GetCamera() const { return camera; }
	Circuit& GetCircuit() { return circuit; }
	SimulationSettings& GetSimulationSettings() { return simulation_settings; }
	bool IsSaved() const { return (lastSavedSnapshotID == action_manager.getCurrentSnapshotID()) && !file_path.empty(); }
private:
	void DrawGrid() const;

	void UpdateIdleState();
	void UpdatePanningState();
	void UpdateDraggingState();
	void UpdateConnectingState();
	void UpdateSelectingState();
	std::vector<componentInfo > getNodeInfoCopy(std::vector<int>& ids);

	void clearSelection();


	Rectangle viewport;

	enum class MouseState { Idle, Panning, Dragging, Connecting, Selecting };

	void(Sheet::* mouse_state_update_functions[5])() = {
		&Sheet::UpdateIdleState,
		&Sheet::UpdatePanningState,
		&Sheet::UpdateDraggingState,
		&Sheet::UpdateConnectingState,
		&Sheet::UpdateSelectingState
	};

	const std::string mouse_state_names[5] = { "Idle", "Panning", "Dragging", "Connecting", "Selecting" };

	Camera2D camera;

	float current_zoom = 1.0f;


	MouseState current_mouse_state = MouseState::Idle;

	PanningContext panning_context;
	DraggingContext dragging_context;
	ConnectingContext connecting_context;
	SelectionContext selecting_context;

	Circuit circuit;

	bool gate_placed = false;

	std::vector <int> selected_component_ids;
	int hovered_component_id = -1;
	std::unordered_map<int, std::vector<int>> selected_wire_nodes;
	std::unordered_map<int, std::vector<WireSegment>> selected_wire_segments;

	SimulationSettings simulation_settings;
	bool is_running = false;
	float time_since_last_tick = 0.0f;
	int number_of_ticks = 0;

	ActionManager action_manager;

	MouseInputs mouse_inputs;
	bool shift_down = false;
	bool is_mouse_over_viewport = false;
	componentInfo::Type selected_gate_type = componentInfo::Type::AND;

	std::string title;
	std::filesystem::path file_path = {};

	int lastSavedSnapshotID = -1;
};