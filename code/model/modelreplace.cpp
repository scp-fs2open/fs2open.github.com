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
	{"$Add Turret:", &make_unique<VirtualPOFOperationAddTurret> },
	{"$Add Engine:", &make_unique<VirtualPOFOperationAddEngine> },
	{"$Add Glowpoint:", &make_unique<VirtualPOFOperationAddGlowpoint> },
	{"$Add Weapon Bank:", &make_unique<VirtualPOFOperationAddWeapons> },
	{"$Add Dock Point:", &make_unique<VirtualPOFOperationAddDockPoint> },
	{"$Add Path:", &make_unique<VirtualPOFOperationAddPath> },
	{"$Rename Subobjects:", &make_unique<VirtualPOFOperationRenameSubobjects> },
	{"$Set Subsystem Data:", &make_unique<VirtualPOFOperationChangeSubsystemData> },
	{"$Set Subobject Data:", &make_unique<VirtualPOFOperationChangeData> },
	{"$Set Header Data:", &make_unique<VirtualPOFOperationHeaderData> }
};

/*
* forward declares for internal modelread functions
*/
extern modelread_status read_model_file(polymodel* pm, const char* filename, int ferror, model_read_deferred_tasks& deferredTasks, model_parse_depth depth = {});
extern void create_family_tree(polymodel* obj);
extern void model_calc_bound_box(vec3d* box, vec3d* big_mn, vec3d* big_mx);

