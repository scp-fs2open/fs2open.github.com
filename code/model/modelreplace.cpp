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
extern int read_model_file(polymodel* pm, const char* filename, int ferror, model_read_deferred_tasks& deferredTasks, model_parse_depth depth = {});
extern void create_family_tree(polymodel* obj);
extern void model_calc_bound_box(vec3d* box, vec3d* big_mn, vec3d* big_mx);

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
		operation->process(pm, deferredTasks, depth);

	return true;
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

static void change_submodel_numbers(polymodel* pm, int source, int dest) {
	//For now only in the subobject data...
	//TODO Phase 2 expand to full polymodel
	for (int i = 0; i < pm->n_models; i++) {
		auto& submodel = pm->submodel[i];
		for (auto& detail : submodel.details)
			REPLACE_IF_EQ(detail);
		REPLACE_IF_EQ(submodel.first_child);
		REPLACE_IF_EQ(submodel.i_replace);
		for (auto& debris : submodel.live_debris)
			REPLACE_IF_EQ(debris);
		REPLACE_IF_EQ(submodel.look_at_submodel);
		REPLACE_IF_EQ(submodel.my_replacement);
		REPLACE_IF_EQ(submodel.next_sibling);
		REPLACE_IF_EQ(submodel.parent);
	}
}

#undef REPLACE_IF_EQ

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

void VirtualPOFOperationAddSubmodel::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth depth) const {
	polymodel* appendingPM = new polymodel();
	model_read_deferred_tasks appendingSubsys;
	SCP_set<int> keepTextures;
	read_model_file(appendingPM, appendingPOF.c_str(), 0, appendingSubsys, depth);

	int src_subobj_no = model_find_submodel_index(appendingPM, subobjNameSrc.c_str());
	int dest_subobj_no = model_find_submodel_index(appendingPM, subobjNameDest.c_str());

	if (src_subobj_no >= 0 && dest_subobj_no >= 0) {
		create_family_tree(pm);
		create_family_tree(appendingPM);

		if (rename != nullptr)
			rename->process(appendingPM, appendingSubsys, depth);

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

			//Modify new data to correct submodel indices. First move all indices to a safe spot where there can be no overlaps, then to the correct spot
			for (const auto& id : to_copy_submodels)
				change_submodel_numbers(appendingPM, id, id + pm->n_models);
			for (int i = 0; i < (int)to_copy_submodels.size(); i++) {
				change_submodel_numbers(appendingPM, to_copy_submodels[i] + pm->n_models, i + old_n_submodel);
				auto it = appendingSubsys.model_subsystems.find(appendingPM->submodel[to_copy_submodels[i]].name);
				if (it != appendingSubsys.model_subsystems.end()) {
					it->second.subobj_nr = i + old_n_submodel;
					deferredTasks.model_subsystems.emplace(*it);
				}
			}
			
			int deltaDepth = (pm->submodel[dest_subobj_no].depth + 1) - appendingPM->submodel[src_subobj_no].depth;

			SCP_map<int, int> textureIDReplace;

			//Copy over new data. This one needs to be fully free'd afterwards, so make sure to nullptr the respective pointers before freeing later
			for (int i = 0; i < (int)to_copy_submodels.size(); i++) {
				auto& newSubmodel = pm->submodel[i + old_n_submodel];
				newSubmodel = appendingPM->submodel[to_copy_submodels[i]];

				//Set new Depth
				newSubmodel.depth += deltaDepth;

				//Store texture replacement indices
				const auto& textureIDs = model_get_textures_used(appendingPM, to_copy_submodels[i]);
				for (const auto& textureID : textureIDs) {
					auto it = textureIDReplace.find(textureID);
					if (it == textureIDReplace.end()) {
						int newID = pm->n_textures++;
						if (pm->n_textures > MAX_MODEL_TEXTURES) {
							Warning(LOCATION, "Failed to add submodel to virtual POF, combined POF has too many (over %d) textures.", MAX_MODEL_TEXTURES);
							model_page_out_textures(appendingPM, true);
							model_free(appendingPM);
							return;
						}
						it = textureIDReplace.emplace(textureID, newID).first;
					}
					deferredTasks.texture_replacements[i + old_n_submodel].replacementIds[textureID] = it->second;
				}

				//Clear old pointer to data
				appendingPM->submodel[to_copy_submodels[i]].bsp_data = nullptr;
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
				keepTextures.emplace(usedTexture.first);
			}
		}
		else {
			Warning(LOCATION, "Failed to add submodel to virtual POF, original POF already has a subsystem with the same name as was supposed to be added.");
		}
	}
	else {
		Warning(LOCATION, "Failed to add submodel to virtual POF, specified subobject was not found. Returning original POF");
	}

	
	model_page_out_textures(appendingPM, true, keepTextures);
	model_free(appendingPM);
}

