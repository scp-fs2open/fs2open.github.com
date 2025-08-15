/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "math/vecmat.h"
#include "object/object.h"
#include "parse/parselo.h"
#include "particle/particle.h"
#include "weapon/muzzleflash.h"
#include "model/modelrender.h"


// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH DEFINES/VARS
// 

// muzzle flash info - read from a table
typedef struct mflash_blob_info {
	char name[MAX_FILENAME_LEN];
	float offset;
	float radius;

	mflash_blob_info( const mflash_blob_info& mbi )
	{
		strcpy_s( name, mbi.name );
		offset = mbi.offset;
		radius = mbi.radius;
	}

	mflash_blob_info() :
		offset( 0.0 ),
		radius( 0.0 )
	{ 
		name[ 0 ] = '\0';
	}

	mflash_blob_info& operator=( const mflash_blob_info& r )
	{
		strcpy_s( name, r.name );
		offset = r.offset;
		radius = r.radius;

		return *this;
	}
} mflash_blob_info;

typedef struct mflash_info {
	char name[MAX_FILENAME_LEN];
	SCP_vector<mflash_blob_info> blobs;

	mflash_info()
	{ 
		name[ 0 ] = '\0';
	}

	mflash_info( const mflash_info& mi )
	{
		strcpy_s( name, mi.name );
		blobs = mi.blobs;
	}

	mflash_info& operator=( const mflash_info& r )
	{
		strcpy_s( name, r.name );
		blobs = r.blobs;

		return *this;
	}
} mflash_info;

SCP_vector<mflash_info> Mflash_info;


// ---------------------------------------------------------------------------------------------------------------------
// MUZZLE FLASH FUNCTIONS
//

static const SCP_string mflash_particle_prefix = ";MflashParticle;";

void parse_mflash_tbl(const char *filename)
{
	uint i;

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// header
		required_string("#Muzzle flash types");

		while (optional_string("$Mflash:")) {
			mflash_info mflash;
			bool override_mflash = false;

			required_string("+name:");
			stuff_string(mflash.name, F_NAME, MAX_FILENAME_LEN);

			if (optional_string("+override"))
				override_mflash = true;

			// read in all blobs
			while (optional_string("+blob_name:")) {
				mflash_blob_info mblob;

				stuff_string(mblob.name, F_NAME, MAX_FILENAME_LEN);

				required_string("+blob_offset:");
				stuff_float(&mblob.offset);

				required_string("+blob_radius:");
				stuff_float(&mblob.radius);

				mflash.blobs.push_back(mblob);
			}

			for (i = 0; i < Mflash_info.size(); i++) {
				if (!stricmp(mflash.name, Mflash_info[i].name)) {
					if (override_mflash) {
						Mflash_info[i] = mflash;
					}
					break;
				}
			}

			// no matching name exists so add as new
			if (i == Mflash_info.size()) {
				Mflash_info.push_back(mflash);
			}
			// a mflash of the same name exists, don't add it again
			else {
				if (!override_mflash) {
					Warning(LOCATION, "Muzzle flash \"%s\" already exists!  Using existing entry instead.", mflash.name);
				}
			}
		}

		// close
		required_string("#end");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

static void convert_mflash_to_particle() {
	Curve new_curve = Curve(";MuzzleFlashMinSizeScalingCurve");
	new_curve.keyframes.emplace_back(curve_keyframe{vec2d{ -0.00001f , 0.f}, CurveInterpFunction::Polynomial, -1.0f, 1.0f}); //just for numerical safety if we ever get an actual size of 0...
	new_curve.keyframes.emplace_back(curve_keyframe{vec2d{ Min_pizel_size_muzzleflash, 1.f }, CurveInterpFunction::Constant, 0.0f, 1.0f});
	Curves.emplace_back(std::move(new_curve));
	modular_curves_entry scaling_curve {(static_cast<int>(Curves.size()) - 1), ::util::UniformFloatRange(1.f), ::util::UniformFloatRange(0.f), false};

	for (const auto& mflash : Mflash_info) {
		SCP_vector<particle::ParticleEffect> subparticles;

		for (const auto& blob : mflash.blobs) {
			subparticles.emplace_back(
				mflash_particle_prefix + mflash.name, //Name
				::util::UniformFloatRange(1.f), //Particle num
				particle::ParticleEffect::Duration::ONETIME, //Single Particle Emission
				::util::UniformFloatRange(), //No duration
				::util::UniformFloatRange (-1.f), //Single particle only
				particle::ParticleEffect::ShapeDirection::ALIGNED, //Particle direction
				::util::UniformFloatRange(1.f), //Velocity Inherit
				false, //Velocity Inherit absolute?
				nullptr, //Velocity volume
				::util::UniformFloatRange(), //Velocity volume multiplier
				particle::ParticleEffect::VelocityScaling::NONE, //Velocity directional scaling
				std::nullopt, //Orientation-based velocity
				std::nullopt, //Position-based velocity
				nullptr, //Position volume
				particle::ParticleEffectHandle::invalid(), //Trail
				1.f, //Chance
				false, //Affected by detail
				-1.f, //Culling range multiplier
				false, //Disregard Animation Length. Must be true for everything using particle::Anim_bitmap_X
				false, //Don't reverse animation
				true, //parent local
				true, //ignore velocity inherit if parented
				false, //position velocity inherit absolute?
				std::nullopt, //Local velocity offset
				vec3d{{{0, 0, blob.offset}}}, //Local offset
				::util::UniformFloatRange(-1.f), //Lifetime
				::util::UniformFloatRange(blob.radius), //Radius
				bm_load_animation(blob.name));

			if (Min_pizel_size_muzzleflash > 0) {
				subparticles.back().m_modular_curves.add_curve("Apparent Visual Size At Emitter", particle::ParticleEffect::ParticleCurvesOutput::RADIUS_MULT, scaling_curve);
			}
		}

		particle::ParticleManager::get()->addEffect(std::move(subparticles));
	}

	//Clean up no longer required data
	Mflash_info.clear();
	Mflash_info.shrink_to_fit();
}

// initialize muzzle flash stuff for the whole game
void mflash_game_init()
{
	// parse main table first
	parse_mflash_tbl("mflash.tbl");

	// look for any modular tables
	parse_modular_table(NOX("*-mfl.tbm"), parse_mflash_tbl);

	//This should really happen at parse time, but that requires modular particle effects which aren't yet a thing
	convert_mflash_to_particle();
}

particle::ParticleEffectHandle mflash_lookup(const char *name) {
	return particle::ParticleManager::get()->getEffectByName(mflash_particle_prefix + name);
}
