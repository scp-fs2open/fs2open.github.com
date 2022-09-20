#include "modelreplace.h"

#include "model/model.h"

#include "cfile/cfile.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

static SCP_unordered_map<SCP_string, std::vector<VirtualPOFDefinition>, SCP_string_lcase_hash, SCP_string_lcase_equal_to> virtual_pofs;
static SCP_unordered_map<SCP_string, std::function<std::unique_ptr<VirtualPOFOperation>()>> virtual_pof_operations = {
	{"$Add Subobject:", &make_unique<VirtualPOFOperationAddSubmodel> },
	{"$Rename Subobjects:", &make_unique<VirtualPOFOperationRenameSubobjects> },
	{"$Set Subobject Data:", &make_unique<VirtualPOFOperationChangeData> },
	{"$Set Header Data:", &make_unique<VirtualPOFOperationHeaderData> }
};

/*
* forward declares for internal modelread functions
*/
extern modelread_status read_model_file(polymodel* pm, const char* filename, int ferror, model_read_deferred_tasks& deferredTasks, model_parse_depth depth = {});
extern void create_family_tree(polymodel* obj);
extern void model_calc_bound_box(vec3d* box, vec3d* big_mn, vec3d* big_mx);

static class VirtualPOFBuildCache {
public:
	class polymodel_holder;

private:
	SCP_unordered_map<SCP_string, std::shared_ptr<polymodel_holder>, SCP_string_lcase_hash, SCP_string_lcase_equal_to> cache;

public:
	class polymodel_holder {
		polymodel* _pm;
		model_read_deferred_tasks _deferred;
		SCP_set<int> keepTextures{}, keepGlowbanks{}, keepSM{};
		bool needs_emplace = false;
		friend class VirtualPOFBuildCache;

	public:
		polymodel_holder(const SCP_string& pof_name, const model_parse_depth& depth) : _pm(new polymodel()), _deferred() {
			auto status = read_model_file(_pm, pof_name.c_str(), 0, _deferred, depth);
			create_family_tree(_pm);
			if (status == modelread_status::SUCCESS_REAL)
				needs_emplace = true;
		}

		//Don't copy, just move
		const polymodel_holder(const polymodel_holder&) = delete;
		polymodel_holder& operator=(const polymodel_holder&) = delete;

		~polymodel_holder() {
			if (_pm == nullptr)
				return;
			for (int sm : keepSM)
				_pm->submodel[sm].bsp_data = nullptr;
			model_page_out_textures(_pm, true, keepTextures, keepGlowbanks);
			model_free(_pm);
		}

		inline const model_read_deferred_tasks& deferred() const { return _deferred; }
		inline const polymodel* pm() const { return _pm; }
		inline void keepTexture(int texture) { keepTextures.emplace(texture); }
		inline void keepGlowbank(int gb) { keepGlowbanks.emplace(gb); }
		inline void keepBSPData(int sm) { keepSM.emplace(sm); }
	};

	const std::shared_ptr<polymodel_holder> operator()(const SCP_string& pof_name, const model_parse_depth& depth) {
		auto it = cache.find(pof_name);
		if (it == cache.end()) {
			auto pmh = ::make_shared<polymodel_holder>(pof_name, depth);
			if(pmh->needs_emplace)
				virtual_pof_build_cache.cache.emplace(pof_name, pmh);
			return pmh;
		}
		return it->second;
	}

	void clear() {
		cache.clear();
	}

} virtual_pof_build_cache;

// General Functions and external code paths

bool model_exists(const SCP_string& filename) {
	auto it = virtual_pofs.find(filename);
	if (it != virtual_pofs.end() && !it->second.empty()) {
		return true;
	}

	return cf_exists_full(filename.c_str(), CF_TYPE_MODELS);
}

