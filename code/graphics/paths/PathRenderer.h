#pragma once

#include <memory>

#include "globalincs/pstypes.h"
#include "nanovg/nanovg.h"

#include "graphics/2d.h"

namespace graphics
{
	namespace paths
	{
		enum Direction
		{
			DIR_CCW,
			DIR_CW
		};

		enum Solidity
		{
			SOLIDITY_SOLID,
			SOLIDITY_HOLE
		};

		enum TextAlign
		{
			// Horizontal align
			ALIGN_LEFT = 1 << 0,	// Default, align text horizontally to left.
			ALIGN_CENTER = 1 << 1,	// Align text horizontally to center.
			ALIGN_RIGHT = 1 << 2,	// Align text horizontally to right.

			// Vertical align
			ALIGN_TOP = 1 << 3,	// Align text vertically to top.
			ALIGN_MIDDLE = 1 << 4,	// Align text vertically to middle.
			ALIGN_BOTTOM = 1 << 5,	// Align text vertically to bottom.
			ALIGN_BASELINE = 1 << 6, // Default, align text vertically to baseline.
		};

		/**
		 * @brief A paint used for drawing
		 * @attention The contents of this struct are private, do not rely on any form of structure in it
		 */
		struct DrawPaint
		{
			// Use a union when more backends are implemented
			NVGpaint nvg;
		};

		class PathRenderer
		{
		private:
			static std::unique_ptr<PathRenderer> s_instance;

		protected:
			PathRenderer()
			{}
		public:
			virtual ~PathRenderer()
			{}

			static bool init();

			static inline PathRenderer* instance()
			{
				return s_instance.get();
			}

			static void shutdown();

			virtual void beginFrame() = 0;

			virtual void cancelFrame() = 0;

			virtual void endFrame() = 0;

			virtual void scissor(float x, float y, float w, float h) = 0;

			virtual void resetScissor() = 0;

			/* begin transforms */

			virtual void resetTransform() = 0;

			virtual void translate(float x, float y) = 0;

			virtual void rotate(float rad) = 0;

			virtual void skewX(float rad) = 0;

			virtual void skewY(float rad) = 0;

			virtual void scale(float x, float y) = 0;

			/* end transforms */

			/* begin paint creation */

			virtual DrawPaint createLinearGradient(float sx, float sy, float ex,
				float ey, color* icol, color* ocol) = 0;

			/* end paint creation */

			/* begin color handling */
			virtual void setAlpha(float alpha) = 0;

			virtual void setFillColor(color* color) = 0;

			virtual void setFillPaint(const DrawPaint& paint) = 0;

			virtual void setStrokeColor(color* color) = 0;

			virtual void setStrokePaint(const DrawPaint& paint) = 0;

			virtual void setStrokeWidth(float witdh) = 0;
			/* end color handling */

			virtual void beginPath() = 0;

			virtual void moveTo(float x, float y) = 0;

			virtual void setSolidity(Solidity solid) = 0;

			/* begin shapes
			   TODO: Replace this with doxygen */

			virtual void lineTo(float x, float y) = 0;

			virtual void rectangle(float x, float y, float w, float h) = 0;

			virtual void roundedRectangle(float x, float y, float w, float h, float radius) = 0;

			virtual void circle(float x, float y, float r) = 0;

			virtual void ellipse(float x, float y, float rx, float ry) = 0;

			virtual void arc(float cx, float cy, float r, float a0, float a1, Direction dir) = 0;

			/* end shapes */

			/* begin font and text */

			virtual int createFontMem(const char* name, unsigned char* data, int ndata, int freeData) = 0;

			virtual void fontSize(float size) = 0;

			virtual void textLetterSpacing(float spacing) = 0;

			virtual void fontFaceId(int font) = 0;

			virtual float text(float x, float y, const char* string, const char* end) = 0;

			virtual float textBounds(float x, float y, const char* string, const char* end, float* bounds) = 0;

			virtual void textMetrics(float* ascender, float* descender, float* lineh) = 0;

			virtual void textAlign(TextAlign align) = 0;

			/* end font and text */

			virtual void closePath() = 0;

			virtual void fill() = 0;

			virtual void stroke() = 0;

			virtual void saveState() = 0;

			virtual void resetState() = 0;

			virtual void restoreState() = 0;
		};
	}
}

