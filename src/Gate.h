#pragma once
#include <vector>
#include <memory>
#include "Pin.h"
#include "ComponentInfo.h"
#include "Settings.h"

class Gate {
public:
	Gate() = delete;
	Gate(int id, Vector2 position, int numInputs, int numOutputs, float w)
		// Even cell-height, min 40px, so the centre lands on the grid.
		: m_id(id), m_rect({ position.x, position.y, w, (numInputs > 2 ? 2 * numInputs : 4) * (float)Settings::grid::CellSize })
	{
		m_inputWireIds.resize(numInputs, -1);
		m_outputWireIds.resize(numOutputs, -1);
		m_outputValues.resize(numOutputs, LogicLevel::UNDEFINED);
	}

	virtual ~Gate() = default;

	bool containsPoint(Vector2 point) const {
		return CheckCollisionPointRec(point, m_rect);
	}

	int getInputWireId(int input_index) const {
		if (input_index < 0 || input_index >= m_inputWireIds.size()) return -1;
		return m_inputWireIds[input_index];
	}

	componentInfo getComponentInfo() const {
		return { getNodeInfoType(), m_id, { m_rect.x, m_rect.y }, m_inputWireIds, m_outputWireIds, m_outputValues };
	}

	virtual int inputPinsContainPoint(Vector2 point) const;
	virtual int outputPinContainsPoint(Vector2 point) const;

	virtual void evaluate(const std::vector<LogicLevel>& inputs) = 0;
	virtual void draw(std::vector<LogicLevel>& inputs, bool selected, bool highlighted) const;
	virtual const char* getLabel() const = 0;
	virtual componentInfo::Type getNodeInfoType() const = 0;
	virtual void onClick() {}

	// Each functional gate overrides drawBody() with its own silhouette; draw() adds the shared leads and pins.
	virtual void drawBody(Color borderColor) const;
	// Where an input lead meets the body at height y (flat left edge by default, concave back for the OR family).
	virtual float bodyLeftAt(float y) const { return m_rect.x; }

	Color pickBorder(bool selected, bool highlighted) const;

	// Silhouette builders returning a closed outline within m_rect ending at rightX on the output side.
	std::vector<Vector2> andShape(float rightX) const;
	std::vector<Vector2> orShape(float rightX) const;
	std::vector<Vector2> notShape(float rightX) const;

	void fillOutline(const std::vector<Vector2>& pts, Color borderColor) const;
	void drawBubble(Color borderColor) const;
	void drawXorBackArc(Color borderColor) const;

	// OR back bulge, keyed to the 4-cell silhouette (not m_rect) so an OR and its NOR twin match.
	float orBackDepth() const { return 4.0f * Settings::grid::CellSize * 0.32f; }
	float orBodyLeftAt(float y) const;

	constexpr static float BubbleRadius = 5.0f;

	// Inputs one cell clear of the body, spread symmetrically about its centre.
	virtual Vector2 getInputPosition(int input_index) const
	{
		int n = (int)m_inputWireIds.size();
		float cy = m_rect.y + m_rect.height / 2;
		return { m_rect.x - Settings::grid::CellSize, cy + (input_index - (n - 1) / 2.0f) * 2 * Settings::grid::CellSize };
	}

	virtual Vector2 getOutputPosition(int output_index = 0) const {
		return { m_rect.x + m_rect.width + Settings::grid::CellSize, m_rect.y + m_rect.height / 2 };
	}


	constexpr static float PinRadius = 5.0f;

	Rectangle m_rect;
	Color body_top = { 45, 50, 62, 255 };
	Color body_bot = { 32, 36, 46, 255 };
	Color AccentColor{ 0, 180, 120, 255 };
	Color BorderColor{ 80, 85, 95, 255 };
	Color BorderHighlightColor{ 100, 160, 255, 255 };
	Color BorderSelectedColor{ 0, 150, 255, 255 };
	Color LeadColor{ 90, 95, 110, 255 }; // short stubs joining pins to the body

	std::vector<int> m_inputWireIds;
	std::vector<int> m_outputWireIds;
	std::vector<LogicLevel> m_outputValues;
	int m_id;
};

