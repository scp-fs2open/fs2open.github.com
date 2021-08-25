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
streaminganim_h::~streaminganim_h() {
	// don't bother to check if valid before unloading
	// generic_anim_unload has safety checks
	generic_anim_unload(&ga);
}
streaminganim_h::streaminganim_h(streaminganim_h&& other) noexcept {
	// Copy the other data over to us
	ga = other.ga;

	// Reset the other instance so that we own the only instance
	generic_anim_init(&other.ga, nullptr);
}
streaminganim_h& streaminganim_h::operator=(streaminganim_h&& other) noexcept {
	generic_anim_unload(&ga);

	ga = other.ga;

	// Reset the other instance so that we own the only instance
	generic_anim_init(&other.ga, nullptr);

	return *this;
}

//**********HANDLE: streamingAnimation
ADE_OBJ(l_streaminganim, streaminganim_h, "streaminganim", "Streaming Animation handle");

ADE_VIRTVAR(Loop, l_streaminganim, "boolean", "Make the streaming animation loop.", "boolean", "Is the animation looping, or nil if anim invalid")
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

ADE_VIRTVAR(Pause, l_streaminganim, "boolean", "Pause the streaming animation.", "boolean", "Is the animation paused, or nil if anim invalid")
{
	streaminganim_h* sah;
	bool pause = false;

	if(!ade_get_args(L, "o|b", l_streaminganim.GetPtr(&sah), &pause))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (pause)
			sah->ga.direction |= GENERIC_ANIM_DIRECTION_PAUSED;
		else
			sah->ga.direction &= ~GENERIC_ANIM_DIRECTION_PAUSED;
	}

	return ((sah->ga.direction & GENERIC_ANIM_DIRECTION_PAUSED) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE);
}

ADE_VIRTVAR(Reverse, l_streaminganim, "boolean", "Make the streaming animation play in reverse.", "boolean", "Is the animation playing in reverse, or nil if anim invalid")
{
	streaminganim_h* sah;
	bool reverse = false;

	if(!ade_get_args(L, "o|b", l_streaminganim.GetPtr(&sah), &reverse))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		if (reverse)
			sah->ga.direction |= GENERIC_ANIM_DIRECTION_BACKWARDS;
		else
			sah->ga.direction &= ~GENERIC_ANIM_DIRECTION_BACKWARDS;
	}

	return ((sah->ga.direction & GENERIC_ANIM_DIRECTION_BACKWARDS) ? ADE_RETURN_TRUE : ADE_RETURN_FALSE);
}

ADE_VIRTVAR(Grayscale, l_streaminganim, "boolean", "Whether the streaming animation is drawn as grayscale multiplied by the current color (the HUD method).", "boolean", "Boolean flag")
{
	streaminganim_h* sah;
	bool grayscale = false;

	if (!ade_get_args(L, "o|b", l_streaminganim.GetPtr(&sah), &grayscale))
		return ADE_RETURN_NIL;

	if (!sah->IsValid())
		return ADE_RETURN_NIL;

	if (ADE_SETTING_VAR) {
		LuaError(L, "This variable can only be set before the anim is loaded.");
	}

	return sah->ga.use_hud_color ? ADE_RETURN_TRUE : ADE_RETURN_FALSE;
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

ADE_FUNC(getFrameCount, l_streaminganim, nullptr, "Get the number of frames in the animation.", "number",
         "Total number of frames")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", sah->ga.num_frames);
}

ADE_FUNC(getFrameIndex, l_streaminganim, nullptr, "Get the current frame index of the animation", "number",
         "Current frame index")
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

ADE_FUNC(getHeight, l_streaminganim, nullptr, "Get the height of the animation in pixels", "number",
         "Height or nil if invalid")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	return ade_set_args(L, "i", sah->ga.height);
}

ADE_FUNC(getWidth, l_streaminganim, nullptr, "Get the width of the animation in pixels", "number",
         "Width or nil if invalid")
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

ADE_FUNC(process, l_streaminganim, "[number x1, number y1, number x2, number y2, number u0, number v0, number u1, number v1, number alpha, boolean draw]",
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

	if (sah->ga.use_hud_color)
	{
		float scale_x = (x2 != INT_MAX) ? i2fl(x2 - x1) / i2fl(sah->ga.width) : 1.0f;
		float scale_y = (y2 != INT_MAX) ? i2fl(y2 - y1) / i2fl(sah->ga.height) : 1.0f;
		int base_w = gr_screen.max_w;
		int base_h = gr_screen.max_h;
		gr_set_screen_scale(fl2ir(base_w / scale_x), fl2ir(base_h / scale_y));

		// generic_anim_render can't use generic_extras, says the function
		// but we can at least do scaling, as above
		generic_anim_render(&sah->ga, flFrametime, fl2ir(x1 / scale_x), fl2ir(y1 / scale_y), false);

		gr_reset_screen_scale();
	}
	else
	{
		ge.width = sah->ga.width;
		ge.height = sah->ga.height;
		if (x2 != INT_MAX) ge.width = x2 - x1;
		if (y2 != INT_MAX) ge.height = y2 - y1;

		// note; generic_anim_render will default to GR_RESIZE_NONE when ge is provided
		generic_anim_render(&sah->ga, flFrametime, x1, y1, false, &ge);
	}

	return ADE_RETURN_TRUE;
}

ADE_FUNC(reset, l_streaminganim, nullptr, "Reset a streaming animation back to its 1st frame", "boolean", "True if successful, otherwise nil")
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

ADE_FUNC(timeLeft, l_streaminganim, nullptr, "Get the amount of time left in the animation, in seconds", "number", "Time left in secs or nil if invalid")
{
	streaminganim_h* sah;

	if (!ade_get_args(L, "o", l_streaminganim.GetPtr(&sah)))
		return ADE_RETURN_NIL;

	if(!sah->IsValid())
		return ADE_RETURN_NIL;

	float timeLeft = 0.0f;
	if ((sah->ga.direction & GENERIC_ANIM_DIRECTION_BACKWARDS))
		timeLeft = sah->ga.anim_time;
	else
		timeLeft = sah->ga.total_time - sah->ga.anim_time;

	// there was an anim that completed with less than 1/1000 seconds left;
	// let's round under 1/200 seconds because nobody plays at 200 FPS
	if (timeLeft < 0.005)
		timeLeft = 0.0f;

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
