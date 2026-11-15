#pragma once

// Near-field depth of field. The focus is at infinity, as for an eye in open space, so only what is very
// near the camera blurs, for example the hull of a ship that the player flies past. The blur radius is the
// thin-lens circle of confusion K * (1/z - 1/N) with K = max_blur * N: no blur at the distance N, the max
// blur at N / 2 and nearer. The cockpit and the sky stay sharp. The pass is OpenGL only.

namespace graphics {

// Blur radius in pixels at view z (metres). This is the reference for coc_of_depth() in dof-common.sdr, which
// repeats the formula because GLSL has no unit tests here; a change to one must change the other.
float dof_coc(float z, float near_dist, float max_blur);

// The max blur option is in pixels at 1080 lines and the default field of view. A real lens gives a blur
// in proportion to the focal length in pixels, so the blur grows with the resolution and when the view
// zooms in. Returns the factor for a frame of screen_h lines and a vertical field of view of fov_y radians
// (Proj_fov, which is the zoom times PROJ_FOV_FACTOR).
float dof_pixel_scale(float screen_h, float fov_y);

constexpr int DOF_MAX_SAMPLES = 512; // the shader's MAX_SAMPLES

// Side of a tile of the largest near CoC, in pixels (DOF_TILE in dof-common.sdr).
constexpr int DOF_TILE_SIZE = 16;

// How many tiles away the neighbour pass looks (DOF_TILE_REACH in dof-common.sdr). The max blur on screen is
// clamped to this reach, so that the search always finds every near disc that can reach a pixel.
constexpr int DOF_TILE_REACH = 8;

// Gather taps for a disc of this radius: the density of 96 taps on 16 px, at least 32, at most DOF_MAX_SAMPLES.
// Past about 37 px the cap makes the gaps between the taps grow and the blur noisier. This is the reference for
// the tap count in dof-f.sdr, which computes it per pixel from the sampled radius.
int dof_samples(float radius);

// The renderer's view of the options for one frame.
struct dof_frame {
	float near_dist; // metres
	float max_blur;  // pixels on this screen
};

// False when the option is off.
bool dof_get_frame(dof_frame* out);

// The OpenGL pass is the only implementation. Records that requirement for every DoF option, so the
// options dialog greys them out on other renderers and a launcher can say why.
void dof_set_requirements();

} // namespace graphics
