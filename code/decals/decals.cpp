#include <nebula/neb.h>
#include <utility>
#include "decals/decals.h"

#include "graphics/2d.h"
#include "graphics/decal_draw_list.h"
#include "graphics/util/uniform_structs.h"
#include "parse/parselo.h"
#include "tracing/tracing.h"
#include "ship/ship.h"

namespace decals {

DecalDefinition::DecalDefinition(SCP_string name) : _name(std::move(name)) {
}

DecalDefinition::~DecalDefinition() {
	if (_diffuseBitmap >= 0) {
		bm_release(_diffuseBitmap);
	}
	if (_glowBitmap >= 0) {
		bm_release(_glowBitmap);
	}
	if (_normalBitmap >= 0) {
		bm_release(_normalBitmap);
	}
}

DecalDefinition::DecalDefinition(DecalDefinition&& other) noexcept {
	*this = std::move(other); // Use operator implementation
}

DecalDefinition& DecalDefinition::operator=(DecalDefinition&& other) noexcept {
	std::swap(_name, other._name);

	std::swap(_diffuseFilename, other._diffuseFilename);
	std::swap(_glowFilename, other._glowFilename);
	std::swap(_normalMapFilename, other._normalMapFilename);

	std::swap(_diffuseBitmap, other._diffuseBitmap);
	std::swap(_glowBitmap, other._glowBitmap);
	std::swap(_normalBitmap, other._normalBitmap);

	std::swap(_loopDiffuse, other._loopDiffuse);
	std::swap(_loopGlow, other._loopGlow);
	std::swap(_loopNormal, other._loopNormal);

	return *this;
}

void DecalDefinition::parse() {
	if (optional_string("+Diffuse:")) {
		stuff_string(_diffuseFilename, F_FILESPEC);

		if (!bm_validate_filename(_diffuseFilename, true, true)) {
			error_display(0, "Animation '%s' is not valid!", _diffuseFilename.c_str());
			_diffuseFilename = "";
		}

		if (optional_string("+Loop:")) {
			stuff_boolean(&_loopDiffuse);
		}
	}
	if (optional_string("+Glow:")) {
		stuff_string(_glowFilename, F_FILESPEC);

		if (!bm_validate_filename(_glowFilename, true, true)) {
			error_display(0, "Animation '%s' is not valid!", _glowFilename.c_str());
			_glowFilename = "";
		}

		if (optional_string("+Loop:")) {
			stuff_boolean(&_loopGlow);
		}
	}
	if (optional_string("+Normal:")) {
		stuff_string(_normalMapFilename, F_FILESPEC);

		if (!bm_validate_filename(_normalMapFilename, true, true)) {
			error_display(0, "Animation '%s' is not valid!", _normalMapFilename.c_str());
			_normalMapFilename = "";
		}

		if (optional_string("+Loop:")) {
			stuff_boolean(&_loopNormal);
		}
	}
}

void DecalDefinition::loadBitmaps() {
	if (_diffuseBitmap == -1 && VALID_FNAME(_diffuseFilename)) {
		_diffuseBitmap = bm_load_either(_diffuseFilename.c_str());
		if (_diffuseBitmap == -1) {
			Warning(LOCATION,
					"Bitmap '%s' failed to load for decal definition %s!",
					_diffuseFilename.c_str(),
					_name.c_str());
		}
	}
	if (_glowBitmap == -1 && VALID_FNAME(_glowFilename)) {
		_glowBitmap = bm_load_either(_glowFilename.c_str());
		if (_glowBitmap == -1) {
			Warning(LOCATION,
					"Bitmap '%s' failed to load for decal definition %s!",
					_glowFilename.c_str(),
					_name.c_str());
		}
	}
	if (_normalBitmap == -1 && VALID_FNAME(_normalMapFilename)) {
		_normalBitmap = bm_load_either(_normalMapFilename.c_str());
		if (_normalBitmap == -1) {
			Warning(LOCATION,
					"Bitmap '%s' failed to load for decal definition %s!",
					_normalMapFilename.c_str(),
					_name.c_str());
		}
	}
}

void DecalDefinition::pageIn() {
	if (_diffuseBitmap >= 0) {
		bm_page_in_texture(_diffuseBitmap);
	}
	if (_glowBitmap >= 0) {
		bm_page_in_texture(_glowBitmap);
	}
	if (_normalBitmap >= 0) {
		bm_page_in_texture(_normalBitmap);
	}
}

bool DecalDefinition::bitmapsLoaded() {
	// Since both bitmap types are optional we need to check if either is loaded to determine if any bitmap is loaded
	return _diffuseBitmap >= 0 || _glowBitmap >= 0 || _normalBitmap >= 0;
}

const SCP_string& DecalDefinition::getName() const {
	return _name;
}
int DecalDefinition::getDiffuseBitmap() const {
	return _diffuseBitmap;
}
int DecalDefinition::getGlowBitmap() const {
	return _glowBitmap;
}
int DecalDefinition::getNormalBitmap() const {
	return _normalBitmap;
}
bool DecalDefinition::isDiffuseLooping() const {
	return _loopDiffuse;
}
bool DecalDefinition::isGlowLooping() const {
	return _loopGlow;
}
bool DecalDefinition::isNormalLooping() const {
	return _loopNormal;
}


SCP_vector<DecalDefinition> DecalDefinitions;

// Variable to indicate if the system is able to work correctly on the current system
bool Decal_system_active = true;
bool Decal_option_active = true;

void parse_decals_table(const char* filename) {
	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Decals");

		while (optional_string("$Decal:")) {
			SCP_string name;
			stuff_string(name, F_NAME);

			DecalDefinition def(name);
			def.parse();

			DecalDefinitions.push_back(std::move(def));
		}

		required_string("#End");
	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}

