#pragma once
//
//

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/RenderInterface.h>

#pragma pop_macro("Assert")

#include <memory>

namespace scpui {

class RocketRenderingInterface : public Rocket::Core::RenderInterface {
	struct Texture {
		int handle    = -1;
		int frame_num = 0;
		std::unique_ptr<uint8_t[]> data;
	};

	struct CompiledGeometry {
		int vertex_buffer = -1;
		int index_buffer  = -1;
		int num_elements  = -1;

		Texture* texture = nullptr;
	};

	bool clipping_active = false;

	vec2d renderOffset;

	Texture* get_texture(Rocket::Core::TextureHandle handle);

	Rocket::Core::TextureHandle get_texture_handle(Texture* bitmap);

	void renderGeometry(int vertex_buffer, int index_buffer, int num_elements, int bitmap,
	                    const Rocket::Core::Vector2f& translation);

	int vertex_stream_buffer = -1;
	int index_stream_buffer  = -1;

	vertex_layout layout;

  public:
	RocketRenderingInterface();
	~RocketRenderingInterface() override;

	/**
	 * @brief Renders geometry from a memory buffer
	 *
	 * @details This uses an internal streaming buffer for storing the data. This is not as fast as using compiled
	 * geometry but needs to be supported for the debugger.
	 *
	 * @param vertices
	 * @param num_vertices
	 * @param indices
	 * @param num_indices
	 * @param texture
	 * @param translation
	 */
	void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices,
	                    Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation) override;

	/**
	 * @brief Compiles geometry to a GPU buffer
	 *
	 * @details This uses GPU buffers provided by the graphics interface and stores the data in them. The buffers are
	 * allocated as static so rendering them should be pretty fast.
	 *
	 * @param vertices
	 * @param num_vertices
	 * @param indices
	 * @param num_indices
	 * @param texture
	 * @return
	 */
	Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices,
	                                                     int num_indices, Rocket::Core::TextureHandle texture) override;

	/**
	 * @brief Renders compiled geometry
	 *
	 * @details Takes the geometry buffers in the specified handle and renders them with the specified translation.
	 *
	 * @param geometry
	 * @param translation
	 */
	void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry,
	                            const Rocket::Core::Vector2f& translation) override;

	/**
	 * @brief Releases the geometry buffers
	 *
	 * @details Frees the underlying geometry buffers.
	 *
	 * @param geometry
	 */
	void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry) override;

	void EnableScissorRegion(bool enable) override;

	void SetScissorRegion(int x, int y, int width, int height) override;

	bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions,
	                 const Rocket::Core::String& source) override;

	bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source,
	                     const Rocket::Core::Vector2i& source_dimensions) override;

	void ReleaseTexture(Rocket::Core::TextureHandle texture) override;

	/**
	 * @brief Sets the render offset of future drawing operations
	 *
	 * @details This allows to offset any subsequent rendering operations without changing the libRocket markup.
	 *
	 * @param render_offset
	 */
	void setRenderOffset(const vec2d& render_offset);

	float GetPixelsPerInch() override;

	/**
	 * @brief From a texture handle, get the bitmap number for bmpman
	 * @param handle The libRocket texture handle
	 * @return The bitmap number
	 */
	int getBitmapNum(Rocket::Core::TextureHandle handle);

	/**
	 * @brief Sets the animation frame that the TextureHandle will use when it's rendered
	 * @param handle The libRocket texture handle to modify
	 * @param frame The animation frame (0-based for the first frame)
	 */
	void setAnimationFrame(Rocket::Core::TextureHandle handle, int frame);
};

} // namespace scpui
