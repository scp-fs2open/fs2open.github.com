//
//

#include "texture.h"
#include "bmpman/bmpman.h"
#define BMPMAN_INTERNAL
#include "bmpman/bm_internal.h"


namespace scripting {
namespace api {


//**********HANDLE: Texture
ADE_OBJ(l_Texture, int, "texture", "Texture handle");

//WMC - int should NEVER EVER be an invalid handle. Return Nil instead. Nil FTW.

ADE_FUNC(__gc, l_Texture, NULL, "Auto-deletes texture", NULL, NULL)
{
	int idx;

	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ADE_RETURN_NIL;

	// Note: due to some unknown reason, in some circumstances this function
	// might get called even for handles to bitmaps which are actually still in
	// use, and in order to prevent that we want to double-check the load count
	// here before unloading the bitmap. -zookeeper
	if(idx > -1 && bm_is_valid(idx) && bm_bitmaps[bm_get_cache_slot(idx, 0)].load_count < 1)
		bm_release(idx);

	return ADE_RETURN_NIL;
}

ADE_FUNC(__eq, l_Texture, "texture, texture", "Checks if two texture handles refer to the same texture", "boolean", "True if textures are equal")
{
	int idx,idx2;

	if(!ade_get_args(L, "oo", l_Texture.Get(&idx), l_Texture.Get(&idx2)))
		return ADE_RETURN_NIL;

	if(idx == idx2)
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
	int idx;
	int frame=-1;
	int newframe=-1;	//WMC - Ignore for now
	if(!ade_get_args(L, "oi|i", l_Texture.Get(&idx), &frame, &newframe))
		return ade_set_error(L, "o", l_Texture.Set(-1));

	if(frame < 1)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	//Get me some info
	int num=-1;
	int first=-1;
	first = bm_get_info(idx, NULL, NULL, NULL, &num);

	//Check it's a valid one
	if(first < 0 || frame > num)
		return ade_set_error(L, "o", l_Texture.Set(-1));

	frame--; //Lua->FS2

	//Get actual texture handle
	frame = first + frame;

	return ade_set_args(L, "o", l_Texture.Set(frame));
}

ADE_FUNC(isValid, l_Texture, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	int idx;
	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", bm_is_valid(idx));
}

ADE_FUNC(unload, l_Texture, NULL, "Unloads a texture from memory", NULL, NULL)
{
	int *idx;

	if(!ade_get_args(L, "o", l_Texture.GetPtr(&idx)))
		return ADE_RETURN_NIL;

	if(!bm_is_valid(*idx))
		return ADE_RETURN_NIL;

	bm_release(*idx);

	//WMC - invalidate this handle
	*idx = -1;

	return ADE_RETURN_NIL;
}

ADE_FUNC(getFilename, l_Texture, NULL, "Returns filename for texture", "string", "Filename, or empty string if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ade_set_error(L, "s", "");

	if(!bm_is_valid(idx))
		return ade_set_error(L, "s", "");

	return ade_set_args(L, "s", bm_get_filename(idx));
}

ADE_FUNC(getWidth, l_Texture, NULL, "Gets texture width", "number", "Texture width, or 0 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ade_set_error(L, "i", 0);

	if(!bm_is_valid(idx))
		return ade_set_error(L, "i", 0);

	int w = -1;

	if(bm_get_info(idx, &w) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", w);
}

ADE_FUNC(getHeight, l_Texture, NULL, "Gets texture height", "number", "Texture height, or 0 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ade_set_error(L, "i", 0);

	if(!bm_is_valid(idx))
		return ade_set_error(L, "i", 0);

	int h=-1;

	if(bm_get_info(idx, NULL, &h) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", h);
}

ADE_FUNC(getFPS, l_Texture, NULL,"Gets frames-per-second of texture", "number", "Texture FPS, or 0 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ade_set_error(L, "i", 0);

	if(!bm_is_valid(idx))
		return ade_set_error(L, "i", 0);

	int fps=-1;

	if(bm_get_info(idx, NULL, NULL, NULL, NULL, &fps) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", fps);
}

ADE_FUNC(getFramesLeft, l_Texture, NULL, "Gets number of frames left, from handle's position in animation", "number", "Frames left, or 0 if handle is invalid")
{
	int idx;
	if(!ade_get_args(L, "o", l_Texture.Get(&idx)))
		return ADE_RETURN_NIL;

	if(!bm_is_valid(idx))
		return ADE_RETURN_NIL;

	int num=-1;

	if(bm_get_info(idx, NULL, NULL, NULL, &num) < 0)
		return ade_set_error(L, "i", 0);

	return ade_set_args(L, "i", num);
}

ADE_FUNC(getFrame, l_Texture, "number Elapsed time (secs), [boolean Loop]",
		 "Get the frame number from the elapsed time of the animation<br>"
			 "The 1st argument is the time that has elapsed since the animation started<br>"
			 "If 2nd argument is set to true, the animation is expected to loop when the elapsed time exceeds the duration of a single playback",
		 "integer",
		 "Frame number")
{
	int idx, frame = 0;
	float elapsed_time;
	bool loop = false;

	if (!ade_get_args(L, "of|b", l_Texture.Get(&idx), &elapsed_time, &loop))
		return ADE_RETURN_NIL;

	if (!bm_is_valid(idx))
		return ADE_RETURN_NIL;

	frame = bm_get_anim_frame(idx, elapsed_time, 0.0f, loop);
	frame++;  // C++ to LUA array idx

	return ade_set_args(L, "i", frame);
}


}
}
