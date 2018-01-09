#include "graphics/paths/PathRenderer.h"

#include "graphics/paths/NanoVGRenderer.h"

namespace {

NVGcolor fsColorToNVG(color* c) {
	return nvgRGBA(c->red, c->green, c->blue, c->is_alphacolor ? c->alpha : 255);
}

}

namespace graphics {
namespace paths {
std::unique_ptr<PathRenderer> PathRenderer::s_instance;

bool PathRenderer::init() {
	s_instance = std::unique_ptr<PathRenderer>(new PathRenderer());

	return true;
}

void PathRenderer::shutdown() {
	s_instance = nullptr;
}

PathRenderer::PathRenderer() : m_inFrame(false) {
	m_context = createNanoVGContext();
}

PathRenderer::~PathRenderer() {
	if (m_context) {
		deleteNanoVGContext(m_context);
		m_context = nullptr;
	}
}

void PathRenderer::beginFrame() {
	Assertion(!m_inFrame,
			  "beginFrame() was called withoug ending previous frame! Check your path renderer calls or get a coder!");

	nvgBeginFrame(m_context, gr_screen.max_w, gr_screen.max_h, 1.0f);
	m_inFrame = true;
}

void PathRenderer::cancelFrame() {
	nvgCancelFrame(m_context);
	m_inFrame = false;
}

void PathRenderer::endFrame() {
	nvgEndFrame(m_context);

	m_inFrame = false;
}

void PathRenderer::scissor(float x, float y, float w, float h) {
	nvgScissor(m_context, x, y, w, h);
}

void PathRenderer::resetScissor() {
	nvgResetScissor(m_context);
}

void PathRenderer::resetTransform() {
	nvgResetTransform(m_context);
}

void PathRenderer::translate(float x, float y) {
	nvgTranslate(m_context, x, y);
}

void PathRenderer::rotate(float rad) {
	nvgRotate(m_context, rad);
}

void PathRenderer::skewX(float rad) {
	nvgSkewX(m_context, rad);
}

void PathRenderer::skewY(float rad) {
	nvgSkewY(m_context, rad);
}

void PathRenderer::scale(float x, float y) {
	nvgScale(m_context, x, y);
}

DrawPaint PathRenderer::createLinearGradient(float sx, float sy, float ex, float ey, color* icol, color* ocol) {
	DrawPaint p;

	p.nvg = nvgLinearGradient(m_context, sx, sy, ex, ey, fsColorToNVG(icol), fsColorToNVG(ocol));

	return p;
}

void PathRenderer::setAlpha(float alpha) {
	nvgGlobalAlpha(m_context, alpha);
}

void PathRenderer::setFillColor(color* color) {
	nvgFillColor(m_context, fsColorToNVG(color));
}

void PathRenderer::setFillPaint(const DrawPaint& paint) {
	nvgFillPaint(m_context, paint.nvg);
}

void PathRenderer::setStrokeColor(::color* color) {
	nvgStrokeColor(m_context, fsColorToNVG(color));
}

void PathRenderer::setStrokePaint(const DrawPaint& paint) {
	nvgStrokePaint(m_context, paint.nvg);
}

void PathRenderer::setStrokeWidth(float width) {
	nvgStrokeWidth(m_context, width);
}

void PathRenderer::beginPath() {
	nvgBeginPath(m_context);
}

void PathRenderer::moveTo(float x, float y) {
	nvgMoveTo(m_context, x, y);
}

void PathRenderer::setSolidity(Solidity solid) {
	int nvgSolid;
	switch (solid) {
	case SOLIDITY_HOLE:
		nvgSolid = NVG_HOLE;
		break;
	case SOLIDITY_SOLID:
		nvgSolid = NVG_SOLID;
		break;
	default:
		nvgSolid = NVG_SOLID;
		break;
	}

	nvgPathWinding(m_context, nvgSolid);
}

void PathRenderer::lineTo(float x, float y) {
	nvgLineTo(m_context, x, y);
}

void PathRenderer::rectangle(float x, float y, float w, float h) {
	nvgRect(m_context, x, y, w, h);
}

void PathRenderer::roundedRectangle(float x, float y, float w, float h, float radius) {
	nvgRoundedRect(m_context, x, y, w, h, radius);
}

void PathRenderer::circle(float x, float y, float r) {
	nvgCircle(m_context, x, y, r);
}

void PathRenderer::ellipse(float x, float y, float rx, float ry) {
	nvgEllipse(m_context, x, y, rx, ry);
}

void PathRenderer::arc(float cx, float cy, float r, float a0, float a1, Direction dir) {
	int nvgDir;
	switch (dir) {
	case DIR_CCW:
		nvgDir = NVG_CCW;
		break;
	case DIR_CW:
		nvgDir = NVG_CW;
		break;
	default:
		nvgDir = NVG_CW;
		break;
	}

	nvgArc(m_context, cx, cy, r, a0, a1, nvgDir);
}

void PathRenderer::closePath() {
	nvgClosePath(m_context);
}

int PathRenderer::createFontMem(const char* name, unsigned char* data, int ndata, int freeData) {
	return nvgCreateFontMem(m_context, name, data, ndata, freeData);
}

void PathRenderer::fontSize(float size) {
	nvgFontSize(m_context, size);
}

void PathRenderer::textLetterSpacing(float spacing) {
	nvgTextLetterSpacing(m_context, spacing);
}

void PathRenderer::fontFaceId(int font) {
	nvgFontFaceId(m_context, font);
}

float PathRenderer::text(float x, float y, const char* string, const char* end) {
	return nvgText(m_context, x, y, string, end);
}

float PathRenderer::textBounds(float x, float y, const char* string, const char* end, float* bounds) {
	return nvgTextBounds(m_context, x, y, string, end, bounds);
}

void PathRenderer::textMetrics(float* ascender, float* descender, float* lineh) {
	nvgTextMetrics(m_context, ascender, descender, lineh);
}

void PathRenderer::textAlign(TextAlign align) {
	int nvgAlign = 0;
	if (align & ALIGN_LEFT) {
		nvgAlign |= NVG_ALIGN_LEFT;
	}
	if (align & ALIGN_CENTER) {
		nvgAlign |= NVG_ALIGN_CENTER;
	}
	if (align & ALIGN_RIGHT) {
		nvgAlign |= NVG_ALIGN_RIGHT;
	}

	if (align & ALIGN_TOP) {
		nvgAlign |= NVG_ALIGN_TOP;
	}
	if (align & ALIGN_MIDDLE) {
		nvgAlign |= NVG_ALIGN_MIDDLE;
	}
	if (align & ALIGN_BOTTOM) {
		nvgAlign |= NVG_ALIGN_BOTTOM;
	}
	if (align & ALIGN_BASELINE) {
		nvgAlign |= NVG_ALIGN_BASELINE;
	}

	nvgTextAlign(m_context, nvgAlign);
}

void PathRenderer::fill() {
	nvgFill(m_context);
}

void PathRenderer::stroke() {
	nvgStroke(m_context);
}

void PathRenderer::saveState() {
	nvgSave(m_context);
}

void PathRenderer::resetState() {
	nvgReset(m_context);
}

void PathRenderer::restoreState() {
	nvgRestore(m_context);
}
}
}
