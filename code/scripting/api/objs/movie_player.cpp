
#include "movie_player.h"
#include "time_obj.h"

namespace scripting {
namespace api {

movie_player_h::movie_player_h() = default;
movie_player_h::movie_player_h(std::unique_ptr<cutscene::Player>&& player) : _player(std::move(player)) {}
bool movie_player_h::isValid() const { return (bool)_player; }
cutscene::Player* movie_player_h::player() { return _player.get(); }

ADE_OBJ(l_MoviePlayer, movie_player_h, "movie_player", "A movie player instance");

ADE_VIRTVAR(Width, l_MoviePlayer, "number", "Determines the width in pixels of this movie <b>Read-only</b>", "number",
            "The width of the movie or -1 if handle is invalid")
{
	movie_player_h* ph = nullptr;
	if (!ade_get_args(L, "o", l_MoviePlayer.GetPtr(&ph)))
		return ade_set_args(L, "i", -1);

	if (ph == nullptr || !ph->isValid()) {
		return ade_set_args(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Tried to modify read-only member!");
	}

	return ade_set_args(L, "i", (int)ph->player()->getMovieProperties().size.width);
}

ADE_VIRTVAR(Height, l_MoviePlayer, "number", "Determines the height in pixels of this movie <b>Read-only</b>", "number",
            "The height of the movie or -1 if handle is invalid")
{
	movie_player_h* ph = nullptr;
	if (!ade_get_args(L, "o", l_MoviePlayer.GetPtr(&ph)))
		return ade_set_args(L, "i", -1);

	if (ph == nullptr || !ph->isValid()) {
		return ade_set_args(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Tried to modify read-only member!");
	}

	return ade_set_args(L, "i", (int)ph->player()->getMovieProperties().size.height);
}

ADE_VIRTVAR(FPS, l_MoviePlayer, "number", "Determines the frames per second of this movie <b>Read-only</b>", "number",
            "The FPS of the movie or -1 if handle is invalid")
{
	movie_player_h* ph = nullptr;
	if (!ade_get_args(L, "o", l_MoviePlayer.GetPtr(&ph)))
		return ade_set_args(L, "i", -1);

	if (ph == nullptr || !ph->isValid()) {
		return ade_set_args(L, "i", -1);
	}

	if (ADE_SETTING_VAR) {
		LuaError(L, "Tried to modify read-only member!");
	}

	return ade_set_args(L, "f", ph->player()->getMovieProperties().fps);
}

ADE_FUNC(update, l_MoviePlayer, "timespan step_time",
         "Updates the current state of the movie and moves the internal timer forward by the specified time span.",
         "boolean", "true if there is more to display, false otherwise")
{
	movie_player_h* ph = nullptr;
	int64_t time_diff  = 0;
	if (!ade_get_args(L, "oo", l_MoviePlayer.GetPtr(&ph), l_TimeSpan.Get(&time_diff)))
		return ADE_RETURN_FALSE;

	if (ph == nullptr || !ph->isValid()) {
		return ADE_RETURN_FALSE;
	}

	auto ret = ph->player()->update(static_cast<uint64_t>(time_diff / 1000));

	return ade_set_args(L, "b", ret);
}

ADE_FUNC(isPlaybackReady, l_MoviePlayer, nullptr,
         "Determines if the player is ready to display the movie. Since the movie frames are loaded asynchronously "
         "there is a short delay between the creation of a player and when it is possible to start displaying the "
         "movie. This function can be used to determine if playback is possible at the moment.",
         "boolean", "true if playback is ready, false otherwise")
{
	movie_player_h* ph = nullptr;
	if (!ade_get_args(L, "o", l_MoviePlayer.GetPtr(&ph)))
		return ADE_RETURN_FALSE;

	if (ph == nullptr || !ph->isValid()) {
		return ADE_RETURN_FALSE;
	}

	auto ret = ph->player()->isPlaybackReady();

	return ade_set_args(L, "b", ret);
}

ADE_FUNC(drawMovie, l_MoviePlayer, "number x1, number y1, [number x2, number y2]",
         "Draws the current frame of the movie at the specified coordinates.", nullptr, "Returns nothing")
{
	movie_player_h* ph = nullptr;
	float x1;
	float y1;
	float x2 = -1.f;
	float y2 = -1.f;
	float alpha = 1.f;

	if (!ade_get_args(L, "off|fff", l_MoviePlayer.GetPtr(&ph), &x1, &y1, &x2, &y2, &alpha))
		return ADE_RETURN_NIL;

	if (ph == nullptr || !ph->isValid()) {
		return ADE_RETURN_NIL;
	}

	auto& props = ph->player()->getMovieProperties();

	// Use equality here to check if it has been changed
	if (x2 == -1.f) {
		x2 = x1 + props.size.width;
	}
	if (y2 == -1.f) {
		y2 = y1 + props.size.height;
	}

	ph->player()->draw(x1, y1, x2, y2, alpha);

	return ADE_RETURN_NIL;
}

ADE_FUNC(stop, l_MoviePlayer, nullptr,
         "Explicitly stops playback. This function should be called when the player isn't needed anymore to free up "
         "some resources.",
         nullptr, "Returns nothing")
{
	movie_player_h* ph = nullptr;
	if (!ade_get_args(L, "o", l_MoviePlayer.GetPtr(&ph)))
		return ADE_RETURN_NIL;

	if (ph == nullptr || !ph->isValid()) {
		return ADE_RETURN_NIL;
	}

	ph->player()->stopPlayback();

	return ADE_RETURN_NIL;
}

ADE_FUNC(isValid, l_MoviePlayer, nullptr, "Determines if this handle is valid", "boolean",
         "true if valid, false otherwise")
{
	movie_player_h* ph = nullptr;
	if (!ade_get_args(L, "o", l_MoviePlayer.GetPtr(&ph)))
		return ADE_RETURN_FALSE;

	if (ph == nullptr) {
		return ADE_RETURN_FALSE;
	}

	if (!ph->isValid()) {
		return ADE_RETURN_FALSE;
	}

	return ADE_RETURN_TRUE;
}

} // namespace api
} // namespace scripting