class AndGate : public Gate {
public:
	constexpr static LogicLevel LookupTable[9] = {
		//		LOW				HIGH				UNDEFINED
		LogicLevel::LOW, LogicLevel::LOW,		LogicLevel::LOW, // LOW
		LogicLevel::LOW, LogicLevel::HIGH,		LogicLevel::UNDEFINED, // HIGH
		LogicLevel::LOW, LogicLevel::UNDEFINED,	LogicLevel::UNDEFINED // UNDEFINED
	};

	AndGate(int id, Vector2 position) : Gate(id, position, 2, 1, 4 * Settings::grid::CellSize)	{	}

	void evaluate(const std::vector<LogicLevel>& inputs) override{
		if (inputs.size() < 2) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}

		m_outputValues[0] = LookupTable[inputs[0] * 3 + inputs[1]];
	}

	void drawBody(Color borderColor) const override {
		fillOutline(andShape(m_rect.x + m_rect.width), borderColor);
	}

	const char* getLabel() const override { return "AND"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::AND; }
};

class OrGate : public Gate {
public:
	constexpr static LogicLevel LookupTable[9] = {
		//		LOW					HIGH				UNDEFINED
		LogicLevel::LOW,		LogicLevel::HIGH,	LogicLevel::UNDEFINED, // LOW
		LogicLevel::HIGH,		LogicLevel::HIGH,	LogicLevel::HIGH, // HIGH
		LogicLevel::UNDEFINED,	LogicLevel::HIGH,	LogicLevel::UNDEFINED // UNDEFINED
	};

	OrGate(int id, Vector2 position) : Gate(id, position, 2, 1,  4 * Settings::grid::CellSize)	{	}

	void evaluate(const std::vector<LogicLevel>& inputs) override{
		if (inputs.size() < 2) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}
		m_outputValues[0] = LookupTable[inputs[0] * 3 + inputs[1]];
	}

	void drawBody(Color borderColor) const override {
		fillOutline(orShape(m_rect.x + m_rect.width), borderColor);
	}
	float bodyLeftAt(float y) const override { return orBodyLeftAt(y); }

	const char* getLabel() const override { return "OR"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::OR; }
};

class NotGate : public Gate {
	public:
	constexpr static LogicLevel LookupTable[3] = {
		LogicLevel::HIGH, // LOW
		LogicLevel::LOW,  // HIGH
		LogicLevel::UNDEFINED // UNDEFINED
	};
	NotGate(int id, Vector2 position) : Gate(id, position, 1, 1, 5 * Settings::grid::CellSize)	{	}

	void evaluate(const std::vector<LogicLevel>& inputs) override{
		if (inputs.size() < 1) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}
		m_outputValues[0] = LookupTable[inputs[0]];
	}

	void drawBody(Color borderColor) const override {
		fillOutline(notShape(m_rect.x + m_rect.width - Settings::grid::CellSize), borderColor);
		drawBubble(borderColor);
	}

	const char* getLabel() const override { return "NOT"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::NOT; }
};

class LedGate : public Gate {
public:
	LedGate(int id, Vector2 position) : Gate(id, position, 1, 0, 4 * Settings::grid::CellSize) {
		m_rect.height =  4 * Settings::grid::CellSize;
	}

	void draw(std::vector<LogicLevel>& inputs, bool selected, bool highlighted) const override
	{
		auto borderColor = BorderColor;
		if (highlighted) borderColor = BorderHighlightColor;
		if (selected) borderColor = BorderSelectedColor;

		DrawRectangleGradientV(m_rect.x, m_rect.y, m_rect.width, m_rect.height, body_top, body_bot);
		DrawRectangle(m_rect.x, m_rect.y, 3.0f, m_rect.height, AccentColor);
		DrawRectangleRoundedLinesEx(m_rect, 0.1f, 4, 1.5f, borderColor);

		Color fillColor = LogicLevelColors[inputs[0]];
		Vector2 pin = getInputPosition(0);
		DrawLineEx({ m_rect.x, pin.y }, pin, 2.0f, LeadColor);
		DrawCircle(pin.x, pin.y, PinRadius, fillColor);
		if (inputs[0] == LogicLevel::UNDEFINED) fillColor = Fade(RED, 0.4f);

		DrawCircle(m_rect.x + m_rect.width / 2, m_rect.y + m_rect.height / 2, 20, fillColor);
		DrawCircleLines(m_rect.x + m_rect.width / 2, m_rect.y + m_rect.height / 2, 20, borderColor);
	}

