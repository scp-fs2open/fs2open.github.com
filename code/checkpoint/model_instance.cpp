#include "checkpoint/model_instance.h"


#include "bmpman/bmpman.h"
#include "model/animation/modelanimation.h"
#include "model/model.h"

namespace checkpoint {

namespace {

void save_state(Writer& w, int model_instance_num)
{
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);

	// the pose is only meaningful for the same model
	save(w, SCP_string(pm->filename));

	w.u32(static_cast<uint32_t>(pm->n_models));
	for (int i = 0; i < pm->n_models; ++i)
		save(w, pmi->submodel[i]);

	// replaced textures are saved by name, since bitmap handles differ between sessions
	SCP_vector<std::pair<int, SCP_string>> textures;
	if (pmi->texture_replace != nullptr) {
		for (int slot = 0; slot < static_cast<int>(pmi->texture_replace->size()); ++slot) {
			int handle = (*pmi->texture_replace)[slot];
			if (handle >= 0)
				textures.emplace_back(slot, bm_get_filename(handle));
		}
	}
	save(w, textures);

	save(w, animation::ModelAnimationSet::getRunningAnimationStates(pmi));
}

void load_state(Reader& r, int model_instance_num)
{
	auto pmi = model_get_instance(model_instance_num);
	auto pm = model_get(pmi->model_num);

	SCP_string filename;
	load(r, filename);
	if (stricmp(filename.c_str(), pm->filename) != 0) {
		mprintf(("Checkpoint: the model %s has replaced %s; its saved pose and animations are dropped\n", pm->filename, filename.c_str()));
		return;
	}

	auto submodel_count = r.u32();
	for (uint32_t i = 0; i < submodel_count && !r.failed(); ++i) {
		if (static_cast<int>(i) < pm->n_models) {
			load(r, pmi->submodel[i]);
		} else {
			static submodel_instance discard;
			load(r, discard);
		}
	}

	SCP_vector<std::pair<int, SCP_string>> textures;
	load(r, textures);
	if (!textures.empty()) {
		pmi->texture_replace = std::make_shared<model_texture_replace>();
		for (const auto& texture : textures) {
			if (texture.first >= 0 && texture.first < static_cast<int>(pmi->texture_replace->size()))
				(*pmi->texture_replace)[texture.first] = bm_load_either(texture.second.c_str());
		}
	}

	SCP_vector<animation::ModelAnimationSet::RunningAnimationState> animations;
	load(r, animations);
	animation::ModelAnimationSet::restoreRunningAnimationStates(pmi, animations);
}

} // namespace

void save_model_instance(Writer& w, int model_instance_num)
{
	auto at = w.begin_block();
	if (model_instance_num >= 0)
		save_state(w, model_instance_num);
	w.end_block(at);
}

void load_model_instance(Reader& r, int model_instance_num)
{
	auto block = r.block();
	if (model_instance_num >= 0 && !block.at_end())
		load_state(block, model_instance_num);
	if (block.failed())
		r.fail();
}

} // namespace checkpoint
