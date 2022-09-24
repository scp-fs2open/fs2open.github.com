//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

// This code was adapted from the original NanoVG OpenGL renderer to work with the FreeSpace Open graphics API

#include "NanoVGRenderer.h"
#include "tracing/tracing.h"


// That is a wrapper function for log prints to be availiable for nanovg components. For now it is stb_truetype.h
// Planted by ksotar with blessing from asarium
extern "C" {
	void nvgOldCPrintf(SCP_FORMAT_STRING const char *message, ...) {
		if (LoggingEnabled) {
			SCP_string buf;
			va_list args;
			va_start(args, message);
			vsprintf(buf, message, args);
			va_end(args);
			outwnd_printf2("%s", message);
		}
	}
}


namespace {
using namespace graphics::paths;

int nvgRenderCreate(void* userptr) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);

	try {
		renderer->initialize();
		return 1;
	} catch (...) {
		return 0;
	}
}

void nvgRenderDelete(void* userptr) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	delete renderer;
}

void nvgViewport(void* userptr, int width, int height) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	renderer->setViewport(width, height);
}

void nvgRenderTriangles(void* userptr, NVGpaint* paint, NVGscissor* scissor, const NVGvertex* verts, int nverts) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	renderer->renderTriangles(paint, scissor, verts, nverts);
}
void nvgRenderFill(void* userptr,
				   NVGpaint* paint,
				   NVGscissor* scissor,
				   float fringe,
				   const float* bounds,
				   const NVGpath* paths,
				   int npaths) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	renderer->renderFill(paint, scissor, fringe, bounds, paths, npaths);
}
void nvgRenderStroke(void* userptr,
					 NVGpaint* paint,
					 NVGscissor* scissor,
					 float fringe,
					 float strokeWidth,
					 const NVGpath* paths,
					 int npaths) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	renderer->renderStroke(paint, scissor, fringe, strokeWidth, paths, npaths);
}
void nvgRenderFlush(void* userptr) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	renderer->renderFlush();

}
void nvgRenderCancel(void* userptr) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	renderer->renderCancel();
}
int nvgCreateTexture(void* userptr, int type, int w, int h, int imageFlags, const unsigned char* data) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	return renderer->createTexture(type, w, h, imageFlags, data);
}
int nvgUpdateTexture(void* userptr, int image, int x, int y, int w, int h, const unsigned char* data) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	return renderer->updateTexture(image, x, y, w, h, data);
}
int nvgDeleteTexture(void* userptr, int image) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	return renderer->deleteTexture(image);
}
int nvgGetTextureSize(void* userptr, int image, int* w, int* h) {
	auto renderer = static_cast<NanoVGRenderer*>(userptr);
	return renderer->getTextureSize(image, w, h);
}

NVGcolor premulColor(NVGcolor c) {
	c.r *= c.a;
	c.g *= c.a;
	c.b *= c.a;
	return c;
}
vec4 colorToVec4(NVGcolor c) {
	vec4 out;
	out.xyzw.x = c.r;
	out.xyzw.y = c.g;
	out.xyzw.z = c.b;
	out.xyzw.w = c.a;
	return out;
}
void xformToMat3x4(float m3[12], float* t) {
	m3[0] = t[0];
	m3[1] = t[1];
	m3[2] = 0.0f;
	m3[3] = 0.0f;
	m3[4] = t[2];
	m3[5] = t[3];
	m3[6] = 0.0f;
	m3[7] = 0.0f;
	m3[8] = t[4];
	m3[9] = t[5];
	m3[10] = 1.0f;
	m3[11] = 0.0f;
}

size_t maxVertCount(const NVGpath* paths, size_t npaths) {
	size_t count = 0;
	for (size_t i = 0; i < npaths; i++) {
		count += paths[i].nfill;
		count += paths[i].nstroke;
	}
	return count;
}

void vertexSet(NVGvertex* vtx, float x, float y, float u, float v) {
	vtx->x = x;
	vtx->y = y;
	vtx->u = u;
	vtx->v = v;
}

nanovg_material getDefaultMaterial() {
	nanovg_material mat;

	material_set_nanovg(&mat, -1);

	return mat;
}
}

