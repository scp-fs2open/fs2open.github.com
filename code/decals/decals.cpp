#include <nebula/neb.h>
#include <utility>
#include "decals/decals.h"

#include "graphics/2d.h"
#include "graphics/decal_draw_list.h"
#include "graphics/util/uniform_structs.h"
#include "parse/parselo.h"
#include "tracing/tracing.h"
#include "ship/ship.h"

namespace {

class DecalDefinition {
	SCP_string _name;

	SCP_string _diffuseFilename;
	bool _loopDiffuse = true;

	SCP_string _glowFilename;
	bool _loopGlow = true;

	SCP_string _normalMapFilename;
	bool _loopNormal = true;

	int _diffuseBitmap = -1;
	int _glowBitmap = -1;
	int _normalBitmap = -1;

 public:
	explicit DecalDefinition(SCP_string name) : _name(std::move(name)) {
	}

	~DecalDefinition() {
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

	// Disallow copying
	DecalDefinition(const DecalDefinition&) = delete;
	DecalDefinition& operator=(const DecalDefinition&) = delete;

	// Move constructor and operator
	DecalDefinition(DecalDefinition&& other) noexcept {
		*this = std::move(other); // Use operator implementation
	}
	DecalDefinition& operator=(DecalDefinition&& other) noexcept {
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

	void parse() {
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

	void loadBitmaps() {
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

	void pageIn() {
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

	bool bitmapsLoaded() {
		// Since both bitmap types are optional we need to check if either is loaded to determine if any bitmap is loaded
		return _diffuseBitmap >= 0 || _glowBitmap >= 0 || _normalBitmap >= 0;
	}

	const SCP_string& getName() const {
		return _name;
	}
	int getDiffuseBitmap() const {
		return _diffuseBitmap;
	}
	int getGlowBitmap() const {
		return _glowBitmap;
	}
	int getNormalBitmap() const {
		return _normalBitmap;
	}
	bool isDiffuseLooping() const {
		return _loopDiffuse;
	}
	bool isGlowLooping() const {
		return _loopGlow;
	}
	bool isNormalLooping() const {
		return _loopNormal;
	}
};

SCP_vector<DecalDefinition> decalDefinitions;

// Variable to indicate if the system is able to work correctly on the current system
bool decal_system_active = true;

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

			decalDefinitions.push_back(std::move(def));
		}

		required_string("#End");
	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}

	if (!gr_is_capable(CAPABILITY_DEFERRED_LIGHTING)) {
		// we need deferred lighting
		decal_system_active = false;
		mprintf(("Note: Decal system has been disabled due to lack of deferred lighting.\n"));
	}
	if (!gr_is_capable(CAPABILITY_NORMAL_MAP)) {
		// We need normal mapping for the full feature range
		decal_system_active = false;
		mprintf(("Note: Decal system has been disabled due to lack of normal mapping.\n"));
	}
	if (!gr_is_capable(CAPABILITY_SEPARATE_BLEND_FUNCTIONS)) {
		// WWe need separate blending functions for different color buffers
		decal_system_active = false;
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
	decalDefinitions.clear();
	graphics::decal_draw_list::globalInit();

	parse_modular_table(NOX("*-dcl.tbm"), parse_decals_table);
}
void shutdown() {
	graphics::decal_draw_list::globalShutdown();

	// Free allocated resources
	decalDefinitions.clear();
}

int getDecalConfig(const SCP_string& name) {
	int index = 0;
	for (auto& def : decalDefinitions) {
		if (!stricmp(def.getName().c_str(), name.c_str())) {
			return index;
		}
		++index;
	}

	return -1;
}

void parseDecalReference(creation_info& dest_info, bool is_new_entry) {
	SCP_string name;
	stuff_string(name, F_NAME);

	auto decalRef = getDecalConfig(name);

	if (decalRef < 0) {
		error_display(0, "Decal definition '%s' is unknown!", name.c_str());
	}
	dest_info.definition_handle = decalRef;

	if (required_string_if_new("+Radius:", is_new_entry)) {
		stuff_float(&dest_info.radius);

		if (dest_info.radius <= 0.0f) {
			error_display(0, "Invalid radius of %f! Must be a positive number.", dest_info.radius);
			dest_info.radius = 1.0f;
		}
	}

	if (required_string_if_new("+Lifetime:", is_new_entry)) {
		if (optional_string("Eternal")) {
			dest_info.lifetime = util::UniformFloatRange(-1.0f);
		} else {
			// Require at least a small lifetime so that the calculations don't have to deal with div-by-zero
			dest_info.lifetime = util::parseUniformRange(0.0001f);
		}
	}
}
void loadBitmaps(const creation_info& info) {
	if (!decal_system_active) {
		return;
	}
	// Silently ignore invalid definition handle since weapons use the default values if the decal option is not present
	if (info.definition_handle < 0) {
		return;
	}

	Assertion(info.definition_handle >= 0 && info.definition_handle < (int) decalDefinitions.size(),
			  "Invalid decal handle detected!");

	auto& def = decalDefinitions[info.definition_handle];

	def.loadBitmaps();
}
void pageInDecal(const creation_info& info) {
	if (!decal_system_active) {
		return;
	}
	// Silently ignore invalid definition handle since weapons use the default values if the decal option is not present
	if (info.definition_handle < 0) {
		return;
	}

	Assertion(info.definition_handle >= 0 && info.definition_handle < (int) decalDefinitions.size(),
			  "Invalid decal handle detected!");

	decalDefinitions[info.definition_handle].pageIn();
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
	model_instance_find_world_point(&worldPos,
									&decal.position,
									pm,
									pmi,
									decal.submodel,
									&objp->orient,
									&objp->pos);

	vec3d worldDir;
	model_instance_find_world_dir(&worldDir,
								  &decal.orientation.vec.fvec,
								  pm,
								  pmi,
								  decal.submodel,
								  &objp->orient);

	vec3d worldUp;
	model_instance_find_world_dir(&worldUp,
								  &decal.orientation.vec.fvec,
								  pm,
								  pmi,
								  decal.submodel,
								  &objp->orient);

	matrix worldOrient;
	vm_vector_2_matrix(&worldOrient, &worldDir, &worldUp);

	// Apply scaling
	worldOrient.a2d[0][0] *= decal.scale.xyz.x;
	worldOrient.a2d[0][1] *= decal.scale.xyz.x;
	worldOrient.a2d[0][2] *= decal.scale.xyz.x;
	worldOrient.a2d[1][0] *= decal.scale.xyz.y;
	worldOrient.a2d[1][1] *= decal.scale.xyz.y;
	worldOrient.a2d[1][2] *= decal.scale.xyz.y;
	worldOrient.a2d[2][0] *= decal.scale.xyz.z;
	worldOrient.a2d[2][1] *= decal.scale.xyz.z;
	worldOrient.a2d[2][2] *= decal.scale.xyz.z;

	matrix4 mat4;
	vm_matrix4_set_transform(&mat4, &worldOrient, &worldPos);

	return mat4;
}

void renderAll() {
	if (!decal_system_active) {
		return;
	}

	// Clear out any invalid decals
	for (auto iter = active_decals.begin(); iter != active_decals.end(); ++iter) {
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
	}

	if (active_decals.empty()) {
		return;
	}

	auto mission_time = f2fl(Missiontime);

	graphics::decal_draw_list draw_list(active_decals.size());
	for (auto& decal : active_decals) {
		int diffuse_bm = -1;
		int glow_bm = -1;
		int normal_bm = -1;

		Assertion(decal.definition_handle >= 0 && decal.definition_handle < (int) decalDefinitions.size(),
				  "Invalid decal handle detected!");
		auto& decalDef = decalDefinitions[decal.definition_handle];

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
void
addDecal(creation_info& info, object* host, int submodel, const vec3d& local_pos, const matrix& local_orient) {
	if (!decal_system_active) {
		return;
	}
	// Silently ignore invalid definition handle since weapons use the default values if the decal option is not present
	if (info.definition_handle < 0) {
		return;
	}

	Assertion(info.definition_handle >= 0 && info.definition_handle < (int) decalDefinitions.size(),
			  "Invalid decal handle detected!");
	auto& def = decalDefinitions[info.definition_handle];

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
	newDecal.scale.xyz.x = info.radius;
	newDecal.scale.xyz.y = info.radius;
	newDecal.scale.xyz.z = info.radius;

	active_decals.push_back(newDecal);
}

}