/*
* Caching base-loaded POFs, as well has providing automatic deallocation for loaded models
*/

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
		polymodel_holder(const polymodel_holder&) = delete;
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

	const std::shared_ptr<polymodel_holder> operator()(const SCP_string& pof_name, model_parse_depth& depth) {
		auto vp_it = virtual_pofs.find(pof_name);

		auto it = cache.find(pof_name);
		
		//Don't load from cache if it's a virtual pof (always reload these) or we don't have it cached
		if ((vp_it != virtual_pofs.end() && (int)vp_it->second.size() > depth[pof_name]) || it == cache.end()) {
			auto pmh = ::make_shared<polymodel_holder>(pof_name, depth);
			if(pmh->needs_emplace)
				cache.emplace(pof_name, pmh);
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

template<typename T, bool vmalloc, typename member_t>
inline void object_copy_including_array_member_inner(const T& item, T& result, int size, member_t T::* ptm) {
	if (vmalloc)
		result.*ptm = (member_t)vm_malloc(sizeof(typename std::remove_pointer<member_t>::type) * (size));
	else
		result.*ptm = new typename std::remove_pointer<member_t>::type[size];

	for (int i = 0; i < size; i++)
		(result.*ptm)[i] = (item.*ptm)[i];
}

template<typename T, bool vmalloc, typename member_t_0, typename member_t_1, typename... member_t>
inline void object_copy_including_array_member_inner(const T& item, T& result, int size, member_t_0 T::* ptm, member_t_1 T::* ptm1, member_t T::*... ptms) {
	object_copy_including_array_member_inner<T, vmalloc>(item, result, size, ptm);
	object_copy_including_array_member_inner<T, vmalloc>(item, result, size, ptm1, ptms...);
}

template<typename T, bool vmalloc = true, typename... member_t>
T object_copy_including_array_member(const T& item, int T::* size, member_t T::*... ptm) {
	T result{ item };
	object_copy_including_array_member_inner<T, vmalloc>(item, result, result.*size, ptm...);
	return result;
}

template<typename T>
int reallocate_and_copy_array(T*& array, int& size, size_t to_add) {
	//Make sure to keep old data
	T* oldArray = array;

	int size_before = size;

	//Realloc new submodel array of proper size
	size += static_cast<int>(to_add);
	array = new T[size];

	//Copy over old data. Pointers in the struct can still point to old members, we will just delete the outer bsp_info array
	for (int i = 0; i < size_before; i++)
		array[i] = std::move(oldArray[i]);
	delete[] oldArray;

	return size_before;
}

template<typename T>
int reallocate_and_copy_array_vmalloc(T*& array, int& size, size_t to_add) {
	//Make sure to keep old data
	T* oldArray = array;

	int size_before = size;

	//Realloc new submodel array of proper size
	size += static_cast<int>(to_add);
	array = (T*)vm_malloc(sizeof(T) * size);

	//Copy over old data. Pointers in the struct can still point to old members, we will just delete the outer bsp_info array
	for (int i = 0; i < size_before; i++)
		array[i] = std::move(oldArray[i]);
	vm_free(oldArray);

	return size_before;
}

#define REPLACE_IF_EQ(data) if ((data) == source) (data) = dest;
#define REPLACE_IF_STRIEQ(data) if (stricmp(data, source.c_str()) == 0) strncpy_s(data, dest.c_str(), dest.size()); 
#define REPLACE_IF_SCPSTRIEQ(data) if (stricmp(data.c_str(), source.c_str()) == 0) data = dest; 

//Generates one function for replacing data in a type, which is a map entry of which the key may be replaced. Takes an rvalue reference, used for making a copy and modifying the temporary to then assign it somewhere
#define CHANGE_HELPER_MAP_KEY(name, intype, argtype) template<typename map_t> static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value, intype>::type name(intype&& pass, map_t replace){ \
	const auto it = replace.find(pass.first); \
	intype input = { (it == replace.end() ? pass.first : it->second), pass.second }; \
	for (const auto& replacement : replace) { \
		const argtype& source = replacement.first; \
		const argtype& dest = replacement.second;
#define CHANGE_HELPER_MAP_KEY_END } return input; }

//Generates two functions for replacing data in a type. One that takes an rvalue reference, used for making a copy and modifying the temporary to then assign it somewhere, and one which takes an lvalue reference for modifying in-place
#define CHANGE_HELPER(name, intype, argtype) template<typename map_t> static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value>::type name(intype& input, map_t replace); \
template<typename map_t> inline static typename std::enable_if<std::is_same<typename map_t::value_type, std::pair<const argtype, argtype>>::value, intype>::type name(intype&& input, map_t replace){ \
	name<map_t>(input, replace); \
	return std::move(input); \
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

//Change submodel numbers in a glowpoint bank definition
CHANGE_HELPER(change_submodel_numbers, glow_point_bank, int)
REPLACE_IF_EQ(input.submodel_parent);
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

//Change subsystem name in an engine subsys definition
CHANGE_HELPER(change_submodel_name, model_read_deferred_tasks::engine_subsystem_pair, SCP_string)
	REPLACE_IF_SCPSTRIEQ(input.second.subsystem_name)
CHANGE_HELPER_END

//Change engine numbers in an engine subsys definition
CHANGE_HELPER_MAP_KEY(change_engine_numbers, model_read_deferred_tasks::engine_subsystem_pair, int)
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

	if (optional_string("+Copy Turrets:")) {
		stuff_boolean(&copyTurrets);
	}

	if (optional_string("+Copy Glowpoints:")) {
		stuff_boolean(&copyGlowpoints);
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

		const VirtualPOFNameReplacementMap& renameMap = (rename == nullptr ? VirtualPOFNameReplacementMap{} : rename->replacements);

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
			int old_n_submodel = reallocate_and_copy_array(pm->submodel, pm->n_models, to_copy_submodels.size());

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

			if (copyTurrets) {
				//Copy turrets
				for (const auto& turretSubsys : appendingSubsys.weapons_subsystems) {
					//Don't copy turrets that aren't fully contained in data to be copied
					if (std::find(to_copy_submodels.begin(), to_copy_submodels.end(), turretSubsys.first) == to_copy_submodels.end())
						continue;

					if (turretSubsys.second.gun_subobj_nr != turretSubsys.first && std::find(to_copy_submodels.begin(), to_copy_submodels.end(), turretSubsys.second.gun_subobj_nr) == to_copy_submodels.end())
						continue;

					deferredTasks.weapons_subsystems.emplace(change_submodel_numbers(model_read_deferred_tasks::weapon_subsystem_pair(turretSubsys), replaceSubobjNo));
				}
			}

			if (copyGlowpoints) {
				SCP_vector<const glow_point_bank*> glowpointbanks;
				for (int i = 0; i < appendingPM->n_glow_point_banks; i++) {
					const glow_point_bank* gpb = &appendingPM->glow_point_banks[i];
					//Don't copy glowpoints that aren't fully contained in data to be copied
					if (std::find(to_copy_submodels.begin(), to_copy_submodels.end(), gpb->submodel_parent) == to_copy_submodels.end())
						continue;

					appendingPMholder->keepGlowbank(i);
					glowpointbanks.emplace_back(std::move(gpb));
				}
				int insertFrom = reallocate_and_copy_array(pm->glow_point_banks, pm->n_glow_point_banks, glowpointbanks.size());
				for (const glow_point_bank* gpb : glowpointbanks) {
					pm->glow_point_banks[insertFrom] = object_copy_including_array_member(*gpb, &glow_point_bank::num_points, &glow_point_bank::points);
					change_submodel_numbers(pm->glow_point_banks[insertFrom], replaceSubobjNo);
				}
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

VirtualPOFOperationAddTurret::VirtualPOFOperationAddTurret() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Source Turret:");
	stuff_string(baseNameSrc, F_NAME);

	required_string("+Destination Turret:");
	stuff_string(baseNameDest, F_NAME);

	if (optional_string("+Destination Barrel:")) {
		SCP_string barrelDest;
		stuff_string(barrelDest, F_NAME);
		barrelNameDest = std::move(barrelDest);
	}
}

void VirtualPOFOperationAddTurret::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	auto appendingPM = virtual_pof_build_cache(appendingPOF, depth);

	SCP_unordered_map<int, int> replaceSubmodelNo;

	int base_src_subobj_no = model_find_submodel_index(appendingPM->pm(), baseNameSrc.c_str());
	int base_dest_subobj_no = model_find_submodel_index(pm, baseNameDest.c_str());

	replaceSubmodelNo.emplace(base_src_subobj_no, base_dest_subobj_no);

	auto it = appendingPM->deferred().weapons_subsystems.find(base_src_subobj_no);
	if (base_src_subobj_no == -1 || base_dest_subobj_no == -1 || it == appendingPM->deferred().weapons_subsystems.end()) {
		Warning(LOCATION, "Failed to find turret base %s of POF %s for virtual POF %s. Returning original POF.", baseNameSrc.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
		return;
	}

	if (it->second.gun_subobj_nr != base_src_subobj_no) {
		int gun_dest_subobj_no = -1;
		if (barrelNameDest) {
			gun_dest_subobj_no = model_find_submodel_index(pm, barrelNameDest->c_str());
		}
		if (gun_dest_subobj_no == -1) {
			Warning(LOCATION, "Failed to find turret barrel of turret %s of POF %s for virtual POF %s. Returning original POF.", baseNameDest.c_str(), pm->filename, virtualPof.name.c_str());
			return;
		}
		replaceSubmodelNo.emplace(it->second.gun_subobj_nr, gun_dest_subobj_no);

		pm->submodel[gun_dest_subobj_no].rotation_type = MOVEMENT_TYPE_TURRET;
	}
	pm->submodel[base_dest_subobj_no].rotation_type = MOVEMENT_TYPE_TURRET;

	deferredTasks.weapons_subsystems.emplace(change_submodel_numbers(model_read_deferred_tasks::weapon_subsystem_pair(*it), replaceSubmodelNo));
}

VirtualPOFOperationAddEngine::VirtualPOFOperationAddEngine() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	if (required_string_either("+Source Engine:", "+Source Engine Subsystem:") == 0) {
		Mp += 15;
		int engineNr;
		stuff_int(&engineNr);
		sourceId = engineNr - 1;
	}
	else {
		Mp += 25;
		SCP_string engineSubsys;
		stuff_string(engineSubsys, F_NAME);
		sourceId = std::move(engineSubsys);
	}

	if (optional_string("+Move Engine:")) {
		vec3d offset;
		stuff_vec3d(&offset);
		moveEngine = std::move(offset);
	}

	if (optional_string("+Destination Subsystem:")) {
		SCP_string destSubsys;
		stuff_string(destSubsys, F_NAME);
		renameSubsystem = std::move(destSubsys);
	}
}

void VirtualPOFOperationAddEngine::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	auto appendingPM = virtual_pof_build_cache(appendingPOF, depth);
	const auto& engineSubsysMap = appendingPM->deferred().engine_subsystems;

	int engineNumber;
	tl::optional<SCP_string> subsystemName = tl::nullopt;
	typename std::remove_reference<decltype(engineSubsysMap)>::type::const_iterator it = engineSubsysMap.cend();

	if (mpark::holds_alternative<int>(sourceId)) {
		engineNumber = mpark::get<int>(sourceId);

		it = engineSubsysMap.find(engineNumber);
		if (it != engineSubsysMap.cend())
			subsystemName = it->second.subsystem_name;
	}
	else {
		subsystemName = mpark::get<SCP_string>(sourceId);

		auto localit = engineSubsysMap.cbegin();
		do {
			if (subsystem_stricmp(localit->second.subsystem_name.c_str(), subsystemName->c_str()) == 0) {
				if (it != engineSubsysMap.cend()) {
					Warning(LOCATION, "Engine subsystem %s of POF %s for virtual POF %s is ambiguous. Use engine number instead. Returning original POF.", subsystemName->c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
					return;
				}
				it = localit;
			}
		} while (++localit != engineSubsysMap.cend());
		if (it == engineSubsysMap.cend()) {
			Warning(LOCATION, "Failed to find engine subsystem %s of POF %s for virtual POF %s. Returning original POF.", subsystemName->c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
			return;
		}
		engineNumber = it->first;
	}

	int newEngineNumber = reallocate_and_copy_array(pm->thrusters, pm->n_thrusters, 1);
	const SCP_unordered_map<int, int> thrusterReplacementMap{ {engineNumber, newEngineNumber} };

	pm->thrusters[newEngineNumber] = object_copy_including_array_member(appendingPM->pm()->thrusters[engineNumber], &thruster_bank::num_points, &thruster_bank::points);
	
	if (moveEngine) {
		const vec3d& offset = *moveEngine;
		for (int i = 0; i < pm->thrusters[newEngineNumber].num_points; i++)
			pm->thrusters[newEngineNumber].points[i].pnt += offset;
	}

	if (subsystemName) {
		if (renameSubsystem) {
			const SCP_unordered_map<SCP_string, SCP_string> thrusterSubsysRenameMap{ {*subsystemName, *renameSubsystem} };
			deferredTasks.engine_subsystems.emplace(change_submodel_name(
					change_engine_numbers(model_read_deferred_tasks::engine_subsystem_pair(*it), thrusterReplacementMap),
				thrusterSubsysRenameMap));
		}
		else {
			deferredTasks.engine_subsystems.emplace(change_engine_numbers(model_read_deferred_tasks::engine_subsystem_pair(*it), thrusterReplacementMap));
		}
	}
}

VirtualPOFOperationAddGlowpoint::VirtualPOFOperationAddGlowpoint() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Source Glowpoint:");
	stuff_int(&sourceId);
	--sourceId;

	if (optional_string("+Move Glowpoint:")) {
		vec3d offset;
		stuff_vec3d(&offset);
		moveGlowpoint = std::move(offset);
	}

	required_string("+Destination Submodel:");
	stuff_string(renameSubmodel, F_NAME);
}

void VirtualPOFOperationAddGlowpoint::process(polymodel* pm, model_read_deferred_tasks& /*deferredTasks*/, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	auto appendingPM = virtual_pof_build_cache(appendingPOF, depth);

	int dest_subobj_no = model_find_submodel_index(pm, renameSubmodel.c_str());

	if (dest_subobj_no < 0) {
		Warning(LOCATION, "Failed to find submodel %s on virtual POF %s. Returning original POF", renameSubmodel.c_str(), virtualPof.name.c_str());
		return;
	}

	int newGPNumber = reallocate_and_copy_array(pm->glow_point_banks, pm->n_glow_point_banks, 1);
	pm->glow_point_banks[newGPNumber] = object_copy_including_array_member(appendingPM->pm()->glow_point_banks[sourceId], &glow_point_bank::num_points, &glow_point_bank::points);
	pm->glow_point_banks[newGPNumber].submodel_parent = dest_subobj_no;

	appendingPM->keepGlowbank(sourceId);

	if (moveGlowpoint) {
		const vec3d& offset = *moveGlowpoint;
		for (int i = 0; i < pm->glow_point_banks[newGPNumber].num_points; i++)
			pm->glow_point_banks[newGPNumber].points[i].pnt += offset;
	}
}

VirtualPOFOperationAddSpecialSubsystem::VirtualPOFOperationAddSpecialSubsystem() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Source Subsystem:");
	stuff_string(sourceSubsystem, F_NAME);

	if (optional_string("+Destination Subsystem:")) {
		SCP_string dest;
		stuff_string(dest, F_NAME);
		renameSubsystem = std::move(dest);
	}
}

void VirtualPOFOperationAddSpecialSubsystem::process(polymodel* /*pm*/, model_read_deferred_tasks& deferredTasks, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	auto appendingPM = virtual_pof_build_cache(appendingPOF, depth);
	const auto& subsystems = appendingPM->deferred().model_subsystems;

	auto it = subsystems.find(sourceSubsystem);

	if (it == subsystems.end()) {
		Warning(LOCATION, "Failed to find subsystem %s on POF %s for virtual POF %s. Returning original POF", sourceSubsystem.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
		return;
	}

	if (it->second.subobj_nr >= 0) {
		Warning(LOCATION, "Subsystem %s on POF %s for virtual POF %s is a modelled subsystem and not a special point. Returning original POF", sourceSubsystem.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
		return;
	}

	const SCP_string& targetName = renameSubsystem ? *renameSubsystem : sourceSubsystem;
	if (deferredTasks.model_subsystems.find(targetName) != deferredTasks.model_subsystems.end()) {
		Warning(LOCATION, "Subsystem %s already exists on virtual POF %s! Returning original POF", targetName.c_str(), virtualPof.name.c_str());
		return;
	}

	if(renameSubsystem)
		deferredTasks.model_subsystems.emplace(change_submodel_name(model_read_deferred_tasks::model_subsystem_pair(*it), VirtualPOFNameReplacementMap{ {sourceSubsystem, *renameSubsystem} }));
	else
		deferredTasks.model_subsystems.emplace(*it);
}

VirtualPOFOperationAddWeapons::VirtualPOFOperationAddWeapons() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Type:");
	primary = required_string_either("Primary", "Secondary") == 0;

	required_string("+Source Weapon Bank:");
	stuff_int(&sourcebank);

	if (optional_string("+Destination Weapon Bank:"))
		stuff_int(&destbank);	
}

void VirtualPOFOperationAddWeapons::process(polymodel* pm, model_read_deferred_tasks& /*deferredTasks*/, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	auto appendingPM = virtual_pof_build_cache(appendingPOF, depth);

	w_bank*& banks = primary ? pm->gun_banks : pm->missile_banks;
	int& n_banks = primary ? pm->n_guns : pm->n_missiles;

	const w_bank* const& banks_src = primary ? appendingPM->pm()->gun_banks : appendingPM->pm()->missile_banks;
	const int& n_banks_src = primary ? appendingPM->pm()->n_guns : appendingPM->pm()->n_missiles;

	const int& n_banks_max = primary ? MAX_SHIP_PRIMARY_BANKS : MAX_SHIP_SECONDARY_BANKS;
	
	if (sourcebank < 0 || sourcebank >= n_banks_src) {
		Warning(LOCATION, "Source bank %d on POF %s for virtual POF %s does not exist. Returning original POF", sourcebank, appendingPOF.c_str(), virtualPof.name.c_str());
		return;
	}

	int actual_destbank = destbank;

	if (destbank < 0) {
		//Add
		if (n_banks >= n_banks_max) {
			Warning(LOCATION, "No space for additional destination bank on POF %s for virtual POF %s. Returning original POF", pm->filename, virtualPof.name.c_str());
			return;
		}

		actual_destbank = reallocate_and_copy_array(banks, n_banks, 1);
	}
	else {
		//Replace
		if (destbank >= n_banks) {
			Warning(LOCATION, "Destination bank %d on POF %s for virtual POF %s does not exist. Returning original POF", destbank, pm->filename, virtualPof.name.c_str());
			return;
		}
	}

	banks[actual_destbank] = object_copy_including_array_member<w_bank, false>(banks_src[sourcebank], &w_bank::num_slots, &w_bank::pnt, &w_bank::norm, &w_bank::external_model_angle_offset);
}


VirtualPOFOperationAddDockPoint::VirtualPOFOperationAddDockPoint() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Source Dock Point:");
	stuff_string(sourcedock, F_NAME);

	if (optional_string("+Rename Dock Point:")) {
		SCP_string name;
		stuff_string(name, F_NAME);
		renameDock = std::move(name);
	}

	while (optional_string("+Rename Path:")) {
		SCP_string from, to;
		required_string("+From:");
		stuff_string(from, F_NAME);
		required_string("+To:");
		stuff_string(to, F_NAME);
		renamePaths.emplace(std::move(from), std::move(to));
	}

	if (optional_string("+Parent Submodel:")) {
		SCP_string name;
		stuff_string(name, F_NAME);
		targetParentSubsystem = std::move(name);
	}
}
	
void VirtualPOFOperationAddDockPoint::process(polymodel* pm, model_read_deferred_tasks& /*deferredTasks*/, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	const polymodel* appendingPM = virtual_pof_build_cache(appendingPOF, depth)->pm();

	int dockpoint = model_find_dock_name_index(appendingPM, sourcedock.c_str());
	if (dockpoint < 0) {
		Warning(LOCATION, "Could not find dockpoint %s on POF %s for virtual POF %s. Returning original POF", sourcedock.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
		return;
	}

	const SCP_string& targetName = renameDock ? *renameDock : sourcedock;
	if (model_find_dock_name_index(pm, targetName.c_str()) >= 0) {
		Warning(LOCATION, "Dockpoint %s already exists on POF %s for virtual POF %s. Returning original POF", targetName.c_str(), pm->filename, virtualPof.name.c_str());
		return;
	}

	if (appendingPM->docking_bays[dockpoint].parent_submodel >= 0 && !targetParentSubsystem) {
		Warning(LOCATION, "Dockpoint %s must have a parent submodel specified for virtual POF %s. Returning original POF", sourcedock.c_str(), virtualPof.name.c_str());
		return;
	}

	int submodel_index = -1;
	if (targetParentSubsystem) {
		submodel_index = model_find_submodel_index(pm, targetParentSubsystem->c_str());
		if (submodel_index < 0) {
			Warning(LOCATION, "Submodel %s does not exist on POF %s for virtual POF %s. Returning original POF", targetParentSubsystem->c_str(), pm->filename, virtualPof.name.c_str());
			return;
		}
	}

	for (int i = 0; i < appendingPM->docking_bays[dockpoint].num_spline_paths; i++) {
		const auto& originalName = appendingPM->paths[appendingPM->docking_bays[dockpoint].splines[i]].name;
		auto renamed = renamePaths.find(originalName);
		const SCP_string& targetSplineName = renamed == renamePaths.end() ? originalName : renamed->second;

		for (int j = 0; j < pm->n_paths; j++) {
			if (stricmp(targetSplineName.c_str(), pm->paths[j].name) == 0) {
				Warning(LOCATION, "Path %s already exists on POF %s for virtual POF %s. Returning original POF", targetSplineName.c_str(), pm->filename, virtualPof.name.c_str());
				return;
			}
		}
	}

	int destdock = reallocate_and_copy_array_vmalloc(pm->docking_bays, pm->n_docks, 1);
	pm->docking_bays[destdock] = object_copy_including_array_member(appendingPM->docking_bays[dockpoint], &dock_bay::num_spline_paths, &dock_bay::splines);
	int splinefrom = reallocate_and_copy_array_vmalloc(pm->paths, pm->n_paths, pm->docking_bays[destdock].num_spline_paths);
	
	for (int i = 0; i < pm->docking_bays[destdock].num_spline_paths; i++) {
		pm->paths[i + splinefrom] = object_copy_including_array_member(appendingPM->paths[appendingPM->docking_bays[dockpoint].splines[i]], &model_path::nverts, &model_path::verts);
		if (targetParentSubsystem) {
			strcpy_s(pm->paths[i + splinefrom].parent_name, targetParentSubsystem->c_str());
			pm->paths[i + splinefrom].parent_submodel = submodel_index;
		}
		else {
			//Make sure that the path's parent name is cleared in this case
			pm->paths[i + splinefrom].parent_name[0] = '\0';
		}
		auto renamed = renamePaths.find(pm->paths[i + splinefrom].name);
		if (renamed != renamePaths.end()) {
			strcpy_s(pm->paths[i + splinefrom].name, renamed->second.c_str());
		}
		pm->docking_bays[destdock].splines[i] = i + splinefrom;
	}

	if (targetParentSubsystem) {
		pm->docking_bays[destdock].parent_submodel = submodel_index;
	}

	if (renameDock) {
		strcpy_s(pm->docking_bays[destdock].name, renameDock->c_str());

		// the name has changed, so the type_flags should be regenerated;
		// c.f. parsing ID_DOCK in modelread.cpp
		if ( !strnicmp(pm->docking_bays[destdock].name, "cargo", 5) )
			pm->docking_bays[destdock].type_flags = DOCK_TYPE_CARGO;
		else
			pm->docking_bays[destdock].type_flags = (DOCK_TYPE_REARM | DOCK_TYPE_GENERIC);
	}
}


VirtualPOFOperationAddPath::VirtualPOFOperationAddPath() {
	required_string("+POF to Add:");
	stuff_string(appendingPOF, F_FILESPEC);

	required_string("+Source Path:");
	stuff_string(sourcepath, F_NAME);

	if (optional_string("+Rename Path:")) {
		SCP_string name;
		stuff_string(name, F_NAME);
		renamePath = std::move(name);
	}

	if (optional_string("+Parent Submodel:")) {
		SCP_string name;
		stuff_string(name, F_NAME);
		targetParentSubsystem = std::move(name);
	}
}

void VirtualPOFOperationAddPath::process(polymodel* pm, model_read_deferred_tasks& /*deferredTasks*/, model_parse_depth depth, const VirtualPOFDefinition& virtualPof) const {
	const polymodel* appendingPM = virtual_pof_build_cache(appendingPOF, depth)->pm();

	int sourcePathNr = -1;
	for (int i = 0; i < appendingPM->n_paths; i++) {
		if (lcase_equal(sourcepath, appendingPM->paths[i].name)) {
			sourcePathNr = i;
			break;
		}
	}

	const SCP_string& targetName = renamePath ? *renamePath : sourcepath;
	for (int i = 0; i < pm->n_paths; i++) {
		if (lcase_equal(targetName, pm->paths[i].name)) {
			Warning(LOCATION, "Path %s already exists on POF %s for virtual POF %s. Returning original POF", targetName.c_str(), pm->filename, virtualPof.name.c_str());
			return;
		}
	}

	if (sourcePathNr < 0) {
		Warning(LOCATION, "Could not find path %s on POF %s for virtual POF %s. Returning original POF", sourcepath.c_str(), appendingPOF.c_str(), virtualPof.name.c_str());
		return;
	}

	int submodel_index = -1;
	bool clear_parent = false;
	if (targetParentSubsystem) {
		if (lcase_equal(*targetParentSubsystem, "<none>"))
			clear_parent = true;
		else
			submodel_index = model_find_submodel_index(pm, targetParentSubsystem->c_str());
		//Not finding this is okay, it means that the parent is a subsystem, not a submodel. Blame whoever designed this field to be double-function
	}

	if (appendingPM->paths[sourcePathNr].parent_submodel >= 0 && submodel_index < 0) {
		Warning(LOCATION, "Path %s must have a parent submodel specified for virtual POF %s. Returning original POF", sourcepath.c_str(), virtualPof.name.c_str());
		return;
	}

	int destpath = reallocate_and_copy_array_vmalloc(pm->paths, pm->n_paths, 1);
	pm->paths[destpath] = object_copy_including_array_member(appendingPM->paths[sourcePathNr], &model_path::nverts, &model_path::verts);

	if (targetParentSubsystem) {
		if(clear_parent){
			pm->paths[destpath].parent_name[0] = '\0';
			pm->paths[destpath].parent_submodel = -1;
		}
		else {
			strcpy_s(pm->paths[destpath].parent_name, targetParentSubsystem->c_str());
			pm->paths[destpath].parent_submodel = submodel_index;
		}
	}

	if (renamePath) {
		strcpy_s(pm->paths[destpath].name, renamePath->c_str());
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

VirtualPOFOperationChangeSubsystemData::VirtualPOFOperationChangeSubsystemData() {
	required_string("+Subsystem:");
	stuff_string(subsystem, F_NAME);

	if (optional_string("+Set Position:")) {
		vec3d& offset = setPosition.emplace();
		stuff_vec3d(&offset);
	}

	if (optional_string("+Set Radius:")) {
		float& rad = setRadius.emplace();
		stuff_float(&rad);
	}

	if (optional_string("+Set Properties:")) {
		SCP_string& props = setProperties.emplace();
		stuff_string(props, F_MULTITEXT);
	} else if (optional_string("+Append Properties:")) {
		propertyReplace = false;
		SCP_string& props = setProperties.emplace();
		stuff_string(props, F_MULTITEXT);
	}
}

void VirtualPOFOperationChangeSubsystemData::process(polymodel* /*pm*/, model_read_deferred_tasks& deferredTasks, model_parse_depth /*depth*/, const VirtualPOFDefinition& virtualPof) const {
	auto subsys_it = deferredTasks.model_subsystems.find(subsystem);

	if (subsys_it == deferredTasks.model_subsystems.end()){
		Warning(LOCATION, "Failed to find subsystem %s to change data of in virtual POF %s. Returning original POF", subsystem.c_str(), virtualPof.name.c_str());
		return;
	}

	model_read_deferred_tasks::model_subsystem_parse& subsys = subsys_it->second;

	if (setPosition) {
		if (subsys.subobj_nr < 0) {
			subsys.pnt = *setPosition;
		}
		else
			Warning(LOCATION, "Cannot change position of modelled subsystem %s in virtual POF %s. Not modifying position", subsystem.c_str(), virtualPof.name.c_str());
	}

	if (setRadius) {
		if (subsys.subobj_nr < 0) {
			subsys.rad = *setRadius;
		}
		else
			Warning(LOCATION, "Cannot change radius of modelled subsystem %s in virtual POF %s. Not modifying radius", subsystem.c_str(), virtualPof.name.c_str());
	}

	if (setProperties) {
		if (propertyReplace)
			subsys.props = *setProperties;
		else
			subsys.props += '\n' + *setProperties;
	}
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