namespace graphics {
namespace paths {

NVGcontext* createNanoVGContext() {
	NVGparams params;
	NVGcontext* ctx = nullptr;
	auto nvgRenderer = new NanoVGRenderer();

	memset(&params, 0, sizeof(params));
	params.renderCreate = nvgRenderCreate;
	params.renderDelete = nvgRenderDelete;
	params.renderViewport = nvgViewport;

	params.renderTriangles = nvgRenderTriangles;
	params.renderFill = nvgRenderFill;
	params.renderStroke = nvgRenderStroke;
	params.renderFlush = nvgRenderFlush;
	params.renderCancel = nvgRenderCancel;

	params.renderCreateTexture = nvgCreateTexture;
	params.renderUpdateTexture = nvgUpdateTexture;
	params.renderDeleteTexture = nvgDeleteTexture;
	params.renderGetTextureSize = nvgGetTextureSize;

	params.userPtr = nvgRenderer;
	params.edgeAntiAlias = 1;

	ctx = nvgCreateInternal(&params);
	if (ctx == nullptr) {
		return nullptr;
	}

	return ctx;
}

void deleteNanoVGContext(NVGcontext* context) {
	Assertion(context != nullptr, "Invalid context passed!");

	nvgDeleteInternal(context);
}

NanoVGRenderer::NanoVGRenderer() {
}

void NanoVGRenderer::initialize() {
	_vertexBuffer = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Streaming);

	_vertexLayout.add_vertex_component(vertex_format_data::POSITION2, sizeof(NVGvertex), (int) offsetof(NVGvertex, x));
	_vertexLayout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(NVGvertex), (int) offsetof(NVGvertex, u));

	// Now create all the materials that we are going to need
	{
		auto material = getDefaultMaterial();

		// Material for simple triangle draws. These use different primitive types so if that is moved into the
		// material struct at some point this can be easily adjusted
		_trianglesMaterial = material;
		_triangleFillMaterial = material;
		_triangleStrokeMaterial = material;
	}
	{
		// Materials for the fill shader
		auto material = getDefaultMaterial();
		material.set_stencil_test(true);
		material.set_stencil_mask(0xFF);
		material.set_stencil_func(ComparisionFunction::Always, 0, 0xFF);
		material.set_color_mask(false, false, false, false);

		material.set_front_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::IncrementWrap);
		material.set_back_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::DecrementWrap);

		_fillShapeMaterial = material;

		material.set_color(true, true, true, true);
		material.set_stencil_func(ComparisionFunction::Equal, 0x00, 0xFF);

		material.set_front_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
		material.set_back_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);

		_fillAntiAliasMaterial = material;

		material.set_stencil_func(ComparisionFunction::NotEqual, 0x00, 0xFF);

		material.set_front_stencil_op(StencilOperation::Zero, StencilOperation::Zero, StencilOperation::Zero);
		material.set_back_stencil_op(StencilOperation::Zero, StencilOperation::Zero, StencilOperation::Zero);

		_fillFillMaterial = material;
	}
	{
		// Pipeline states for the stroke shader
		auto material = getDefaultMaterial();
		material.set_stencil_test(true);
		material.set_stencil_mask(0xFF);
		material.set_stencil_func(ComparisionFunction::Equal, 0x00, 0xFF);
		material.set_front_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Increment);
		material.set_back_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Increment);
		_strokeFillMaterial = material;

		material.set_front_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
		material.set_back_stencil_op(StencilOperation::Keep, StencilOperation::Keep, StencilOperation::Keep);
		_strokeAntiaiasMaterial = material;

		material.set_color_mask(false, false, false, false);
		material.set_stencil_func(ComparisionFunction::Always, 0x00, 0xFF);
		material.set_front_stencil_op(StencilOperation::Zero, StencilOperation::Zero, StencilOperation::Zero);
		material.set_back_stencil_op(StencilOperation::Zero, StencilOperation::Zero, StencilOperation::Zero);
		_strokeClearStencilMaterial = material;
	}
}

void NanoVGRenderer::setViewport(int width, int height) {
	_viewport.x = width;
	_viewport.y = height;
}

