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

			NVGcontext* m_context;
			bool m_inFrame;
		public:
			PathRenderer();
			~PathRenderer();

			static bool init();

			static inline PathRenderer* instance()
			{
				return s_instance.get();
			}

			static void shutdown();

			void beginFrame();

			void cancelFrame();

			void endFrame();

			void scissor(float x, float y, float w, float h);

			void resetScissor();

			/* begin transforms */

			void resetTransform();

			void translate(float x, float y);

			void rotate(float rad);

			void skewX(float rad);

			void skewY(float rad);

			void scale(float x, float y);

			/* end transforms */

			/* begin paint creation */

			DrawPaint createLinearGradient(float sx, float sy, float ex,
				float ey, color* icol, color* ocol);

			/* end paint creation */

			/* begin color handling */
			void setAlpha(float alpha);

			void setFillColor(color* color);

			void setFillPaint(const DrawPaint& paint);

			void setStrokeColor(color* color);

			void setStrokePaint(const DrawPaint& paint);

			void setStrokeWidth(float witdh);
			/* end color handling */

			void beginPath();

			void moveTo(float x, float y);

			void setSolidity(Solidity solid);

			/* begin shapes
			   TODO: Replace this with doxygen */

			void lineTo(float x, float y);

			void rectangle(float x, float y, float w, float h);

			void roundedRectangle(float x, float y, float w, float h, float radius);

			void circle(float x, float y, float r);

			void ellipse(float x, float y, float rx, float ry);

			void arc(float cx, float cy, float r, float a0, float a1, Direction dir);

			/* end shapes */

			/* begin font and text */

			int createFontMem(const char* name, unsigned char* data, int ndata, int freeData);

			void fontSize(float size);

			void textLetterSpacing(float spacing);

			void fontFaceId(int font);

			float text(float x, float y, const char* string, const char* end);

			float textBounds(float x, float y, const char* string, const char* end, float* bounds);

			void textMetrics(float* ascender, float* descender, float* lineh);

			void textAlign(TextAlign align);

			/* end font and text */

			void closePath();

			void fill();

			void stroke();

			void saveState();

			void resetState();

			void restoreState();
		};
	}
}