VirtualPOFOperationRenameSubobjects::VirtualPOFOperationRenameSubobjects() {
	while (optional_string("+Replace:")) {
		SCP_string replace;
		stuff_string(replace, F_NAME);
		SCP_tolower(replace);
		required_string("+With:");
		stuff_string(replacements[replace], F_NAME);
	}
}

void VirtualPOFOperationRenameSubobjects::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth /*depth*/) const {
	for (int i = 0; i < pm->n_models; i++) {
		auto it = replacements.find(pm->submodel[i].name);
		if (it != replacements.end()) {
			strncpy(pm->submodel[i].name, it->second.c_str(), it->second.size());
			pm->submodel[i].name[it->second.size()] = '\0';
		}
		it = replacements.find(pm->submodel[i].lod_name);
		if (it != replacements.end()) {
			strncpy(pm->submodel[i].lod_name, it->second.c_str(), it->second.size());
			pm->submodel[i].lod_name[it->second.size()] = '\0';
		}
	}

	auto copy_model_subsys = deferredTasks.model_subsystems;
	for (const auto& replace : replacements) {
		deferredTasks.model_subsystems.erase(replace.first);
	}
	for (const auto& replace : replacements) {
		auto it = copy_model_subsys.find(replace.first);
		if (it != copy_model_subsys.end())
			deferredTasks.model_subsystems.emplace(replace.second, it->second);
	}

	auto copy_engine_subsys = deferredTasks.engine_subsystems;
	for (const auto& replace : replacements) {
		deferredTasks.engine_subsystems.erase(replace.first);
	}
	for (const auto& replace : replacements) {
		auto it = copy_engine_subsys.find(replace.first);
		if (it != copy_engine_subsys.end())
			deferredTasks.engine_subsystems.emplace(replace.second, it->second);
	}
}

VirtualPOFOperationChangeData::VirtualPOFOperationChangeData() {
	required_string("+Submodel:");
	stuff_string(submodel, F_NAME);
	SCP_tolower(submodel);

	if (optional_string("+Set Offset:")) {
		setOffset = make_unique<vec3d>();
		stuff_vec3d(setOffset.get());
	}
}

void VirtualPOFOperationChangeData::process(polymodel* pm, model_read_deferred_tasks& deferredTasks, model_parse_depth /*depth*/) const {
	int subobj_no = model_find_submodel_index(pm, submodel.c_str());

	if (subobj_no == -1) {
		Warning(LOCATION, "Failed to find submodel to change data of. Returning original POF");
		return;
	}

	if (setOffset != nullptr) {
		pm->submodel[subobj_no].offset = *setOffset;
		auto it = deferredTasks.model_subsystems.find(submodel);
		if (it != deferredTasks.model_subsystems.end()) {
			it->second.pnt = *setOffset;
		}
	}
}

VirtualPOFOperationHeaderData::VirtualPOFOperationHeaderData() {
	if (optional_string("+Set Radius:")) {
		radius = make_unique<float>();
		stuff_float(radius.get());
	}

	if (optional_string("$Set Bounding Box:")) {
		boundingbox = make_unique<std::pair<vec3d, vec3d>>();
		required_string("+Minimum:");
		stuff_vec3d(&boundingbox->first);
		required_string("+Maximum:");
		stuff_vec3d(&boundingbox->second);
	}
}

void VirtualPOFOperationHeaderData::process(polymodel* pm, model_read_deferred_tasks& /*deferredTasks*/, model_parse_depth /*depth*/) const {
	if (radius != nullptr) {
		pm->rad = pm->core_radius = *radius;
	}

	if (boundingbox != nullptr) {
		pm->mins = boundingbox->first;
		pm->maxs = boundingbox->second;
		model_calc_bound_box(pm->bounding_box, &pm->mins, &pm->maxs);
	}
}