void NanoVGRenderer::renderFill(NVGpaint* paint,
								NVGscissor* scissor,
								float fringe,
								const float* bounds,
								const NVGpath* paths,
								int npaths) {
	if (npaths <= 0) {
		// Ignore irrelevant render calls
		mprintf(("NanoVG asked us to render filled triangles but no paths were supplied!\n"));
		return;
	}

	auto call = addDrawCall(CallType::Fill);

	call->type = CallType::Fill;
	call->pathOffset = static_cast<uint32_t>(addPaths((size_t) npaths));
	call->pathCount = static_cast<uint32_t>((size_t) npaths);
	call->image = paint->image;

	if (npaths == 1 && paths[0].convex) {
		call->type = CallType::ConvexFill;
	}

	// Allocate vertices for all the paths.
	auto maxverts = maxVertCount(paths, (size_t) npaths) + 6;
	auto offset = addVertices(maxverts);

	for (int i = 0; i < npaths; i++) {
		Path* copy = &_paths[call->pathOffset + i];
		const NVGpath* path = &paths[i];
		memset(copy, 0, sizeof(Path));
		if (path->nfill > 0) {
			copy->fillOffset = static_cast<uint32_t>(offset);
			copy->fillCount = static_cast<uint32_t>((size_t) path->nfill);
			memcpy(&_vertices[offset], path->fill, sizeof(NVGvertex) * path->nfill);
			offset += path->nfill;
		}
		if (path->nstroke > 0) {
			copy->strokeOffset = static_cast<uint32_t>(offset);
			copy->strokeCount = static_cast<uint32_t>((size_t) path->nstroke);
			memcpy(&_vertices[offset], path->stroke, sizeof(NVGvertex) * path->nstroke);
			offset += path->nstroke;
		}
	}

	// Quad
	call->triangleOffset = static_cast<uint32_t>(offset);
	call->triangleCount = 6;
	auto quad = &_vertices[call->triangleOffset];
	vertexSet(&quad[0], bounds[0], bounds[3], 0.5f, 1.0f);
	vertexSet(&quad[1], bounds[2], bounds[3], 0.5f, 1.0f);
	vertexSet(&quad[2], bounds[2], bounds[1], 0.5f, 1.0f);

	vertexSet(&quad[3], bounds[0], bounds[3], 0.5f, 1.0f);
	vertexSet(&quad[4], bounds[2], bounds[1], 0.5f, 1.0f);
	vertexSet(&quad[5], bounds[0], bounds[1], 0.5f, 1.0f);

	// Setup uniforms for draw calls
	if (call->type == CallType::Fill) {
		call->uniformIndex = addUniformData(2);
		// Simple shader for stencil
		auto frag = &_uniformData[call->uniformIndex];
		frag->strokeThr = -1.0f;
		frag->type = NanoVGShaderType::Simple;
		// Fill shader
		convertPaint(&_uniformData[call->uniformIndex + 1], paint, scissor, fringe, fringe, -1.0f);
	} else {
		call->uniformIndex = addUniformData(1);
		// Fill shader
		convertPaint(&_uniformData[call->uniformIndex], paint, scissor, fringe, fringe, -1.0f);
	}
}
void NanoVGRenderer::renderTriangles(NVGpaint* paint, NVGscissor* scissor, const NVGvertex* verts, int nverts) {
	if (nverts <= 0) {
		// Ignore irrelevant render calls
		mprintf(("NanoVG asked us to render triangles but no vertices were supplied!\n"));
		return;
	}

	_vertices.insert(_vertices.end(), verts, verts + nverts);

	auto call = addDrawCall(CallType::Triangles);
	call->type = CallType::Triangles;
	call->triangleCount = static_cast<uint32_t>(nverts);
	call->triangleOffset = static_cast<uint32_t>(addVertices(verts, static_cast<size_t>(nverts)));
	call->image = paint->image;

	call->uniformIndex = addUniformData(1);

	auto uniformData = &_uniformData[call->uniformIndex];
	auto succcess = convertPaint(uniformData, paint, scissor, 1.0f, 1.0f, -1.0f);

	Assertion(succcess, "Failed to convert paint, probably caused by an invalid texture handle.");

	uniformData->type = NanoVGShaderType::Image;
}
void NanoVGRenderer::renderStroke(NVGpaint* paint,
								  NVGscissor* scissor,
								  float fringe,
								  float strokeWidth,
								  const NVGpath* paths,
								  int npaths) {
	if (npaths <= 0) {
		// Ignore irrelevant render calls
		mprintf(("NanoVG asked us to render stroke triangles but no paths were supplied!\n"));
		return;
	}

	auto call = addDrawCall(CallType::Stroke);

	call->type = CallType::Stroke;
	call->pathOffset = static_cast<uint32_t>(addPaths((size_t) npaths));
	call->pathCount = static_cast<uint32_t>(npaths);
	call->image = paint->image;

	// Allocate vertices for all the paths.
	auto maxverts = maxVertCount(paths, (size_t) npaths);
	auto offset = addVertices(maxverts);

	for (int i = 0; i < npaths; i++) {
		Path* copy = &_paths[call->pathOffset + i];
		const NVGpath* path = &paths[i];
		memset(copy, 0, sizeof(*copy));
		if (path->nstroke) {
			copy->strokeOffset = static_cast<uint32_t>(offset);
			copy->strokeCount = static_cast<uint32_t>(path->nstroke);
			memcpy(&_vertices[offset], path->stroke, sizeof(NVGvertex) * path->nstroke);
			offset += path->nstroke;
		}
	}
	// Fill shader
	call->uniformIndex = addUniformData(2);

	convertPaint(&_uniformData[call->uniformIndex], paint, scissor, strokeWidth, fringe, -1.0f);
	convertPaint(&_uniformData[call->uniformIndex + 1], paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);
}
void NanoVGRenderer::renderFlush() {
	if (_drawCalls.empty() && _drawFillCalls.empty() && _drawConvexFillCalls.empty() && _drawStrokeCalls.empty() && _drawTriangleCalls.empty() ) {
		return;
	}

	TRACE_SCOPE(tracing::NanoVGFlushFrame);
	GR_DEBUG_SCOPE("NanoVG flush");

	gr_set_viewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	_uniformBuffer = gr_get_uniform_buffer(uniform_block_type::NanoVGData, _uniformData.size());
	// This copies the uniform data from our vector into the uniform buffer aligner
	for (auto& uniform : _uniformData) {
		memcpy(_uniformBuffer.aligner().addTypedElement<nanovg_draw_data>(), &uniform, sizeof(uniform));
	}

	_uniformBuffer.submitData();
	gr_update_buffer_data(_vertexBuffer, sizeof(NVGvertex) * _vertices.size(), _vertices.data());
	{
		TRACE_SCOPE(tracing::NanoVGFlushAll);
		for (auto& drawCall : _drawCalls) {
			switch (drawCall.type) {
			case CallType::Fill:
				drawFill(drawCall);
				break;
			case CallType::ConvexFill:
				drawConvexFill(drawCall);
				break;
			case CallType::Stroke:
				drawStroke(drawCall);
				break;
			case CallType::Triangles:
				drawTriangles(drawCall);
				break;
			}
		}
	}
	{
		TRACE_SCOPE(tracing::NanoVGFlushFill);
		for (auto& drawCall : _drawFillCalls) {
			drawFill(drawCall);
		}
	}
	{
		TRACE_SCOPE(tracing::NanoVGFlushConvex);
		for (auto& drawCall : _drawConvexFillCalls) {
			drawConvexFill(drawCall);
		}
    }
	{
		TRACE_SCOPE(tracing::NanoVGFlushPath);
		for (auto& drawCall : _drawStrokeCalls) {
			drawStroke(drawCall);
		}
	}
	{
		TRACE_SCOPE(tracing::NanoVGFlushTriangle);
		for (auto& drawCall : _drawTriangleCalls) {
			drawTriangles(drawCall);
		}
	}
	// Reset all data again
	renderCancel();
}
void NanoVGRenderer::renderCancel() {
	// Clear all data written by the render functions
	_vertices.clear();
	_uniformData.clear();
	_drawCalls.clear();
	_paths.clear();
	_drawFillCalls.clear();
	_drawConvexFillCalls.clear();
	_drawStrokeCalls.clear();
	_drawTriangleCalls.clear();
}
int NanoVGRenderer::createTexture(int type, int w, int h, int imageFlags, const unsigned char* data) {
	int bpp;
	if (type == NVG_TEXTURE_RGBA) {
		bpp = 32;
	} else {
		bpp = 8;
	}

	Assertion(!(imageFlags & NVG_IMAGE_REPEATX), "Repeat X is not supported yet!");
	Assertion(!(imageFlags & NVG_IMAGE_REPEATY), "Repeat Y is not supported yet!");
	Assertion(!(imageFlags & NVG_IMAGE_GENERATE_MIPMAPS), "Generate Mipmaps is not supported yet!");

	std::unique_ptr<uint8_t[]> data_buffer(new uint8_t[w * h * bpp]);
	if (data != nullptr) {
		memcpy(data_buffer.get(), data, static_cast<size_t>(w * h * bpp));
	}

	Image img;
	img.data = std::move(data_buffer);
	img.bitmap = bm_create(bpp, w, h, img.data.get(), bpp == 8 ? BMP_AABITMAP : 0);
	img.type = type;
	img.flags = imageFlags;
	img.width = w;
	img.height = h;

	auto id = ++_lastImageId;
	_textureMap.insert(std::make_pair(id, std::move(img)));

	return id;
}
int NanoVGRenderer::updateTexture(int image, int  /*x*/, int  /*y*/, int  /*w*/, int  /*h*/, const unsigned char* data) {
	auto texture = getTexture(image);
	if (texture == nullptr) {
		return 0;
	}

	int bpp;
	if (texture->type == NVG_TEXTURE_RGBA) {
		bpp = 32;
	} else {
		bpp = 8;
	}

	// Copy the updated data to our internal buffer so that changes in the hardware texture detail don't discard the
	// changes NanoVG made to the texture
	memcpy(texture->data.get(), data, texture->width * texture->height * bpp / 8);

	// TODO: This could probably be done better by only uploading the changed area
	gr_update_texture(texture->bitmap, bpp, data, texture->width, texture->height);

	return 1;
}
int NanoVGRenderer::deleteTexture(int image) {
	auto iter = _textureMap.find(image);
	if (iter == _textureMap.end()) {
		return 0;
	}

	bm_release(iter->second.bitmap);
	// This will call the destructor of the class and free the resources
	_textureMap.erase(iter);

	return 1;
}
int NanoVGRenderer::getTextureSize(int image, int* w, int* h) {
	auto img = getTexture(image);
	if (img == nullptr) {
		return 0;
	}

	*w = img->width;
	*h = img->height;

	return 1;
}
NanoVGRenderer::DrawCall* NanoVGRenderer::addDrawCall() {
	_drawCalls.emplace_back();
	return &_drawCalls.back();
}
NanoVGRenderer::DrawCall* NanoVGRenderer::addDrawCall(CallType t) {for (auto& drawCall : _drawCalls) {
		switch (t) {
		case CallType::Fill:
			_drawFillCalls.emplace_back();
			return  &_drawFillCalls.back();
			break;
		case CallType::ConvexFill:
			_drawConvexFillCalls.emplace_back();
			return  &_drawConvexFillCalls.back();
			break;
		case CallType::Stroke:
			_drawStrokeCalls.emplace_back();
			return  &_drawStrokeCalls.back();
			break;
		case CallType::Triangles:
			_drawTriangleCalls.emplace_back();
			return  &_drawTriangleCalls.back();
			break;
		}
	}
	_drawCalls.emplace_back();
	return &_drawCalls.back();
}
size_t NanoVGRenderer::addUniformData(size_t num) {
	nanovg_draw_data data;
	memset(&data, 0, sizeof(data));
	// Push an unitialized struct into the vector but set the viewport here since that is the same for all elements
	data.viewSize = { i2fl(_viewport.x), i2fl(_viewport.y) };

	auto current = _uniformData.size();
	_uniformData.resize(current + num, data);
	return current;
}