	void evaluate(const std::vector<LogicLevel>& inputs) override{
	}
	const char* getLabel() const override { return "LED"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::LED; }
	Vector2 getOutputPosition(int output_index = 0) const override
	{
		return { 0, 0 }; // No output pins
	}
	int inputPinsContainPoint(Vector2 point) const override
	{
		Vector2 inputPosition = getInputPosition(0);
		if (CheckCollisionCircles(point, PinRadius, inputPosition, PinRadius)) {
			return 0;
		}
		return -1;
	}

};

class ToggleGate : public Gate {
	public:
	ToggleGate(int id, Vector2 position) : Gate(id, position, 0, 1, 4 * Settings::grid::CellSize)
	{
		m_rect.height =  4 * Settings::grid::CellSize;
		m_outputValues[0] = LogicLevel::LOW;
	}

	void evaluate(const std::vector<LogicLevel>& inputs) override {
	}

	const char* getLabel() const override { return "TOGGLE"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::TOGGLE; }
	
	void onClick() override {
		m_outputValues[0] = (m_outputValues[0] == LogicLevel::HIGH) ? LogicLevel::LOW : LogicLevel::HIGH;
	}

	void draw(std::vector<LogicLevel>& inputs, bool selected, bool highlighted) const override
	{
		auto borderColor = BorderColor;
		if (highlighted) borderColor = BorderHighlightColor;
		if (selected) borderColor = BorderSelectedColor;
		DrawRectangleGradientV(m_rect.x, m_rect.y, m_rect.width, m_rect.height, body_top, body_bot);
		DrawRectangle(m_rect.x, m_rect.y, 3.0f, m_rect.height, AccentColor);
		DrawRectangleRoundedLinesEx(m_rect, 0.1f, 4, 1.5f, borderColor);
		
		Color fillColor = LogicLevelColors[m_outputValues[0]];
		DrawCircle(m_rect.x + m_rect.width / 2, m_rect.y + m_rect.height / 2, 20, fillColor);
		DrawCircleLines(m_rect.x + m_rect.width / 2, m_rect.y + m_rect.height / 2, 20, borderColor);
		
		const char* label = m_outputValues[0] == LogicLevel::HIGH ? "1" : (m_outputValues[0] == LogicLevel::LOW ? "0" : "");
		int text_w = MeasureText(label, 12);
		float tx = m_rect.x + (m_rect.width - text_w) * 0.5f;
		float ty = m_rect.y + (m_rect.height - 12) * 0.5f;
		DrawText(label, (int)tx, (int)ty, 12, WHITE);

		Vector2 out = getOutputPosition();
		DrawLineEx({ m_rect.x + m_rect.width, out.y }, out, 2.0f, LeadColor);
		DrawCircle(out.x, out.y, PinRadius, LogicLevelColors[m_outputValues[0]]);
	}
};

class NandGate : public Gate {
public:
	constexpr static LogicLevel LookupTable[9] = {
		//		LOW				HIGH				UNDEFINED
		LogicLevel::HIGH, LogicLevel::HIGH,	LogicLevel::UNDEFINED, // LOW
		LogicLevel::HIGH, LogicLevel::LOW,  LogicLevel::UNDEFINED, // HIGH
		LogicLevel::UNDEFINED, LogicLevel::UNDEFINED, LogicLevel::UNDEFINED // UNDEFINED
	};
	NandGate(int id, Vector2 position) : Gate(id, position, 2, 1, 5 * Settings::grid::CellSize) { }
	void evaluate(const std::vector<LogicLevel>& inputs) override {
		if (inputs.size() < 2) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}
		m_outputValues[0] = LookupTable[inputs[0] * 3 + inputs[1]];
	}
	void drawBody(Color borderColor) const override {
		fillOutline(andShape(m_rect.x + m_rect.width - Settings::grid::CellSize), borderColor);
		drawBubble(borderColor);
	}
	const char* getLabel() const override { return "NAND"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::NAND; }
};