bool read_virtual_model_file(polymodel* pm, const SCP_string& filename, model_parse_depth depth, int ferror, model_read_deferred_tasks& deferredTasks) {
	auto virtual_pof_it = virtual_pofs.find(filename);

	//We don't have a virtual pof
	if(virtual_pof_it == virtual_pofs.end())
		return false;

	//We have one, but we're already past it and are processing whatever it overwrote
	int& depthLocal = depth[filename];

	if ((int)virtual_pof_it->second.size() <= depthLocal)
		return false;

	const auto& virtual_pof = virtual_pof_it->second[depthLocal];
	depthLocal++;

	read_model_file(pm, filename.c_str(), ferror, deferredTasks, depth);

	for (const auto& operation : virtual_pof.operationList)
		operation->process(pm, deferredTasks, depth, virtual_pof);

	return true;
}

void virtual_pof_purge_cache() {
	virtual_pof_build_cache.clear();
}

// Parsing function

static void parse_virtual_pof() {
	SCP_string name;
	stuff_string(name, F_NAME);

	auto& virtual_pof_list = virtual_pofs[name];
	virtual_pof_list.emplace_back();
	VirtualPOFDefinition& toFill = virtual_pof_list.back();

	toFill.name = name;
	
	required_string("+Base POF:");
	stuff_string(toFill.basePOF, F_FILESPEC);

	do {
		ignore_white_space();
		char operation_type[NAME_LENGTH];
		stuff_string(operation_type, F_NAME, NAME_LENGTH);

		auto operation_parser = virtual_pof_operations.find(operation_type);
		if (operation_parser != virtual_pof_operations.end())
			toFill.operationList.emplace_back(operation_parser->second());
		else {
			error_display(1, "Unknown operation type %s in virtual POF %s!", operation_type, name.c_str());
			virtual_pof_list.pop_back();
			skip_to_start_of_string_either("$POF:", "#End");
			return;
		}
	} while (optional_string_either("$POF:", "#End", false) == -1);
}