size_t NanoVGRenderer::addVertices(const NVGvertex* vert, size_t num) {
	auto offset = _vertices.size();
	_vertices.insert(_vertices.end(), vert, vert + num);
	return offset;
}

size_t NanoVGRenderer::addVertices(size_t num) {
	auto offset = _vertices.size();
	_vertices.resize(offset + num);
	return offset;
}

size_t NanoVGRenderer::addPaths(size_t num) {
	auto offset = _paths.size();
	_paths.resize(offset + num);
	return offset;
}

bool NanoVGRenderer::convertPaint(graphics::nanovg_draw_data* frag,
								  NVGpaint* paint,
								  NVGscissor* scissor,
								  float width,
								  float fringe,
								  float strokeThr) {
	float invxform[6];

	frag->innerCol = colorToVec4(premulColor(paint->innerColor));
	frag->outerCol = colorToVec4(premulColor(paint->outerColor));

	if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f) {
		memset(&frag->scissorMat, 0, sizeof(frag->scissorMat));
		frag->scissorExt.x = 1.0f;
		frag->scissorExt.y = 1.0f;
		frag->scissorScale.x = 1.0f;
		frag->scissorScale.y = 1.0f;
	} else {
		nvgTransformInverse(invxform, scissor->xform);
		xformToMat3x4(frag->scissorMat, invxform);

		frag->scissorExt.x = scissor->extent[0];
		frag->scissorExt.y = scissor->extent[1];
		frag->scissorScale.x =
			sqrtf(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
		frag->scissorScale.y =
			sqrtf(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;
	}

	memcpy(&frag->extent, paint->extent, sizeof(frag->extent));
	frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
	frag->strokeThr = strokeThr;

	if (paint->image != 0) {
		auto tex = getTexture(paint->image);
		if (tex == nullptr) {
			return false;
		}
		if ((tex->flags & NVG_IMAGE_FLIPY) != 0) {
			float flipped[6];
			nvgTransformScale(flipped, 1.0f, -1.0f);
			nvgTransformMultiply(flipped, paint->xform);
			nvgTransformInverse(invxform, flipped);
		} else {
			nvgTransformInverse(invxform, paint->xform);
		}
		frag->type = NanoVGShaderType::FillImage;

		if (tex->type == NVG_TEXTURE_RGBA) {
			frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0 : 1;
		} else {
			frag->texType = 2;
		}
		//		printf("frag->texType = %d\n", frag->texType);
	} else {
		frag->type = NanoVGShaderType::FillGradient;
		frag->radius = paint->radius;
		frag->feather = paint->feather;
		nvgTransformInverse(invxform, paint->xform);
	}

	xformToMat3x4(frag->paintMat, invxform);

	auto tex = getTexture(paint->image);
	if (tex != nullptr) {
		// Since textures may be stored in texture arrays we need to pass the index of the image into the shader
		frag->texArrayIndex = bm_get_array_index(tex->bitmap);
	}

	return true;
}

NanoVGRenderer::Image* NanoVGRenderer::getTexture(int id) {
	auto iter = _textureMap.find(id);
	if (iter == _textureMap.end()) {
		return nullptr;
	}
	return &iter->second;
}

void NanoVGRenderer::drawTriangles(const DrawCall& call) {
	GR_DEBUG_SCOPE("Draw triangles");
	TRACE_SCOPE(tracing::NanoVGDrawTriangles);

	auto mat = _trianglesMaterial;
	materialSetTexture(mat, call.image);

	gr_bind_uniform_buffer(uniform_block_type::NanoVGData, _uniformBuffer.getAlignerElementOffset(call.uniformIndex),
	                       sizeof(nanovg_draw_data), _uniformBuffer.bufferHandle());

	gr_render_nanovg(&mat, PRIM_TYPE_TRIS, &_vertexLayout, call.triangleOffset, call.triangleCount, _vertexBuffer);
}
void NanoVGRenderer::sortCalls( std::vector<DrawCall> calls){
	std::sort(calls.begin(), calls.end(),
		[](const DrawCall& a,const DrawCall& b)
	{ if( a.uniformIndex == b.uniformIndex){
			return a.uniformIndex > b.uniformIndex;
		}
		return a.image > b.image;
	});
}
void NanoVGRenderer::drawFill(const DrawCall& call) {
	GR_DEBUG_SCOPE("Draw fill");
	TRACE_SCOPE(tracing::NanoVGDrawFill);

	auto mat = _fillShapeMaterial;
	mat.set_texture_map(TM_BASE_TYPE, -1);

	gr_bind_uniform_buffer(uniform_block_type::NanoVGData, _uniformBuffer.getAlignerElementOffset(call.uniformIndex),
	                       sizeof(nanovg_draw_data), _uniformBuffer.bufferHandle());

	auto pathOffset = call.pathOffset;
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRISTRIP,
						 &_vertexLayout,
						 _paths[pathOffset + i].fillOffset,
						 _paths[pathOffset + i].fillCount,
						 _vertexBuffer);
	}

	mat = _fillAntiAliasMaterial;
	materialSetTexture(mat, call.image);
	gr_bind_uniform_buffer(uniform_block_type::NanoVGData,
	                       _uniformBuffer.getAlignerElementOffset(call.uniformIndex + 1), sizeof(nanovg_draw_data),
	                       _uniformBuffer.bufferHandle());

	// Draw fringes
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRISTRIP,
						 &_vertexLayout,
						 _paths[pathOffset + i].strokeOffset,
						 _paths[pathOffset + i].strokeCount,
						 _vertexBuffer);
	}

	mat = _fillFillMaterial;
	materialSetTexture(mat, call.image);

	gr_render_nanovg(&mat, PRIM_TYPE_TRIS, &_vertexLayout, call.triangleOffset, call.triangleCount, _vertexBuffer);
}
void NanoVGRenderer::drawConvexFill(const DrawCall& call) {
	GR_DEBUG_SCOPE("Draw convex fill");
	TRACE_SCOPE(tracing::NanoVGDrawConvexFill);

	auto mat = _triangleFillMaterial;
	materialSetTexture(mat, call.image);
	gr_bind_uniform_buffer(uniform_block_type::NanoVGData, _uniformBuffer.getAlignerElementOffset(call.uniformIndex),
	                       sizeof(nanovg_draw_data), _uniformBuffer.bufferHandle());

	auto pathOffset = call.pathOffset;
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRIFAN,
						 &_vertexLayout,
						 _paths[pathOffset + i].fillOffset,
						 _paths[pathOffset + i].fillCount,
						 _vertexBuffer);
	}

	mat = _triangleStrokeMaterial;
	materialSetTexture(mat, call.image);
	// Draw fringes
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRISTRIP,
						 &_vertexLayout,
						 _paths[pathOffset + i].strokeOffset,
						 _paths[pathOffset + i].strokeCount,
						 _vertexBuffer);
	}
}
void NanoVGRenderer::drawStroke(const DrawCall& call) {
	GR_DEBUG_SCOPE("Draw stroke");
	TRACE_SCOPE(tracing::NanoVGDrawStroke);

	auto mat = _strokeFillMaterial;
	materialSetTexture(mat, call.image);
	gr_bind_uniform_buffer(uniform_block_type::NanoVGData,
	                       _uniformBuffer.getAlignerElementOffset(call.uniformIndex + 1), sizeof(nanovg_draw_data),
	                       _uniformBuffer.bufferHandle());

	auto pathOffset = call.pathOffset;

	// Fill the stroke base without overlap
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRISTRIP,
						 &_vertexLayout,
						 _paths[pathOffset + i].strokeOffset,
						 _paths[pathOffset + i].strokeCount,
						 _vertexBuffer);
	}

	// Draw anti-aliased pixels.
	gr_bind_uniform_buffer(uniform_block_type::NanoVGData, _uniformBuffer.getAlignerElementOffset(call.uniformIndex),
	                       sizeof(nanovg_draw_data), _uniformBuffer.bufferHandle());
	mat = _strokeAntiaiasMaterial;
	materialSetTexture(mat, call.image);
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRISTRIP,
						 &_vertexLayout,
						 _paths[pathOffset + i].strokeOffset,
						 _paths[pathOffset + i].strokeCount,
						 _vertexBuffer);
	}

	// Clear stencil buffer.
	mat = _strokeClearStencilMaterial;
	materialSetTexture(mat, call.image);
	for (size_t i = 0; i < call.pathCount; ++i) {
		gr_render_nanovg(&mat,
						 PRIM_TYPE_TRISTRIP,
						 &_vertexLayout,
						 _paths[pathOffset + i].strokeOffset,
						 _paths[pathOffset + i].strokeCount,
						 _vertexBuffer);
	}
}
void NanoVGRenderer::materialSetTexture(nanovg_material& mat, int nvgHandle) {
	auto tex = getTexture(nvgHandle);
	if (tex != nullptr) {
		mat.set_texture_map(TM_BASE_TYPE, tex->bitmap);
		if (tex->type == NVG_TEXTURE_ALPHA) {
			mat.set_texture_type(material::TEX_TYPE_AABITMAP);
		}
	} else {
		mat.set_texture_map(TM_BASE_TYPE, -1);
	}
}

}
}
