//
//

// Undefine stupid windows API defines that get included by the prefix headers
#ifdef GetNextSibling
#undef GetNextSibling
#endif

#ifdef GetFirstChild
#undef GetFirstChild
#endif

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include "RocketRenderingInterface.h"
#include "RocketFileInterface.h"

#pragma pop_macro("Assert")

#include "graphics/2d.h"
#include "graphics/generic.h"
#include "graphics/material.h"
#include "tracing/categories.h"
#include "tracing/tracing.h"

using namespace Rocket::Core;

namespace scpui {

RocketRenderingInterface::RocketRenderingInterface()
{
	renderOffset.x = 0.0f;
	renderOffset.y = 0.0f;

	vertex_stream_buffer = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Streaming);
	index_stream_buffer  = gr_create_buffer(BufferType::Index, BufferUsageHint::Streaming);

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(Vertex), offsetof(Vertex, position));
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(Vertex), offsetof(Vertex, colour));
	layout.add_vertex_component(vertex_format_data::TEX_COORD2, sizeof(Vertex), offsetof(Vertex, tex_coord));
}

RocketRenderingInterface::~RocketRenderingInterface()
{
	if (vertex_stream_buffer.isValid()) {
		gr_delete_buffer(vertex_stream_buffer);
	}
	if (index_stream_buffer.isValid()) {
		gr_delete_buffer(index_stream_buffer);
	}
}
RocketRenderingInterface::Texture* RocketRenderingInterface::get_texture(Rocket::Core::TextureHandle handle)
{
	return reinterpret_cast<Texture*>(handle);
}
Rocket::Core::TextureHandle RocketRenderingInterface::get_texture_handle(RocketRenderingInterface::Texture* bitmap)
{
	return reinterpret_cast<Rocket::Core::TextureHandle>(bitmap);
}

CompiledGeometryHandle RocketRenderingInterface::CompileGeometry(Vertex* vertices, int num_vertices, int* indices,
                                                                 int num_indices, TextureHandle texture)
{
	TRACE_SCOPE(tracing::RocketCompileGeometry);
	GR_DEBUG_SCOPE("libRocket::CompileGeometry");

	auto* geom          = new CompiledGeometry();
	geom->vertex_buffer = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Static);
	gr_update_buffer_data(geom->vertex_buffer, num_vertices * sizeof(Vertex), reinterpret_cast<void*>(vertices));

	geom->index_buffer = gr_create_buffer(BufferType::Index, BufferUsageHint::Static);
	gr_update_buffer_data(geom->index_buffer, num_indices * sizeof(int), reinterpret_cast<void*>(indices));
	geom->num_elements = num_indices;

	geom->texture = get_texture(texture);

	return reinterpret_cast<CompiledGeometryHandle>(geom);
}
void RocketRenderingInterface::RenderCompiledGeometry(CompiledGeometryHandle geometry, const Vector2f& translation)
{
	TRACE_SCOPE(tracing::RocketRenderCompiledGeometry);
	GR_DEBUG_SCOPE("libRocket::RenderCompiledGeometry");

	auto geom = reinterpret_cast<CompiledGeometry*>(geometry);

	auto bitmap = -1;
	if (geom->texture != nullptr) {
		if (geom->texture->is_animation) {
			bitmap = geom->texture->animation.bitmap_id;
		} else {
			bitmap = geom->texture->bm_handle;
		}
	}

	renderGeometry(geom->vertex_buffer, geom->index_buffer, geom->num_elements, bitmap, translation);
}
void RocketRenderingInterface::ReleaseCompiledGeometry(CompiledGeometryHandle geometry)
{
	GR_DEBUG_SCOPE("libRocket::ReleaseCompiledGeometry");

	auto geom = reinterpret_cast<CompiledGeometry*>(geometry);

	gr_delete_buffer(geom->vertex_buffer);
	gr_delete_buffer(geom->index_buffer);

	delete geom;
}
void RocketRenderingInterface::EnableScissorRegion(bool enable)
{
	GR_DEBUG_SCOPE("libRocket::EnableScissorRegion");
	if (!enable) {
		// Disable clipping if requested
		gr_reset_clip();
	}

	// If clipping is active then SetScissorRegion will be called next
	clipping_active = enable;
}
void RocketRenderingInterface::SetScissorRegion(int x, int y, int width, int height)
{
	GR_DEBUG_SCOPE("libRocket::SetScissorRegion");
	if (clipping_active) {
		gr_set_clip((int)(x + renderOffset.x), (int)(y + renderOffset.y), width, height, GR_RESIZE_NONE);
	}
}
bool RocketRenderingInterface::LoadTexture(TextureHandle& texture_handle, Vector2i& texture_dimensions,
                                           const String& source)
{
	TRACE_SCOPE(tracing::RocketLoadTexture);
	GR_DEBUG_SCOPE("libRocket::LoadTexture");
	SCP_string filename;
	int dir_type;
	if (!RocketFileInterface::getCFilePath(source, filename, dir_type)) {
		return false;
	}

	auto period_pos = filename.rfind('.');
	if (period_pos != SCP_string::npos) {
		filename = filename.substr(0, period_pos);
	}

	std::unique_ptr<Texture> tex(new Texture());
	// If there is a file that ends with an animation extension, try to load that
	if (generic_anim_init_and_stream(&tex->animation, filename.c_str(), BM_TYPE_NONE, true) == 0) {
		tex->is_animation = true;

		texture_dimensions.x = tex->animation.width;
		texture_dimensions.y = tex->animation.height;
	}

	if (!tex->is_animation) {
		// Try to load as standalone image instead
		auto id = bm_load_either(filename.c_str(), nullptr, nullptr, nullptr, false, dir_type);
		if (id < 0) {
			return false;
		}

		int w, h;
		bm_get_info(id, &w, &h);

		texture_dimensions.x = w;
		texture_dimensions.y = h;

		tex->bm_handle = id;
	}

	// We give the pointer to libRocket now so we release it from our unique ptr
	texture_handle = get_texture_handle(tex.release());
	return true;
}
bool RocketRenderingInterface::GenerateTexture(TextureHandle& texture_handle, const Rocket::Core::byte* source,
                                               const Vector2i& source_dimensions)
{
	TRACE_SCOPE(tracing::RocketGenerateTexture);
	GR_DEBUG_SCOPE("libRocket::GenerateTexture");
	auto size = (size_t)(source_dimensions.x * source_dimensions.y * 4); // RGBA format

	std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
	memcpy(buffer.get(), source, size);

	auto id = bm_create(32, source_dimensions.x, source_dimensions.y, buffer.get());
	if (id < 0) {
		return false;
	}

	auto* tex      = new Texture();
	tex->bm_handle = id;
	tex->data      = std::move(buffer);

	texture_handle = get_texture_handle(tex);

	return true;
}
void RocketRenderingInterface::ReleaseTexture(TextureHandle texture)
{
	GR_DEBUG_SCOPE("libRocket::ReleaseTexture");
	Assertion(texture, "Invalid texture handle!");

	auto tex = get_texture(texture);

	if (tex->is_animation) {
		generic_anim_unload(&tex->animation);
	} else {
		bm_release(tex->bm_handle);
	}
	delete tex;
}
void RocketRenderingInterface::RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices,
                                              TextureHandle texture, const Vector2f& translation)
{
	TRACE_SCOPE(tracing::RocketRenderGeometry);
	GR_DEBUG_SCOPE("libRocket::RenderGeometry");
	gr_update_buffer_data(vertex_stream_buffer, sizeof(*vertices) * num_vertices, vertices);
	gr_update_buffer_data(index_stream_buffer, sizeof(*indices) * num_indices, indices);

	int bitmap;
	if (texture == 0) {
		bitmap = -1;
	} else {
		if (get_texture(texture)->is_animation) {
			bitmap = get_texture(texture)->animation.bitmap_id;
		} else {
			bitmap = get_texture(texture)->bm_handle;
		}
	}

	renderGeometry(vertex_stream_buffer, index_stream_buffer, num_indices, bitmap, translation);
}