static void parse_virtual_pof_table(const char* filename) {
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Virtual POFs");

		while (optional_string("$POF:"))
			parse_virtual_pof();

		required_string("#End");
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void virtual_pof_init() {
	if (cf_exists_full("virtual_pofs.tbl", CF_TYPE_TABLES))
		parse_virtual_pof_table("virtual_pofs.tbl");

	parse_modular_table(NOX("*-pof.tbm"), parse_virtual_pof_table);
}

// Internal helper functions

#define REPLACE_IF_EQ(data) if ((data) == source) (data) = dest;
#define REPLACE_IF_STRIEQ(data) if (stricmp(data, source.c_str()) == 0) { strncpy(data, dest.c_str(), dest.size()); data[dest.size()] = '\0'; }

//Generates one function for replacing data in a type, which is a map entry of which the key may be replaced. Takes an rvalue reference, used for making a copy and modifying the tempoary to then assign it somewhere
#define CHANGE_HELPER_MAP_KEY(name, intype, argtype) template<typename map_t> static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value, intype>::type name(intype&& input, map_t replace){ \
	const auto it = replace.find(input.first); \
	intype result = { (it == replace.end() ? input.first : it->second), input.second }; \
	for (const auto& replacement : replace) { \
		const argtype& source = replacement.first; \
		const argtype& dest = replacement.second;
#define CHANGE_HELPER_MAP_KEY_END } return result; }

//Generates two functions for replacing data in a type. One that takes an rvalue reference, used for making a copy and modifying the tempoary to then assign it somewhere, and one which takes an lvalue reference for modifying in-place
#define CHANGE_HELPER(name, intype, argtype) template<typename map_t> static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value>::type name(intype& input, map_t replace); \
template<typename map_t> inline static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value, intype>::type name(intype&& input, map_t replace){ \
	name<map_t>(input, replace); \
	return input; \
} \
template<typename map_t> static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value>::type name(intype& input, map_t replace) {\
	for (const auto& replacement : replace) { \
		const argtype& source = replacement.first; \
		const argtype& dest = replacement.second;
#define CHANGE_HELPER_END }}

//Change submodel numbers in a submodel
CHANGE_HELPER(change_submodel_numbers, bsp_info, int) 
	for (auto& detail : input.details)
		REPLACE_IF_EQ(detail);
	REPLACE_IF_EQ(input.first_child);
	REPLACE_IF_EQ(input.i_replace);
	for (auto& debris : input.live_debris)
		REPLACE_IF_EQ(debris);
	REPLACE_IF_EQ(input.look_at_submodel);
	REPLACE_IF_EQ(input.my_replacement);
	REPLACE_IF_EQ(input.next_sibling);
	REPLACE_IF_EQ(input.parent);
CHANGE_HELPER_END

//Change submodel numbers in a subsystem definition
CHANGE_HELPER(change_submodel_numbers, model_read_deferred_tasks::model_subsystem_pair, int)
	REPLACE_IF_EQ(input.second.subobj_nr);
CHANGE_HELPER_END

//Change submodel numbers in a turret definition
CHANGE_HELPER_MAP_KEY(change_submodel_numbers, model_read_deferred_tasks::weapon_subsystem_pair, int)
	REPLACE_IF_EQ(input.second.gun_subobj_nr);
CHANGE_HELPER_MAP_KEY_END

//Change submodel name in a submodel
CHANGE_HELPER(change_submodel_name, bsp_info, SCP_string)
	REPLACE_IF_STRIEQ(input.name);
	REPLACE_IF_STRIEQ(input.lod_name);
CHANGE_HELPER_END

//Change submodel name in a subsystem definition
CHANGE_HELPER_MAP_KEY(change_submodel_name, model_read_deferred_tasks::model_subsystem_pair, SCP_string)
	SCP_UNUSED(source);
	SCP_UNUSED(dest);
CHANGE_HELPER_MAP_KEY_END

//Change submodel numbers in an engine definition
CHANGE_HELPER_MAP_KEY(change_submodel_name, model_read_deferred_tasks::engine_subsystem_pair, SCP_string)
	SCP_UNUSED(source);
	SCP_UNUSED(dest);
CHANGE_HELPER_MAP_KEY_END

#undef CHANGE_HELPER
#undef CHANGE_HELPER_END
#undef CHANGE_HELPER_MAP_KEY
#undef CHANGE_HELPER_MAP_KEY_END
#undef REPLACE_IF_EQ
#undef REPLACE_IF_STRIEQ

// Actual replacement operations

VirtualPOFOperationAddSubmodel::VirtualPOFOperationAddSubmodel() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Source Subobject:");
	stuff_string(subobjNameSrc, F_NAME);

	if (optional_string("+Copy Children:")) {
		stuff_boolean(&copyChildren);
	}

	if (optional_string("$Rename Subobjects:")) {
		rename = make_unique<VirtualPOFOperationRenameSubobjects>();
	}

	required_string("+Destination Subobject:");
	stuff_string(subobjNameDest, F_NAME);
}

void VirtualPOFOperationAddSubmodel::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	auto appendingPMholder = virtual_pof_build_cache(appendingPOF, depth);
	const model_read_deferred_tasks& appendingSubsys = appendingPMholder->deferred();
	const polymodel* appendingPM = appendingPMholder->pm();

	int src_subobj_no = model_find_submodel_index(appendingPM, subobjNameSrc.c_str());
	int dest_subobj_no = model_find_submodel_index(pm, subobjNameDest.c_str());

	if (src_subobj_no >= 0 && dest_subobj_no >= 0) {
		create_family_tree(pm);

		const VirtualPOFOperationRenameSubobjects::replacement_map& renameMap = (rename == nullptr ? VirtualPOFOperationRenameSubobjects::replacement_map{} : rename->replacements);

		SCP_vector<int> to_copy_submodels;
		bool has_name_collision = false;
		if (copyChildren) {
			model_iterate_submodel_tree(appendingPM, src_subobj_no, [&to_copy_submodels, &has_name_collision, pm, appendingPM](int submodel, int /*level*/, bool /*isLeaf*/) {
				to_copy_submodels.emplace_back(submodel);
				if(model_find_submodel_index(pm, appendingPM->submodel[submodel].name) != -1)
					has_name_collision = true;
				});
		}
		else {
			to_copy_submodels.emplace_back(src_subobj_no);
			if (model_find_submodel_index(pm, appendingPM->submodel[src_subobj_no].name) != -1)
				has_name_collision = true;
		}

		if (!has_name_collision) {
			//Make sure to keep old data
			bsp_info* oldSubmodels = pm->submodel;
			int old_n_submodel = pm->n_models;

			//Realloc new submodel array of proper size
			pm->n_models += (int)to_copy_submodels.size();
			pm->submodel = new bsp_info[pm->n_models];

			//Copy over old data. Pointers in the struct can still point to old members, we will just delete the outer bsp_info array
			for (int i = 0; i < old_n_submodel; i++)
				pm->submodel[i] = oldSubmodels[i];
			delete[] oldSubmodels;

			SCP_unordered_map<int, int> replaceSubobjNo;
			for (int i = 0; i < (int)to_copy_submodels.size(); i++)
				replaceSubobjNo.emplace(to_copy_submodels[i], i + old_n_submodel);

			//Copy subsystem definitions
			for (int i = 0; i < (int)to_copy_submodels.size(); i++) {
				auto it = appendingSubsys.model_subsystems.find(appendingPM->submodel[to_copy_submodels[i]].name);
				if (it != appendingSubsys.model_subsystems.end()) {
					deferredTasks.model_subsystems.emplace(
						change_submodel_name(change_submodel_numbers(model_read_deferred_tasks::model_subsystem_pair(*it), replaceSubobjNo), renameMap)
					);
				}
			}
			
			int deltaDepth = (pm->submodel[dest_subobj_no].depth + 1) - appendingPM->submodel[src_subobj_no].depth;

			SCP_map<int, int> textureIDReplace;

			//Copy over new data. This one needs to be fully free'd afterwards, so make sure to nullptr the respective pointers before freeing later
			for (int i = 0; i < (int)to_copy_submodels.size(); i++) {
				auto& newSubmodel = pm->submodel[i + old_n_submodel];
				newSubmodel = change_submodel_name(change_submodel_numbers(bsp_info(appendingPM->submodel[to_copy_submodels[i]]), replaceSubobjNo), renameMap);

				//Set new Depth
				newSubmodel.depth += deltaDepth;

				//Store texture replacement indices
				const auto& textureIDs = model_get_textures_used(appendingPM, to_copy_submodels[i]);
				for (const auto& textureID : textureIDs) {
					auto it = textureIDReplace.find(textureID);
					if (it == textureIDReplace.end()) {
						int newID = pm->n_textures++;
						if (pm->n_textures > MAX_MODEL_TEXTURES) {
							Warning(LOCATION, "Failed to add submodel %s of POF %s to virtual POF %s, combined POF has too many (over %d) textures.", subobjNameSrc.c_str(), appendingPOF.c_str(), virtualPof.name.c_str(), MAX_MODEL_TEXTURES);
							return;
						}
						it = textureIDReplace.emplace(textureID, newID).first;
					}
					deferredTasks.texture_replacements[i + old_n_submodel].replacementIds[textureID] = it->second;
				}

				//Clear old pointer to data
				appendingPMholder->keepBSPData(to_copy_submodels[i]);
			}

			//Register as next child to our destination
			pm->submodel[dest_subobj_no].num_children++;
			int last_sibling = pm->submodel[dest_subobj_no].first_child;
			int last_true_sibling = -1;
			while (last_sibling >= 0) {
				last_true_sibling = last_sibling;
				last_sibling = pm->submodel[last_sibling].next_sibling;
			}

			if(last_true_sibling == -1)
				pm->submodel[dest_subobj_no].first_child = old_n_submodel;
			else
				pm->submodel[last_true_sibling].next_sibling = old_n_submodel; //The selected submodel will always be first, and thus get the first id
			pm->submodel[old_n_submodel].next_sibling = -1; //Our old submodel might've had a sibling. Not anymore.
			pm->submodel[old_n_submodel].parent = dest_subobj_no;

			if (!copyChildren) {
				//Clear children if needed
				pm->submodel[old_n_submodel].num_children = 0;
				pm->submodel[old_n_submodel].first_child = -1;
			}

			//Actually copy textures
			for (const auto& usedTexture : textureIDReplace) {
				pm->maps[usedTexture.second] = appendingPM->maps[usedTexture.first];
				appendingPMholder->keepTexture(usedTexture.first);
			}
		}
		else {
			Warning(LOCATION, "Failed to add submodel %s of POF %s to virtual POF %s, original POF already has a subsystem with the same name as was supposed to be added.", subobjNameSrc.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
		}
	}
	else {
		Warning(LOCATION, "Failed to add submodel %s of POF %s to virtual POF %s, specified subobject was not found. Returning original POF", subobjNameSrc.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
	}
}

VirtualPOFOperationRenameSubobjects::VirtualPOFOperationRenameSubobjects() {
	while (optional_string("+Replace:")) {
		SCP_string replace;
		stuff_string(replace, F_NAME);
		required_string("+With:");
		stuff_string(replacements[replace], F_NAME);
	}
}

void VirtualPOFOperationRenameSubobjects::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth /*depth*/, const VirtualPOFDefinition& /*virtualPof*/) const {

	for (int i = 0; i < pm->n_models; i++)
		change_submodel_name(pm->submodel[i], replacements);

	decltype(deferredTasks.model_subsystems) copy_model_subsys;
	for (const auto& entry : deferredTasks.model_subsystems)
		copy_model_subsys.emplace(change_submodel_name(model_read_deferred_tasks::model_subsystem_pair(entry), replacements));
	deferredTasks.model_subsystems = std::move(copy_model_subsys);

	decltype(deferredTasks.engine_subsystems) copy_engine_subsys;
	for (const auto& entry : deferredTasks.engine_subsystems)
		copy_engine_subsys.emplace(change_submodel_name(model_read_deferred_tasks::engine_subsystem_pair(entry), replacements));
	deferredTasks.engine_subsystems = std::move(copy_engine_subsys);
}

VirtualPOFOperationChangeData::VirtualPOFOperationChangeData() {
	required_string("+Submodel:");
	stuff_string(submodel, F_NAME);

	if (optional_string("+Set Offset:")) {
		vec3d& offset = setOffset.emplace();
		stuff_vec3d(&offset);
	}
}

void VirtualPOFOperationChangeData::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth /*depth*/, const VirtualPOFDefinition& virtualPof) const {
	int subobj_no = model_find_submodel_index(pm, submodel.c_str());

	if (subobj_no == -1) {
		Warning(LOCATION, "Failed to find submodel %s to change data of in virtual POF %s. Returning original POF", submodel.c_str(), virtualPof.name.c_str());
		return;
	}

	if (setOffset) {
		pm->submodel[subobj_no].offset = *setOffset;
		auto it = deferredTasks.model_subsystems.find(submodel);
		if (it != deferredTasks.model_subsystems.end()) {
			it->second.pnt = *setOffset;
		}
	}
}

VirtualPOFOperationHeaderData::VirtualPOFOperationHeaderData() {
	if (optional_string("+Set Radius:")) {
		radius = 0.0f;
		stuff_float(&(*radius));
	}

	if (optional_string("$Set Bounding Box:")) {
		auto& bb = boundingbox.emplace();
		required_string("+Minimum:");
		stuff_vec3d(&bb.first);
		required_string("+Maximum:");
		stuff_vec3d(&bb.second);
	}
}

void VirtualPOFOperationHeaderData::process(polymodel* pm, model_read_deferred_tasks& /*deferredTasks*/, model_parse_depth /*depth*/, const VirtualPOFDefinition& /*virtualPof*/) const {
	if (radius) {
		pm->rad = pm->core_radius = *radius;
	}

	if (boundingbox) {
		pm->mins = (*boundingbox).first;
		pm->maxs = (*boundingbox).second;
		model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
	}
}