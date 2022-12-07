#pragma once

#include "globalincs/pstypes.h"

#include "object/object.h"

#include "utils/RandomRange.h"

namespace decals {

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
	explicit DecalDefinition(SCP_string name);
	~DecalDefinition();

	// Disallow copying
	DecalDefinition(const DecalDefinition&) = delete;
	DecalDefinition& operator=(const DecalDefinition&) = delete;

	// Move constructor and operator
	DecalDefinition(DecalDefinition&& other) noexcept;
	DecalDefinition& operator=(DecalDefinition&& other) noexcept;

	void parse();
	void loadBitmaps();
	void pageIn();
	bool bitmapsLoaded();

	const SCP_string& getName() const;
	int getDiffuseBitmap() const;
	int getGlowBitmap() const;
	int getNormalBitmap() const;
	bool isDiffuseLooping() const;
	bool isGlowLooping() const;
	bool isNormalLooping() const;
};

extern SCP_vector<DecalDefinition> DecalDefinitions;
extern bool Decal_system_active;
extern bool Decal_option_active;


/**
 * @brief A reference to a decal definition
 */
typedef int DecalReference;

/**
 * @brief A structure containing all information for creating a decal
 */
struct creation_info {
	DecalReference definition_handle = -1;
	util::UniformFloatRange radius = ::util::UniformFloatRange(-1.0f);
	float width = -1.0f;
	float height = -1.0f;
	util::UniformFloatRange lifetime = ::util::UniformFloatRange(-1.0f);
	bool random_rotation = false;
};

/**
 * @brief Initializes the global state of the decal system. Call once at game init.
 */
void initialize();


/**
 * Returns the index of this decal, or -1 if not found
 */
int findDecalDefinition(const SCP_string& name);

/**
 * @brief Parses the information for a decal reference
 *
 * @details This function expects that the initial token has already been consumed by the parse system.
 *
 * @param dest_info The creation information struct to parse the information into
 * @param is_new_entry Set to @c false if the currently parsed object used +nocreate
 */
void parseDecalReference(creation_info& dest_info, bool is_new_entry = true);

/**
 * @brief Loads the bitmaps specified by the creation info into texture memory
 *
 * This should be called when the bitmaps of the object using this decal are loaded.
 *
 * @param info The creation information containing the decal definition handle
 */
void loadBitmaps(const creation_info& info);

/**
 * @brief Page in the bitmap data of the specified creation information. Needs to be called after loadBitmaps
 * @param info The information to page in
 */
void pageInDecal(const creation_info& info);

/**
 * @brief Shut down the decal system. Must be called from game_shutdown
 */
void shutdown();

/**
 * @brief Initialize the decal system for the mission. Must be called before every mission is started
 */
void initializeMission();

/**
 * @brief Renders all currently active decals and removes invalid decals from the active list.
 */
void renderAll();

/**
 * @brief Creates a new decal on the specified object at the specified location
 *
 * @param info The decal to create
 * @param host The object host on which to create this decal. Must be a ship.
 * @param submodel The submodel of the object model on which to create the decal.
 * @param local_pos The position in the frame of reference of the submodel at which the decal should be created.
 * @param local_orient The orientation of the decal in the frame of reference of the submodel. The forward vector of the
 * orientation should look along the direction
 */
void addDecal(creation_info& info,
			  object* host,
			  int submodel,
			  const vec3d& local_pos,
			  const matrix& local_orient);

}