void RocketRenderingInterface::setRenderOffset(const vec2d& render_offset) { renderOffset = render_offset; }
float RocketRenderingInterface::GetPixelsPerInch()
{
#if SDL_VERSION_ATLEAST(2, 0, 4)
	auto display = os_config_read_uint("Video", "Display", 0);
	float ddpi;
	if (SDL_GetDisplayDPI(display, &ddpi, nullptr, nullptr) != 0) {
		mprintf(("Failed to determine display DPI: %s\n", SDL_GetError()));
		return 96.f;
	}

	// Diagonal DPI should be accurate enough
	return ddpi;
#else
	// return a default value
	return 96.f;
#endif
}

void RocketRenderingInterface::renderGeometry(gr_buffer_handle vertex_buffer,
	gr_buffer_handle index_buffer,
	int num_elements,
	int bitmap,
	const Rocket::Core::Vector2f& translation)
{
	interface_material material;

	vec2d transl;
	transl.x = translation.x + renderOffset.x;
	transl.y = translation.y + renderOffset.y;

	material_set_rocket_interface(&material, bitmap, transl, horizontal_swipe_offset);

	gr_render_rocket_primitives(&material, PRIM_TYPE_TRIS, &layout, num_elements, vertex_buffer, index_buffer);
}
int RocketRenderingInterface::getBitmapNum(Rocket::Core::TextureHandle handle)
{
	int bitmap;
	if (handle == 0) {
		bitmap = -1;
	} else {
		bitmap = get_texture(handle)->bm_handle;
	}

	return bitmap;
}
void RocketRenderingInterface::advanceAnimation(Rocket::Core::TextureHandle handle, float advanceTime)
{
	Assertion(handle != 0, "Invalid handle for setAnimationFrame");
	Assertion(get_texture(handle)->is_animation, "Tried to use advanceAnimation with a non-animation!");

	auto tex = get_texture(handle);

	generic_extras extras;
	extras.draw = false; // We only want to advance the time but not actually render the animation

	generic_anim_render(&tex->animation, advanceTime, -1, -1, false, &extras);
}
void RocketRenderingInterface::setHorizontalSwipeOffset(float value) {
	horizontal_swipe_offset = value;
}

} // namespace scpui