class NorGate : public Gate {
public:
	constexpr static LogicLevel LookupTable[9] = {
		//		LOW					HIGH				UNDEFINED
		LogicLevel::HIGH,		LogicLevel::LOW, LogicLevel::UNDEFINED, // LOW
		LogicLevel::LOW,		LogicLevel::LOW, LogicLevel::LOW, // HIGH
		LogicLevel::UNDEFINED,	LogicLevel::LOW, LogicLevel::UNDEFINED // UNDEFINED
	};
	NorGate(int id, Vector2 position) : Gate(id, position, 2, 1,  5 * Settings::grid::CellSize) { }
	void evaluate(const std::vector<LogicLevel>& inputs) override {
		if (inputs.size() < 2) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}
		m_outputValues[0] = LookupTable[inputs[0] * 3 + inputs[1]];
	}
	void drawBody(Color borderColor) const override {
		fillOutline(orShape(m_rect.x + m_rect.width - Settings::grid::CellSize), borderColor);
		drawBubble(borderColor);
	}
	float bodyLeftAt(float y) const override { return orBodyLeftAt(y); }
	const char* getLabel() const override { return "NOR"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::NOR; }
};

class XorGate : public Gate {
	public:
	constexpr static LogicLevel LookupTable[9] = {
		//		LOW					HIGH				UNDEFINED
		LogicLevel::LOW,	LogicLevel::HIGH,	LogicLevel::UNDEFINED, // LOW
		LogicLevel::HIGH,	LogicLevel::LOW,	LogicLevel::UNDEFINED, // HIGH
		LogicLevel::UNDEFINED, LogicLevel::UNDEFINED, LogicLevel::UNDEFINED // UNDEFINED
	};
	XorGate(int id, Vector2 position) : Gate(id, position, 2, 1,  4 * Settings::grid::CellSize) { }
	void evaluate(const std::vector<LogicLevel>& inputs) override {
		if (inputs.size() < 2) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}
		m_outputValues[0] = LookupTable[inputs[0] * 3 + inputs[1]];
	}
	void drawBody(Color borderColor) const override {
		fillOutline(orShape(m_rect.x + m_rect.width), borderColor);
		drawXorBackArc(borderColor);
	}
	float bodyLeftAt(float y) const override { return orBodyLeftAt(y); }
	const char* getLabel() const override { return "XOR"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::XOR; }
};

class XnorGate : public Gate {

	public:
	constexpr static LogicLevel LookupTable[9] = {
		//		LOW					HIGH				UNDEFINED
		LogicLevel::HIGH,	LogicLevel::LOW,	LogicLevel::UNDEFINED, // LOW
		LogicLevel::LOW,	LogicLevel::HIGH,	LogicLevel::UNDEFINED, // HIGH
		LogicLevel::UNDEFINED, LogicLevel::UNDEFINED, LogicLevel::UNDEFINED // UNDEFINED
	};
	XnorGate(int id, Vector2 position) : Gate(id, position, 2, 1,  5 * Settings::grid::CellSize) { }
	void evaluate(const std::vector<LogicLevel>& inputs) override {
		if (inputs.size() < 2) {
			m_outputValues[0] = LogicLevel::UNDEFINED;
			return;
		}
		m_outputValues[0] = LookupTable[inputs[0] * 3 + inputs[1]];
	}
	void drawBody(Color borderColor) const override {
		fillOutline(orShape(m_rect.x + m_rect.width - Settings::grid::CellSize), borderColor);
		drawBubble(borderColor);
		drawXorBackArc(borderColor);
	}
	float bodyLeftAt(float y) const override { return orBodyLeftAt(y); }
	const char* getLabel() const override { return "XNOR"; }
	componentInfo::Type getNodeInfoType() const override { return componentInfo::Type::XNOR; }
};




using GateFactory = std::unique_ptr<Gate>(*)(int id, Vector2 position);

template<typename T>
std::unique_ptr<Gate> makeGate(int id, Vector2 position) {
	return std::make_unique<T>(id, position);
}

inline constexpr GateFactory GateFactories[componentInfo::COMPONENT_COUNT] = {
	makeGate<AndGate>,     // AND
	makeGate<NandGate>,    // NAND
	makeGate<OrGate>,      // OR
	makeGate<NorGate>,     // NOR
	makeGate<XorGate>,     // XOR
	makeGate<XnorGate>,    // XNOR
	makeGate<NotGate>,     // NOT
	makeGate<LedGate>,     // LED
	makeGate<ToggleGate>,  // TOGGLE
};