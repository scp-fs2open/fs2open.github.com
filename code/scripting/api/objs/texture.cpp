//
//

#include "texture.h"
#include "bmpman/bmpman.h"
#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"

namespace scripting {
namespace api {

texture_h::texture_h() = default;
texture_h::texture_h(int bm, bool refcount, int parent_bm) : handle(bm), parent_handle(parent_bm) {
	if (refcount && isValid())
		bm_get_entry(parent_bm != -1 ? parent_bm : bm)->load_count++;
}
texture_h::~texture_h()
{
	if (!isValid()) {
		// There is nothing to release
		return;
	}

	//Note: We previously checked the load count here to make sure we don't unload the texture if it is in use otherwise.
	//That is dangerous, as if anything else locks a lua-created texture while lua loses scope of the texture, we leak
	//the texture as the load_count from the creation in lua will never decrease. To fix this, we need to properly refcount
	//the textures using load_count. Anything that creates a texture object must also increase load count, unless it is
	//created in a way that already increases load_count (like bm_load). That way, a texture going out of scope needs to be
	//released and is safed against memleaks. -Lafiel
	//Note 2: Some textures, notably subframes of animations, aren't first-class bmpman citizens and mustn't be released directly.
	//Otherwise it is possible (and has been observed in practice) that the parent texture get's deleted before all dependent objects,
	//causing this release of the dependent object to clear unrelated textures that were assigned the previously freed spots.
	//So instead, both lock and later unlock the parent texture rather than this child texture. -Lafiel
	bm_release(parent_handle != -1 ? parent_handle : handle);
}
bool texture_h::isValid() const { return bm_is_valid(handle) != 0; }

texture_h::texture_h(texture_h&& other) noexcept {
	*this = std::move(other);
}
texture_h& texture_h::operator=(texture_h&& other) noexcept {
	if (this != &other) {
		std::swap(handle, other.handle);
		std::swap(parent_handle, other.parent_handle);
	}
	return *this;
}

//**********HANDLE: Texture
ADE_OBJ(l_Texture, texture_h, "texture", "Texture handle");

//WMC - int should NEVER EVER be an invalid handle. Return Nil instead. Nil FTW.

ADE_FUNC(__eq, l_Texture, "texture, texture", "Checks if two texture handles refer to the same texture", "boolean", "True if textures are equal")
{
	texture_h* th;
	texture_h* th2;

	if (!ade_get_args(L, "oo", l_Texture.GetPtr(&th), l_Texture.GetPtr(&th2)))
		return ADE_RETURN_NIL;

	if (th->handle == th2->handle)
		return ADE_RETURN_TRUE;

	return ADE_RETURN_FALSE;
}

ADE_INDEXER(l_Texture, "number",
			"Returns texture handle to specified frame number in current texture's animation."
				"This means that [1] will always return the first frame in an animation, no matter what frame an animation is."
				"You cannot change a texture animation frame.",
			"texture",
			"Texture handle, or invalid texture handle if index is invalid")
{
	texture_h* th;
	int frame=-1;
	int newframe=-1;	//WMC - Ignore for now
	if (!ade_get_args(L, "oi|i", l_Texture.GetPtr(&th), &frame, &newframe))
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if(frame < 1)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	if (!th->isValid()) {
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));
	}

	//Get me some info
	int num=-1;
	int first = bm_get_info(th->handle, nullptr, nullptr, nullptr, &num);

	//Check it's a valid one
	if(first < 0 || frame > num)
		return ade_set_error(L, "o", l_Texture.Set(texture_h()));

	frame--; //Lua->FS2

	//Get actual texture handle
	frame = first + frame;

	return ade_set_args(L, "o", l_Texture.Set(texture_h(frame, true, first)));
}

ADE_FUNC(isValid, l_Texture, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	texture_h* th;
	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", th->isValid());
}

ADE_FUNC(unload, l_Texture, NULL, "Unloads a texture from memory", NULL, NULL)
{
	texture_h* th;

	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ADE_RETURN_NIL;

	if (!th->isValid())
		return ADE_RETURN_NIL;

	bm_release(th->handle);

	//WMC - invalidate this handle
	th->handle = -1;

	return ADE_RETURN_NIL;
}

ADE_FUNC(destroyRenderTarget, l_Texture, nullptr, "Destroys a texture's render target. Call this when done drawing to a texture, as it frees up resources.", nullptr, nullptr)
{
	texture_h* th;

	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ADE_RETURN_NIL;

	if (!th->isValid())
		return ADE_RETURN_NIL;

	if (!bm_is_render_target(th->handle)) {
		LuaError(L, "Tried to destroy a render target of a non-renderable texture!");
		return ADE_RETURN_NIL;
	}

	bm_release_rendertarget(th->handle);

	th->handle = -1;

	return ADE_RETURN_NIL;
}

ADE_FUNC(getFilename, l_Texture, NULL, "Returns filename for texture", "string", "Filename, or empty string if handle is invalid")
{
	texture_h* th;
	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ade_set_error(L, "s", "");

	if (!th->isValid())
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", bm_get_filename(th->handle));
}

ADE_FUNC(getWidth, l_Texture, NULL, "Gets texture width", "number", "Texture width, or 0 if handle is invalid")
{
	texture_h* th;
	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ade_set_error(L, "i", 0);

	if (!th->isValid())
		return ade_set_error(L, "i", 0);

	int w = -1;

	if (bm_get_info(th->handle, &w) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", w);
}

ADE_FUNC(getHeight, l_Texture, NULL, "Gets texture height", "number", "Texture height, or 0 if handle is invalid")
{
	texture_h* th;
	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ade_set_error(L, "i", 0);

	if (!th->isValid())
		return ade_set_error(L, "i", 0);

	int h=-1;

	if (bm_get_info(th->handle, nullptr, &h) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", h);
}

ADE_FUNC(getFPS, l_Texture, NULL,"Gets frames-per-second of texture", "number", "Texture FPS, or 0 if handle is invalid")
{
	texture_h* th;
	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ade_set_error(L, "i", 0);

	if (!th->isValid())
		return ade_set_error(L, "i", 0);

	int fps=-1;

	if (bm_get_info(th->handle, nullptr, nullptr, nullptr, nullptr, &fps) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", fps);
}

ADE_FUNC(getFramesLeft, l_Texture, NULL, "Gets number of frames left, from handle's position in animation", "number", "Frames left, or 0 if handle is invalid")
{
	texture_h* th;
	if (!ade_get_args(L, "o", l_Texture.GetPtr(&th)))
		return ADE_RETURN_NIL;

	if (!th->isValid())
		return ADE_RETURN_NIL;

	int num=-1;

	if (bm_get_info(th->handle, nullptr, nullptr, nullptr, &num) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", num);
}

ADE_FUNC(getFrame, l_Texture, "number ElapsedTimeSeconds, [boolean Loop]",
         "Get the frame number from the elapsed time of the animation<br>"
         "The 1st argument is the time that has elapsed since the animation started<br>"
         "If 2nd argument is set to true, the animation is expected to loop when the elapsed time exceeds the duration "
         "of a single playback",
         "number", "Frame number")
{
	texture_h* th;
	int frame = 0;
	float elapsed_time;
	bool loop = false;

	if (!ade_get_args(L, "of|b", l_Texture.GetPtr(&th), &elapsed_time, &loop))
		return ADE_RETURN_NIL;

	if (!th->isValid())
		return ADE_RETURN_NIL;

	frame = bm_get_anim_frame(th->handle, elapsed_time, 0.0f, loop);
	frame++;  // C++ to LUA array idx

	return ade_set_args(L, "i", frame);
}

}
}
