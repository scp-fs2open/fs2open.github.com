#pragma once

#include "graphics/2d.h"
#include "graphics/material.h"
#include "nanovg/nanovg.h"
#include "graphics/util/UniformBuffer.h"
#include "graphics/util/uniform_structs.h"

#include <unordered_map>
#include <memory>

namespace graphics {
namespace paths {


NVGcontext* createNanoVGContext();

void deleteNanoVGContext(NVGcontext* context);

class NanoVGRenderer {
	enum class CallType {
		Fill, ConvexFill, Stroke, Triangles
	};
	struct Image {
		int bitmap = -1;
		std::unique_ptr<uint8_t[]> data;
		int type = 0;
		int flags = 0;
		int width = -1;
		int height = -1;

		Image() {
		}

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		Image(Image&& other) noexcept {
			*this = std::move(other);
		}
		Image& operator=(Image&& other) noexcept {
			std::swap(bitmap, other.bitmap);
			std::swap(data, other.data);
			std::swap(type, other.type);
			std::swap(flags, other.flags);
			std::swap(width, other.width);
			std::swap(height, other.height);

			return *this;
		}
	};
	struct DrawCall {
		CallType type;

		uint32_t triangleOffset;
		uint32_t triangleCount;

		uint32_t pathOffset;
		uint32_t pathCount;

		size_t uniformIndex;

		int image;
	};
	struct Path {
		uint32_t fillOffset;
		uint32_t fillCount;
		uint32_t strokeOffset;
		uint32_t strokeCount;
	};
	gr_buffer_handle _vertexBuffer;
	vertex_layout _vertexLayout;

	util::UniformBuffer _uniformBuffer;

	nanovg_material _trianglesMaterial;
	nanovg_material _triangleFillMaterial;
	nanovg_material _triangleStrokeMaterial;

	nanovg_material _fillShapeMaterial;
	nanovg_material _fillAntiAliasMaterial;
	nanovg_material _fillFillMaterial;

	nanovg_material _strokeFillMaterial;
	nanovg_material _strokeAntiaiasMaterial;
	nanovg_material _strokeClearStencilMaterial;

	std::vector<NVGvertex> _vertices;
	std::vector<DrawCall> _drawCalls;
	std::vector<graphics::nanovg_draw_data> _uniformData;
	std::vector<Path> _paths;

	std::unordered_map<int, Image> _textureMap;
	int _lastImageId = 0;

	ivec2 _viewport;

	DrawCall* addDrawCall();
	size_t addUniformData(size_t num);
	size_t addVertices(const NVGvertex* vert, size_t num);

	size_t addVertices(size_t num);
	size_t addPaths(size_t num);

	bool convertPaint(graphics::nanovg_draw_data* frag,
					  NVGpaint* paint,
					  NVGscissor* scissor,
					  float width,
					  float fringe,
					  float strokeThr);

	Image* getTexture(int id);

	void drawTriangles(const DrawCall& call);
	void drawFill(const DrawCall& call);
	void drawConvexFill(const DrawCall& call);
	void drawStroke(const DrawCall& call);

	void materialSetTexture(nanovg_material& mat, int nvgHandle);
 public:
	explicit NanoVGRenderer();

	void initialize();

	void setViewport(int width, int height);

	void renderFill(NVGpaint* paint,
					NVGscissor* scissor,
					float fringe,
					const float* bounds,
					const NVGpath* paths,
					int npaths);

	void renderStroke(NVGpaint* paint,
					  NVGscissor* scissor,
					  float fringe,
					  float strokeWidth,
					  const NVGpath* paths,
					  int npaths);

	void renderTriangles(NVGpaint* paint, NVGscissor* scissor, const NVGvertex* verts, int nverts);

	void renderFlush();

	void renderCancel();

	int createTexture(int type, int w, int h, int imageFlags, const unsigned char* data);

	int deleteTexture(int image);

	int updateTexture(int image, int x, int y, int w, int h, const unsigned char* data);

	int getTextureSize(int image, int* w, int* h);
};

}
}
