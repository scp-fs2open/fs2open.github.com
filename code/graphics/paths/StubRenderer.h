
#pragma once

#include "graphics/paths/PathRenderer.h"

namespace graphics
{
	namespace paths
	{
		class StubRenderer : public PathRenderer
		{
		public:
			StubRenderer()
			{}
			virtual ~StubRenderer()
			{}

			virtual void beginFrame() SCP_OVERRIDE
			{}

			virtual void cancelFrame() SCP_OVERRIDE
			{}

			virtual void endFrame() SCP_OVERRIDE
			{}

			virtual void scissor(float x, float y, float w, float h) SCP_OVERRIDE
			{}

			virtual void resetScissor() SCP_OVERRIDE
			{}

			virtual void resetTransform() SCP_OVERRIDE
			{}

			virtual void translate(float x, float y) SCP_OVERRIDE
			{}

			virtual void rotate(float rad) SCP_OVERRIDE
			{}

			virtual void skewX(float rad) SCP_OVERRIDE
			{}

			virtual void skewY(float rad) SCP_OVERRIDE
			{}

			virtual void scale(float x, float y) SCP_OVERRIDE
			{}

			virtual DrawPaint createLinearGradient(float sx, float sy, float ex,
				float ey, color* icol, color* ocol) SCP_OVERRIDE
			{
				return DrawPaint();
			}

			virtual void setAlpha(float alpha) SCP_OVERRIDE
			{}

			virtual void setFillColor(color* color) SCP_OVERRIDE
			{}

			virtual void setFillPaint(const DrawPaint& paint) SCP_OVERRIDE
			{}

			virtual void setStrokeColor(color* color) SCP_OVERRIDE
			{}

			virtual void setStrokePaint(const DrawPaint& paint) SCP_OVERRIDE
			{}

			virtual void setStrokeWidth(float witdh) SCP_OVERRIDE
			{}

			virtual void beginPath() SCP_OVERRIDE
			{}

			virtual void moveTo(float x, float y) SCP_OVERRIDE
			{}

			virtual void setSolidity(Solidity solid) SCP_OVERRIDE
			{}

			virtual void lineTo(float x, float y) SCP_OVERRIDE
			{}

			virtual void rectangle(float x, float y, float w, float h) SCP_OVERRIDE
			{};

			virtual void roundedRectangle(float x, float y, float w, float h, float radius) SCP_OVERRIDE
			{}

			virtual void circle(float x, float y, float r) SCP_OVERRIDE
			{}

			virtual void ellipse(float x, float y, float rx, float ry) SCP_OVERRIDE
			{}

			virtual void arc(float cx, float cy, float r, float a0, float a1, Direction dir) SCP_OVERRIDE
			{}

			virtual void closePath() SCP_OVERRIDE
			{}

			virtual int createFontMem(const char* name, unsigned char* data, int ndata, int freeData) SCP_OVERRIDE
			{
				return 0;
			}

			virtual void fontSize(float size) SCP_OVERRIDE
			{}

			virtual void textLetterSpacing(float spacing) SCP_OVERRIDE
			{}

			virtual void fontFaceId(int font) SCP_OVERRIDE
			{}

			virtual float text(float x, float y, const char* string, const char* end) SCP_OVERRIDE
			{
				return 0.0f;
			}

			virtual float textBounds(float x, float y, const char* string, const char* end, float* bounds) SCP_OVERRIDE
			{
				return 0.0f;
			}

			virtual void textMetrics(float* ascender, float* descender, float* lineh) SCP_OVERRIDE
			{
				if (ascender)
					*ascender = 3.f;
				if (descender)
					*descender = 0.5f;
				if (lineh)
					*lineh = 4.f;
			}

			virtual void textAlign(TextAlign align) SCP_OVERRIDE
			{}

			virtual void fill() SCP_OVERRIDE
			{}

			virtual void stroke() SCP_OVERRIDE
			{}

			virtual void saveState() SCP_OVERRIDE
			{}

			virtual void resetState() SCP_OVERRIDE
			{}

			virtual void restoreState() SCP_OVERRIDE
			{}
		};
	}
}
