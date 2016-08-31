
#include "globalincs/pstypes.h"

#if SCP_COMPILER_IS_GNU
// Disable warnings inside the NanoVG implementation
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include "graphics/paths/NVGRenderer.h"

#include "graphics/opengl/gropengl.h"
#include "graphics/opengl/gropenglstate.h"
#include "graphics/opengl/gropengltnl.h"
#include "graphics/opengl/gropenglshader.h"

#include "nanovg/nanovg.h"
// NanoVG supports OpenGL 2 and 3, we currently use OpenGL 2
//#define NANOVG_GL2_IMPLEMENTATION
#define NANOVG_GL3_IMPLEMENTATION

#include "nanovg/nanovg_gl.h"

#if SCP_COMPILER_IS_GNU
#pragma GCC diagnostic pop
#endif

namespace
{
	void resetGLState()
	{
		// After we are done with nanovg we need to tell our state tracker that things have changed
		GL_state.Blend(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_state.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

		GL_state.CullFace(GL_FALSE);
		GL_state.CullFaceValue(GL_BACK);

		GL_state.FrontFaceValue(GL_CCW);

		GL_state.DepthTest(GL_FALSE);
		GL_state.ScissorTest(GL_FALSE);
		GL_state.ColorMask(GL_TRUE);

		GL_state.SetStencilType(STENCIL_TYPE_WRITE);
		GL_state.StencilTest(GL_TRUE);

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(0);

		if ( GL_version >= 30 ) {
			glBindVertexArray(GL_vao);
		}

		GL_state.Array.BindArrayBuffer(0);
		GL_state.Array.BindUniformBuffer(0);
		opengl_shader_set_current(nullptr);

		// Now reset the values to what we need
		GL_state.SetStencilType(STENCIL_TYPE_NONE);
		GL_state.StencilTest(GL_FALSE);
	}

	NVGcolor fsColorToNVG(color* c)
	{
		return nvgRGBA(c->red, c->green, c->blue, c->is_alphacolor ? c->alpha : 255);
	}
}

namespace graphics
{
	namespace paths
	{
		NVGRenderer::NVGRenderer() : m_inFrame(false)
		{
			m_context = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
		}

		NVGRenderer::~NVGRenderer()
		{
			if (m_context)
			{
				nvgDeleteGL3(m_context);
				m_context = nullptr;
			}
		}

		void NVGRenderer::beginFrame()
		{
			Assertion(!m_inFrame, "beginFrame() was called withoug ending previous frame! Check your path renderer calls or get a coder!");

			nvgBeginFrame(m_context, gr_screen.max_w, gr_screen.max_h, 1.0f);
			m_inFrame = true;
		}

		void NVGRenderer::cancelFrame()
		{
			nvgCancelFrame(m_context);
			m_inFrame = false;
		}

		void NVGRenderer::endFrame()
		{
			GR_DEBUG_SCOPE(nvg_scope, "NanoVG flush");

			if ( GL_version >= 30 ) {
				glBindVertexArray(0);
			}

			gr_opengl_set_2d_matrix();

			nvgEndFrame(m_context);

			gr_opengl_end_2d_matrix();

			resetGLState();
			m_inFrame = false;
		}

		void NVGRenderer::scissor(float x, float y, float w, float h)
		{
			nvgScissor(m_context, x, y, w, h);
		}

		void NVGRenderer::resetScissor()
		{
			nvgResetScissor(m_context);
		}

		void NVGRenderer::resetTransform()
		{
			nvgResetTransform(m_context);
		}

		void NVGRenderer::translate(float x, float y)
		{
			nvgTranslate(m_context, x, y);
		}

		void NVGRenderer::rotate(float rad)
		{
			nvgRotate(m_context, rad);
		}

		void NVGRenderer::skewX(float rad)
		{
			nvgSkewX(m_context, rad);
		}

		void NVGRenderer::skewY(float rad)
		{
			nvgSkewY(m_context, rad);
		}

		void NVGRenderer::scale(float x, float y)
		{
			nvgScale(m_context, x, y);
		}

		DrawPaint NVGRenderer::createLinearGradient(float sx, float sy, float ex,
			float ey, color* icol, color* ocol)
		{
			DrawPaint p;

			p.nvg = nvgLinearGradient(m_context, sx, sy, ex, ey, fsColorToNVG(icol), fsColorToNVG(ocol));

			return p;
		}

		void NVGRenderer::setAlpha(float alpha)
		{
			nvgGlobalAlpha(m_context, alpha);
		}

