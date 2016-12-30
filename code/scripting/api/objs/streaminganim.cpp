//
//

#include "streaminganim.h"
#include "freespace.h"


namespace scripting {
namespace api {

bool streaminganim_h::IsValid() {
	return (ga.num_frames > 0);
}
streaminganim_h::streaminganim_h(const char* filename) {
	generic_anim_init(&ga, filename);
}

//**********HANDLE: streamingAnimation
ADE_OBJ(l_streaminganim, streaminganim_h, "streaminganim", "Streaming Animation handle");

ADE_FUNC(__gc, l_streaminganim, NULL, "Auto-deletes streaming animation", NULL, NULL)
{
	// NOTE: very similar to the l_streaminganim ADE_FUNC unload
	streaminganim_h* sah;

	if(!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	// don't bother to check if valid before unloading
	// generic_anim_unload has safety checks
	generic_anim_unload(&sah->ga);

	return ADE_RETURN_NIL;
}

ADE_VIRTVAR(Loop, l_streaminganim, "[boolean loop]", "Make the streaming animation loop.", "boolean", "Is the animation looping, or nil if anim invalid")
{
	streaminganim_h* sah;
	bool loop = false;

	if(!ade_get_args(L, "o|b", l_streaminganim.GetPtr(&sah), &loop))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (loop == false)
			sah->ga.direction |= GENERIC_ANIM_DIRECTION_NOLOOP;
		else
			sah->ga.direction &= ~GENERIC_ANIM_DIRECTION_NOLOOP;
	}

	return ((sah->ga.direction & GENERIC_ANIM_DIRECTION_NOLOOP) == false ? ADE_RETURN_TRUE : ADE_RETURN_FALSE);
}

ADE_VIRTVAR(Pause, l_streaminganim, "[boolean pause]", "Pause the streaming animation.", "boolean", "Is the animation paused, or nil if anim invalid")
{
	streaminganim_h* sah;
	bool pause = false;

	if(!ade_get_args(L, "o|b", l_streaminganim.GetPtr(&sah), &pause))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (pause == true)
			sah->ga.direction |= GENERIC_ANIM_DIRECTION_PAUSED;
		else
			sah->ga.direction &= ~GENERIC_ANIM_DIRECTION_PAUSED;
	}

	return ((sah->ga.direction & GENERIC_ANIM_DIRECTION_PAUSED) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE);
}

ADE_VIRTVAR(Reverse, l_streaminganim, "[boolean reverse]", "Make the streaming animation play in reverse.", "boolean", "Is the animation playing in reverse, or nil if anim invalid")
{
	streaminganim_h* sah;
	bool reverse = false;

	if(!ade_get_args(L, "o|b", l_streaminganim.GetPtr(&sah), &reverse))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (reverse == true)
			sah->ga.direction |= GENERIC_ANIM_DIRECTION_BACKWARDS;
		else
			sah->ga.direction &= ~GENERIC_ANIM_DIRECTION_BACKWARDS;
	}

	return ((sah->ga.direction & GENERIC_ANIM_DIRECTION_BACKWARDS) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE);
}

ADE_FUNC(getFilename, l_streaminganim, NULL, "Get the filename of the animation", "string", "Filename or nil if invalid")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "s", sah->ga.filename);
}

ADE_FUNC(getFrameCount, l_streaminganim, NULL, "Get the number of frames in the animation.", "integer", "Total number of frames")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", sah->ga.num_frames);
}

ADE_FUNC(getFrameIndex, l_streaminganim, NULL, "Get the current frame index of the animation", "integer", "Current frame index")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	int cframe = sah->ga.current_frame;
	if (cframe < 0 || cframe >= sah->ga.num_frames) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", ++cframe); // C++ to LUA array index
}

ADE_FUNC(getHeight, l_streaminganim, NULL, "Get the height of the animation in pixels", "integer", "Height or nil if invalid")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", sah->ga.height);
}

ADE_FUNC(getWidth, l_streaminganim, NULL, "Get the width of the animation in pixels", "integer", "Width or nil if invalid")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", sah->ga.width);
}

ADE_FUNC(isValid, l_streaminganim, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	streaminganim_h* sah;
	if(!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	return sah->IsValid();
}

ADE_FUNC(preload, l_streaminganim, NULL, "Load all apng animations into memory, enabling apng frame cache if not already enabled", "boolean", "true if preload was successful, nil if a syntax/type error occurs")
{
	streaminganim_h* sah;
	if(!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if (sah->ga.type != BM_TYPE_PNG)
		return ADE_RETURN_NIL;

	sah->ga.png.anim->preload();

	return ADE_RETURN_TRUE;
}

ADE_FUNC(process, l_streaminganim, "[int x1, int y1, int x2, int y2, float u0, float v0, float u1, float v1, float alpha, boolean draw]",
		 "Processes a streaming animation, including selecting the correct frame & drawing it.",
		 "boolean", "True if processing was successful, otherwise nil")
{
	streaminganim_h* sah;
	generic_extras ge;
	int x1 = 0, y1 = 0;
	int x2 = INT_MAX, y2 = INT_MAX;

	if(!ade_get_args(L, "o|iiiifffffb", l_streaminganim.GetPtr(&sah),
					 &x1, &y1, &x2, &y2,
					 &ge.u0, &ge.v0, &ge.u1, &ge.v1, &ge.alpha,
					 &ge.draw))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	ge.width = sah->ga.width;
	ge.height = sah->ga.height;
	if (x2 != INT_MAX) ge.width = x2-x1;
	if (y2 != INT_MAX) ge.height = y2-y1;

	// note; generic_anim_render will default to GR_RESIZE_NONE when ge is provided
	generic_anim_render(&sah->ga, flFrametime, x1, y1, false, &ge);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(reset, l_streaminganim, "[none]", "Reset a streaming animation back to its 1st frame", "boolean", "True if successful, otherwise nil")
{
	streaminganim_h* sah;

	if(!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	sah->ga.png.anim->goto_start();
	sah->ga.current_frame = 0;
	sah->ga.anim_time = 0.0f;
	sah->ga.png.previous_frame_time = 0.0f;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(timeLeft, l_streaminganim, NULL, "Get the amount of time left in the animation, in seconds", "float", "Time left in secs or nil if invalid")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	float timeLeft = 0.0f;
	if ((sah->ga.direction & GENERIC_ANIM_DIRECTION_BACKWARDS) == true)
		timeLeft = sah->ga.anim_time;
	else
		timeLeft = sah->ga.total_time - sah->ga.anim_time;

	return ade_set_args(L, "f", timeLeft);
}

ADE_FUNC(unload, l_streaminganim, NULL, "Unloads a streaming animation from memory", NULL, NULL)
{
	streaminganim_h* sah;

	if(!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	// don't bother to check if valid before unloading
	// generic_anim_unload has safety checks
	generic_anim_unload(&sah->ga);

	return ADE_RETURN_TRUE;
}

}
}