	if (!gr_is_capable(CAPABILITY_DEFERRED_LIGHTING)) {
		// We need deferred lighting
		Decal_system_active = false;
		mprintf(("Note: Decal system has been disabled due to lack of deferred lighting.\n"));
	}
	if (!gr_is_capable(CAPABILITY_NORMAL_MAP)) {
		// We need normal mapping for the full feature range
		Decal_system_active = false;
		mprintf(("Note: Decal system has been disabled due to lack of normal mapping.\n"));
	}
	if (!gr_is_capable(CAPABILITY_SEPARATE_BLEND_FUNCTIONS)) {
		// We need separate blending functions for different color buffers
		Decal_system_active = false;
		mprintf(("Note: Decal system has been disabled due to lack of separate color buffer blend functions.\n"));
	}
}

struct Decal {
	int definition_handle = -1;
	object_h object;
	int orig_obj_type = OBJ_NONE;
	int submodel = -1;

	float creation_time = -1.0f; //!< The mission time at which this decal was created
	float lifetime = -1.0f; //!< The time this decal is active. When negative it never expires

	vec3d position = vmd_zero_vector;
	vec3d scale;
	matrix orientation = vmd_identity_matrix;

	Decal() {
		vm_vec_make(&scale, 1.f, 1.f, 1.f);
	}

	bool isValid() {
		if (!object.IsValid()) {
			return false;
		}
		if (object.objp->flags[Object::Object_Flags::Should_be_dead]) {
			return false;
		}

		if (orig_obj_type != object.objp->type) {
			mprintf(("Decal object type for object %d has changed from %s to %s. Please let m!m know about this\n",
			         OBJ_INDEX(object.objp), Object_type_names[orig_obj_type], Object_type_names[object.objp->type]));
			return false;
		}

		if (lifetime > 0.0f) {
			if (f2fl(Missiontime) >= creation_time + lifetime) {
				// Decal has expired
				return false;
			}
		}

		auto objp = object.objp;
		if (objp->type == OBJ_SHIP) {
			auto shipp = &Ships[objp->instance];
			auto model_instance = model_get_instance(shipp->model_instance_num);

			Assertion(submodel >= 0 && submodel < model_get(object_get_model(objp))->n_models,
					  "Invalid submodel number detected!");
			auto smi = &model_instance->submodel[submodel];

			if (smi->blown_off) {
				return false;
			}
		} else {
			Assertion(false, "Only ships are currently supported for decals!");
			return false;
		}

		return true;
	}
};

SCP_vector<Decal> active_decals;

bool required_string_if_new(const char* token, bool new_entry) {
	if (!new_entry) {
		return optional_string(token) == 1;
	}

	required_string(token);
	return true;
}

float clamp(float x, float min, float max) {
	return std::min(std::max(x, min), max);
}

