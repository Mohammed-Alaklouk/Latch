#include "Gate.h"
#include <cmath>

namespace {
	// Point on a quadratic Bezier curve.
	Vector2 qbez(Vector2 a, Vector2 b, Vector2 c, float t) {
		float u = 1.0f - t;
		return { u * u * a.x + 2.0f * u * t * b.x + t * t * c.x,
				 u * u * a.y + 2.0f * u * t * b.y + t * t * c.y };
	}

	constexpr int ArcSegments = 18;
}

int Gate::inputPinsContainPoint(Vector2 point) const
{
	for (int i = 0; i < m_inputWireIds.size(); ++i) {
		Vector2 inputPosition = getInputPosition(i);
		if (CheckCollisionCircles(point, PinRadius, inputPosition, PinRadius)) {
			return i;
		}
	}
	return -1;
}

int Gate::outputPinContainsPoint(Vector2 point) const
{
	for (int i = 0; i < m_outputWireIds.size(); ++i) {
		Vector2 outputPosition = getOutputPosition(i);
		if (CheckCollisionCircles(point, PinRadius, outputPosition, PinRadius)) {
			return i;
		}
	}
	return -1;
}

Color Gate::pickBorder(bool selected, bool highlighted) const
{
	if (selected) return Settings::CurrentTheme().SelectionColor;
	if (highlighted) return Settings::CurrentTheme().Gates.BorderHighlightColor;
	return Settings::CurrentTheme().Gates.BorderColor;
}

// Fill a closed silhouette and trace its outline; fan both windings since raylib culls back-facing triangles and the shapes wind opposite ways.
void Gate::fillOutline(const std::vector<Vector2>& pts, Color borderColor) const
{
	if (pts.size() < 3) return;
	DrawTriangleFan(const_cast<Vector2*>(pts.data()), (int)pts.size(), Settings::CurrentTheme().Gates.BodyTop);
	std::vector<Vector2> reversed(pts.rbegin(), pts.rend());
	DrawTriangleFan(reversed.data(), (int)reversed.size(), Settings::CurrentTheme().Gates.BodyTop);
	for (size_t i = 0; i + 1 < pts.size(); ++i)
		DrawLineEx(pts[i], pts[i + 1], 1.5f, borderColor);
	DrawLineEx(pts.back(), pts.front(), 1.5f, borderColor);
}

// Flat back with a semicircular nose reaching rightX.
std::vector<Vector2> Gate::andShape(float rightX) const
{
	float top = m_rect.y, bot = m_rect.y + m_rect.height, left = m_rect.x;
	float r = m_rect.height * 0.5f, cy = m_rect.y + r;
	float arcX = rightX - r;

	std::vector<Vector2> pts;
	pts.push_back({ left, bot });
	pts.push_back({ left, top });
	for (int i = 0; i <= ArcSegments; ++i) {
		float a = -PI * 0.5f + PI * (float)i / ArcSegments;
		pts.push_back({ arcX + r * cosf(a), cy + r * sinf(a) });
	}
	return pts;
}

// Concave back with top/bottom curves meeting at a nose on rightX; ordered from the nose so the fan pivots there.
std::vector<Vector2> Gate::orShape(float rightX) const
{
	float top = m_rect.y, bot = m_rect.y + m_rect.height, left = m_rect.x;
	float cy = m_rect.y + m_rect.height * 0.5f;
	float depth = orBackDepth();

	Vector2 nose = { rightX, cy };
	Vector2 tl = { left, top }, bl = { left, bot };
	Vector2 backCtrl = { left + depth, cy };
	Vector2 edgeCtrl = { left + (rightX - left) * 0.55f, 0 };
	Vector2 topCtrl = { edgeCtrl.x, top };
	Vector2 botCtrl = { edgeCtrl.x, bot };

	std::vector<Vector2> pts;
	pts.push_back(nose);
	// top edge, nose -> top-left
	for (int i = ArcSegments - 1; i >= 0; --i)
		pts.push_back(qbez(tl, topCtrl, nose, (float)i / ArcSegments));
	// concave back, top-left -> bottom-left
	for (int i = 1; i <= ArcSegments; ++i)
		pts.push_back(qbez(tl, backCtrl, bl, (float)i / ArcSegments));
	// bottom edge, bottom-left -> nose
	for (int i = 1; i < ArcSegments; ++i)
		pts.push_back(qbez(bl, botCtrl, nose, (float)i / ArcSegments));
	return pts;
}