		void NVGRenderer::setFillColor(color* color)
		{
			nvgFillColor(m_context, fsColorToNVG(color));
		}

		void NVGRenderer::setFillPaint(const DrawPaint& paint)
		{
			nvgFillPaint(m_context, paint.nvg);
		}

		void NVGRenderer::setStrokeColor(color* color)
		{
			nvgStrokeColor(m_context, fsColorToNVG(color));
		}

		void NVGRenderer::setStrokePaint(const DrawPaint& paint)
		{
			nvgStrokePaint(m_context, paint.nvg);
		}

		void NVGRenderer::setStrokeWidth(float width)
		{
			nvgStrokeWidth(m_context, width);
		}

		void NVGRenderer::beginPath()
		{
			nvgBeginPath(m_context);
		}

		void NVGRenderer::moveTo(float x, float y)
		{
			nvgMoveTo(m_context, x, y);
		}

		void NVGRenderer::setSolidity(Solidity solid)
		{
			int nvgSolid;
			switch (solid)
			{
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

		void NVGRenderer::lineTo(float x, float y)
		{
			nvgLineTo(m_context, x, y);
		}

		void NVGRenderer::rectangle(float x, float y, float w, float h)
		{
			nvgRect(m_context, x, y, w, h);
		}

		void NVGRenderer::roundedRectangle(float x, float y, float w, float h, float radius)
		{
			nvgRoundedRect(m_context, x, y, w, h, radius);
		}

		void NVGRenderer::circle(float x, float y, float r)
		{
			nvgCircle(m_context, x, y, r);
		}

		void NVGRenderer::ellipse(float x, float y, float rx, float ry)
		{
			nvgEllipse(m_context, x, y, rx, ry);
		}

		void NVGRenderer::arc(float cx, float cy, float r, float a0, float a1, Direction dir)
		{
			int nvgDir;
			switch (dir)
			{
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

		void NVGRenderer::closePath()
		{
			nvgClosePath(m_context);
		}

		int NVGRenderer::createFontMem(const char* name, unsigned char* data, int ndata, int freeData)
		{
			return nvgCreateFontMem(m_context, name, data, ndata, freeData);
		}

		void NVGRenderer::fontSize(float size)
		{
			nvgFontSize(m_context, size);
		}

		void NVGRenderer::textLetterSpacing(float spacing)
		{
			nvgTextLetterSpacing(m_context, spacing);
		}

		void NVGRenderer::fontFaceId(int font)
		{
			nvgFontFaceId(m_context, font);
		}

		float NVGRenderer::text(float x, float y, const char* string, const char* end)
		{
			auto res = nvgText(m_context, x, y, string, end);

			resetGLState();

			return res;
		}

		float NVGRenderer::textBounds(float x, float y, const char* string, const char* end, float* bounds)
		{
			return nvgTextBounds(m_context, x, y, string, end, bounds);
		}

		void NVGRenderer::textMetrics(float* ascender, float* descender, float* lineh)
		{
			nvgTextMetrics(m_context, ascender, descender, lineh);
		}

		void NVGRenderer::textAlign(TextAlign align)
		{
			int nvgAlign = 0;
			if (align & ALIGN_LEFT)
			{
				nvgAlign |= NVG_ALIGN_LEFT;
			}
			if (align & ALIGN_CENTER)
			{
				nvgAlign |= NVG_ALIGN_CENTER;
			}
			if (align & ALIGN_RIGHT)
			{
				nvgAlign |= NVG_ALIGN_RIGHT;
			}

			if (align & ALIGN_TOP)
			{
				nvgAlign |= NVG_ALIGN_TOP;
			}
			if (align & ALIGN_MIDDLE)
			{
				nvgAlign |= NVG_ALIGN_MIDDLE;
			}
			if (align & ALIGN_BOTTOM)
			{
				nvgAlign |= NVG_ALIGN_BOTTOM;
			}
			if (align & ALIGN_BASELINE)
			{
				nvgAlign |= NVG_ALIGN_BASELINE;
			}

			nvgTextAlign(m_context, nvgAlign);
		}

		void NVGRenderer::fill()
		{
			nvgFill(m_context);
		}

		void NVGRenderer::stroke()
		{
			nvgStroke(m_context);
		}

		void NVGRenderer::saveState()
		{
			nvgSave(m_context);
		}

		void NVGRenderer::resetState()
		{
			nvgReset(m_context);
		}

		void NVGRenderer::restoreState()
		{
			nvgRestore(m_context);
		}
	}
}