/**
 * @brief Produces a smoothstep value as specified by the GLSL specification
 * @param edge0
 * @param edge1
 * @param x
 * @return
 */
float smoothstep(float edge0, float edge1, float x) {
	auto t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

}

namespace decals {

void initialize() {
	if (gr_screen.mode == GR_STUB) {
		Decal_system_active = false;
		return;
	}

	DecalDefinitions.clear();
	graphics::decal_draw_list::globalInit();

	parse_modular_table(NOX("*-dcl.tbm"), parse_decals_table);
}

void shutdown() {
	graphics::decal_draw_list::globalShutdown();

	// Free allocated resources
	DecalDefinitions.clear();
}

int findDecalDefinition(const SCP_string& name) {
	auto ii = std::find_if(DecalDefinitions.begin(), DecalDefinitions.end(), [&](const DecalDefinition& def)
		{
			return def.getName() == name;
		});

	if (ii == DecalDefinitions.end())
		return -1;

	return static_cast<int>(std::distance(DecalDefinitions.begin(), ii));
}

void parseDecalReference(creation_info& dest_info, bool is_new_entry) {
	SCP_string name;
	stuff_string(name, F_NAME);

	// decals are not initialized in FRED, so no decal definitions are known
	if (!Fred_running) {
		auto decalRef = findDecalDefinition(name);

		if (decalRef < 0) {
			error_display(0, "Decal definition '%s' is unknown!", name.c_str());
		}
		dest_info.definition_handle = decalRef;
	}

	if (required_string_if_new("+Radius:", is_new_entry)) {
		dest_info.radius = util::parseUniformRange(0.0001f);
	}

	if (required_string_if_new("+Lifetime:", is_new_entry)) {
		if (optional_string("Eternal")) {
			dest_info.lifetime = util::UniformFloatRange(-1.0f);
		} else {
			// Require at least a small lifetime so that the calculations don't have to deal with div-by-zero
			dest_info.lifetime = util::parseUniformRange(0.0001f);
		}
	}

	if (optional_string("+Use Random Rotation:")) {
		stuff_boolean(&dest_info.random_rotation);
	}
}

void loadBitmaps(const creation_info& info) {
	if (!Decal_system_active || !Decal_option_active) {
		return;
	}
	// Silently ignore invalid definition handle since weapons use the default values if the decal option is not present
	if (info.definition_handle < 0) {
		return;
	}

	Assertion(info.definition_handle >= 0 && info.definition_handle < (int) DecalDefinitions.size(),
			  "Invalid decal handle detected!");

	auto& def = DecalDefinitions[info.definition_handle];

	def.loadBitmaps();
}

void pageInDecal(const creation_info& info) {
	if (!Decal_system_active || !Decal_option_active) {
		return;
	}
	// Silently ignore invalid definition handle since weapons use the default values if the decal option is not present
	if (info.definition_handle < 0) {
		return;
	}

	Assertion(info.definition_handle >= 0 && info.definition_handle < (int) DecalDefinitions.size(),
			  "Invalid decal handle detected!");

	DecalDefinitions[info.definition_handle].pageIn();
}

void initializeMission() {
	active_decals.clear();
}

matrix4 getDecalTransform(Decal& decal) {
	Assertion(decal.object.objp->type == OBJ_SHIP, "Only ships are currently supported for decals!");

	auto objp = decal.object.objp;
	auto ship = &Ships[objp->instance];
	auto pmi = model_get_instance(ship->model_instance_num);
	auto pm = model_get(pmi->model_num);

	vec3d worldPos;
	matrix worldOrient;
	model_instance_local_to_global_point_orient(&worldPos,
									&worldOrient,
									&decal.position,
									&decal.orientation,
									pm,
									pmi,
									decal.submodel,
									&objp->orient,
									&objp->pos);

	// The decal API sees the "direction" of a decal to be along the normal of the surface it is attached to. However,
	// this will lead to a situation where we would look at the decal texture "from behind" causing the texture to
	// appear flipped. We fix that here for the graphics transform by inverting the Z scaling.

	// ALSO for some reason the uvec needs to be flipped as well, otherwise the decals render upside-down.  Without
	// being sure of the root cause, this at least makes it appear correct for modders.

	// Apply scaling
	vm_vec_scale(&worldOrient.vec.rvec, decal.scale.xyz.x);
	vm_vec_scale(&worldOrient.vec.uvec, -decal.scale.xyz.y);
	vm_vec_scale(&worldOrient.vec.fvec, -decal.scale.xyz.z);

	matrix4 mat4;
	vm_matrix4_set_transform(&mat4, &worldOrient, &worldPos);

	return mat4;
}

void renderAll() {
	if (!Decal_system_active || !Decal_option_active) {
		return;
	}

	// Clear out any invalid decals
	for (auto iter = active_decals.begin(); iter != active_decals.end();) {
		if (!iter->isValid()) {
			// if we're sitting on the very last element, popping-back will invalidate the iterator!
			if (iter + 1 == active_decals.end()) {
				active_decals.pop_back();
				break;
			}

			*iter = active_decals.back();
			active_decals.pop_back();
			continue;
		}

		// next decal, only increment the iterator if we found a valid value so nothing gets skipped
		// otherwise we may skip a decal which can then get through to the draw_list loop while being invalid
		++iter;
	}

	if (active_decals.empty()) {
		return;
	}

	auto mission_time = f2fl(Missiontime);

	graphics::decal_draw_list draw_list(active_decals.size());
	for (auto& decal : active_decals) {

		Assertion(decal.definition_handle >= 0 && decal.definition_handle < (int)DecalDefinitions.size(),
			"Invalid decal handle detected!");
		auto& decalDef = DecalDefinitions[decal.definition_handle];

		int diffuse_bm = -1;
		int glow_bm = -1;
		int normal_bm = -1;

		auto decal_time = mission_time - decal.creation_time;
		auto progress = decal_time / decal.lifetime;

		float alpha = 1.0f;
		if (progress > 0.8) {
			// Fade the decal out for the last 20% of its lifetime
			alpha = 1.0f - smoothstep(0.8f, 1.0f, progress);
		}

		if (decalDef.getDiffuseBitmap() >= 0) {
			diffuse_bm = decalDef.getDiffuseBitmap()
				+ bm_get_anim_frame(decalDef.getDiffuseBitmap(), decal_time, 0.0f, decalDef.isDiffuseLooping());
		}

		if (decalDef.getGlowBitmap() >= 0) {
			glow_bm = decalDef.getGlowBitmap()
				+ bm_get_anim_frame(decalDef.getGlowBitmap(), decal_time, 0.0f, decalDef.isGlowLooping());
		}

		if (decalDef.getNormalBitmap() >= 0) {
			normal_bm = decalDef.getNormalBitmap()
				+ bm_get_anim_frame(decalDef.getNormalBitmap(), decal_time, 0.0f, decalDef.isNormalLooping());
		}

		draw_list.add_decal(diffuse_bm, glow_bm, normal_bm, decal_time, getDecalTransform(decal), alpha);
	}

	draw_list.render();
}

void addDecal(creation_info& info, object* host, int submodel, const vec3d& local_pos, const matrix& local_orient) {
	if (!Decal_system_active || !Decal_option_active) {
		return;
	}
	// Silently ignore invalid definition handle since weapons use the default values if the decal option is not present
	if (info.definition_handle < 0) {
		return;
	}

	Assertion(info.definition_handle >= 0 && info.definition_handle < (int) DecalDefinitions.size(),
			  "Invalid decal handle detected!");
	auto& def = DecalDefinitions[info.definition_handle];

	if (!def.bitmapsLoaded()) {
		// If this decal was never used before then the bitmaps are not loaded so we need to do that here.
		def.loadBitmaps();
	}

	Decal newDecal;
	newDecal.definition_handle = info.definition_handle;
	newDecal.object = object_h(host);
	newDecal.orig_obj_type     = host->type;
	newDecal.submodel = submodel;
	newDecal.creation_time = f2fl(Missiontime);
	newDecal.lifetime = info.lifetime.next();

	newDecal.position = local_pos;
	newDecal.orientation = local_orient;
	if (info.width < 0.0f || info.height < 0.0f) {
		float radius = info.radius.next();
		newDecal.scale.xyz.x = radius;
		newDecal.scale.xyz.y = radius;
		newDecal.scale.xyz.z = radius;
	} else {
		newDecal.scale.xyz.x = info.width;
		newDecal.scale.xyz.y = info.height;
		newDecal.scale.xyz.z = MIN(info.width, info.height);
	}

	active_decals.push_back(newDecal);
}

}