// Triangle pointing right to rightX.
std::vector<Vector2> Gate::notShape(float rightX) const
{
	float top = m_rect.y, bot = m_rect.y + m_rect.height, left = m_rect.x;
	float cy = m_rect.y + m_rect.height * 0.5f;
	return { { left, top }, { left, bot }, { rightX, cy } };
}

// Where a lead at height y meets the OR back curve (linear in y since the control point sits at mid-height).
float Gate::orBodyLeftAt(float y) const
{
	float top = m_rect.y, h = m_rect.height;
	float t = h > 0 ? (y - top) / h : 0.0f;
	if (t < 0) t = 0; else if (t > 1) t = 1;
	return m_rect.x + 2.0f * orBackDepth() * (1.0f - t) * t;
}

// Negation bubble filling the complement gate's last body cell
void Gate::drawBubble(Color borderColor) const
{
	float cy = m_rect.y + m_rect.height * 0.5f;
	float cx = m_rect.x + m_rect.width - BubbleRadius;
	DrawCircle(cx, cy, BubbleRadius, Settings::CurrentTheme().Gates.BodyTop);
	DrawCircleLines(cx, cy, BubbleRadius, borderColor);
}

// The detached second curve behind an XOR/XNOR back.
void Gate::drawXorBackArc(Color borderColor) const
{
	float top = m_rect.y, bot = m_rect.y + m_rect.height;
	float cy = m_rect.y + m_rect.height * 0.5f;
	float gap = 4.0f;
	Vector2 tl = { m_rect.x - gap, top };
	Vector2 bl = { m_rect.x - gap, bot };
	Vector2 ctrl = { m_rect.x - gap + orBackDepth(), cy };

	Vector2 prev = tl;
	for (int i = 1; i <= ArcSegments; ++i) {
		Vector2 cur = qbez(tl, ctrl, bl, (float)i / ArcSegments);
		DrawLineEx(prev, cur, 1.5f, borderColor);
		prev = cur;
	}
}

void Gate::drawBody(Color borderColor) const
{
	// Fallback for gates without a distinctive shape: rounded body with a centred label.
	DrawRectangleGradientV(m_rect.x, m_rect.y, m_rect.width, m_rect.height, Settings::CurrentTheme().Gates.BodyTop, Settings::CurrentTheme().Gates.BodyBot);
	DrawRectangle(m_rect.x, m_rect.y, 3.0f, m_rect.height, Settings::CurrentTheme().Gates.AccentColor);
	DrawRectangleRoundedLinesEx(m_rect, 0.1f, 4, 1.5f, borderColor);

	const char* label = getLabel();
	int text_w = MeasureText(label, 12);
	float tx = m_rect.x + (m_rect.width - text_w) * 0.5f;
	float ty = m_rect.y + (m_rect.height - 12) * 0.5f;
	DrawText(label, (int)tx, (int)ty, 12, Settings::CurrentTheme().LabelColor);
}

void Gate::draw(std::vector<LogicLevel>& inputs, bool selected, bool highlighted) const
{
	Color borderColor = pickBorder(selected, highlighted);

	drawBody(borderColor);

	// Output lead from the body edge out to the pin.
	Vector2 out = getOutputPosition();
	DrawLineEx({ m_rect.x + m_rect.width, out.y }, out, 2.0f, Settings::CurrentTheme().Gates.LeadColor);
	DrawCircle(out.x, out.y, PinRadius, Settings::CurrentTheme().LogicLevelColors[m_outputValues[0]]);

	for (int i = 0; i < inputs.size(); i++) {
		Vector2 pin = getInputPosition(i);
		Vector2 join = { bodyLeftAt(pin.y), pin.y };
		if (join.x - pin.x > 0.5f) DrawLineEx(pin, join, 2.0f, Settings::CurrentTheme().Gates.LeadColor);
		DrawCircle(pin.x, pin.y, PinRadius, Settings::CurrentTheme().LogicLevelColors[inputs[i]]);
		if (inputs[i] == LogicLevel::UNDEFINED) DrawCircle(pin.x, pin.y, PinRadius + 1.5f, Settings::CurrentTheme().UndefinedHaloColor);
	}
}