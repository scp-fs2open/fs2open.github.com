#include "modelreplace.h"

#include "model.h"

#include "cfile/cfile.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

static std::unordered_map<SCP_string, std::vector<VirtualPOFDefinition>> virtual_pofs;
static std::unordered_map<SCP_string, std::function<std::unique_ptr<VirtualPOFOperation>()>> virtual_pof_operations = { 
	{"$Replace Props:", &std::make_unique<VirtualPOFOperationAddSubmodel> }
};

/*
* forward declares for internal modelread functions
*/
extern int read_model_file(polymodel* pm, const char* filename, int ferror, subsystem_parse_list& subsystemParseList, int depth = 0);


// General Functions and external code paths

bool model_exists(const SCP_string& filename) {
	auto it = virtual_pofs.find(filename);
	if (it != virtual_pofs.end() && !it->second.empty()) {
		return true;
	}

	return cf_exists_full(filename.c_str(), CF_TYPE_MODELS);
}

bool read_virtual_model_file(polymodel* pm, const SCP_string& filename, int depth) {
	return false;

	//if return true, increment depth
}

// Parsing function

static void parse_virtual_pof() {

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

#define REPLACE_IF_EQ(data, source, dest) if ((data) == source) (data) = dest;

void change_submodel_numbers(polymodel* pm, int source, int dest) {
	//For now only in the subobject data...
	for (int i = 0; i < pm->n_models; i++) {
		auto& submodel = pm->submodel[i];
		for (auto& detail : submodel.details)
			REPLACE_IF_EQ(detail, source, dest);
		REPLACE_IF_EQ(submodel.first_child, source, dest);
		REPLACE_IF_EQ(submodel.i_replace, source, dest);
		for (auto& debris : submodel.live_debris)
			REPLACE_IF_EQ(debris, source, dest);
		REPLACE_IF_EQ(submodel.look_at_submodel, source, dest);
		REPLACE_IF_EQ(submodel.my_replacement, source, dest);
		REPLACE_IF_EQ(submodel.next_sibling, source, dest);
		REPLACE_IF_EQ(submodel.parent, source, dest);
	}
}

// Actual replacement operations

VirtualPOFOperationAddSubmodel::VirtualPOFOperationAddSubmodel() {

}

void VirtualPOFOperationAddSubmodel::process(polymodel* pm, subsystem_parse_list& subsysList, int depth) const {
	polymodel* appendingPM = new polymodel();
	subsystem_parse_list appendingSubsys;
	std::set<int> keepTextures;
	read_model_file(appendingPM, appendingPOF.c_str(), 0, appendingSubsys, depth);

	int src_subobj_no = -1;
	for (int i = 0; i < appendingPM->n_models; i++) {
		if (!stricmp(appendingPM->submodel[i].name, subobjNameSrc.c_str()))
			src_subobj_no = i;
	}

	int dest_subobj_no = -1;
	for (int i = 0; i < pm->n_models; i++) {
		if (!stricmp(pm->submodel[i].name, subobjNameDest.c_str()))
			src_subobj_no = i;
	}

	if (src_subobj_no >= 0 && dest_subobj_no >= 0) {
		std::vector<int> to_copy_submodels;
		bool has_name_collision = false;
		model_iterate_submodel_tree(appendingPM, src_subobj_no, [&to_copy_submodels, &has_name_collision, pm, appendingPM](int submodel, int /*level*/, bool /*isLeaf*/) {
			to_copy_submodels.emplace_back(submodel);
			for (int i = 0; i < pm->n_models; i++) {
				if (!stricmp(pm->submodel[i].name, appendingPM->submodel[submodel].name))
					has_name_collision = true;
			}
			});

		if (!has_name_collision) {
			//Make sure to keep old data
			bsp_info* oldSubmodels = pm->submodel;
			int old_n_submodel = pm->n_models;

			//Realloc new submodel array of proper size
			pm->n_models += (int)to_copy_submodels.size();
			pm->submodel = new bsp_info[pm->n_models];

			//Copy over old data. Pointers in the struct can still point to old members, we will just delete the outer bsp_info array
			memcpy_s(&pm->submodel, pm->n_models, oldSubmodels, old_n_submodel);
			delete[] oldSubmodels;

			//Modify new data to correct submodel indices. First move all indices to a safe spot where there can be no overlaps, then to the correct spot
			for (const auto& id : to_copy_submodels)
				change_submodel_numbers(appendingPM, id, id + pm->n_models);
			for (int i = 0; i < to_copy_submodels.size(); i++) {
				change_submodel_numbers(appendingPM, to_copy_submodels[i] + pm->n_models, i + old_n_submodel);
				auto it = appendingSubsys.model_subsystems.find(appendingPM->submodel[to_copy_submodels[i]].name);
				if (it != appendingSubsys.model_subsystems.end()) {
					it->second.subobj_nr = i + old_n_submodel;
					subsysList.model_subsystems.emplace(*it);
				}
			}
			
			int deltaDepth = (pm->submodel[dest_subobj_no].depth + 1) - appendingPM->submodel[src_subobj_no].depth;

			//Copy over new data. This one needs to be fully free'd afterwards, so make sure to nullptr the respective pointers before freeing later
			for (int i = 0; i < to_copy_submodels.size(); i++) {
				auto& newSubmodel = pm->submodel[i + old_n_submodel];
				newSubmodel = appendingPM->submodel[to_copy_submodels[i]];

				//Set new Depth
				newSubmodel.depth += deltaDepth;

				//Store texture replacement indices
				//TODO

				//Clear old pointer to data
				appendingPM->submodel[i].bsp_data = nullptr;
			}

			//Register as next child to our destination
			pm->submodel[dest_subobj_no].num_children++;
			int last_sibling = pm->submodel[dest_subobj_no].first_child;
			while (last_sibling >= 0)
				last_sibling = pm->submodel[last_sibling].next_sibling;

			pm->submodel[last_sibling].next_sibling = old_n_submodel; //The selected submodel will always be first, and thus get the first id
			pm->submodel[old_n_submodel].next_sibling = -1; //Our old submodel might've had a sibling. Not anymore.
		}
		else {
			Warning(LOCATION, "Failed to add submodel to virtual POF, original POF already has a subsystem with the same name as was supposed to be added."); //TODO automatic submodel renaming
		}
	}
	else {
		Warning(LOCATION, "Failed to add submodel to virtual POF, specified subobject was not found. Returning original POF");
	}

	
	model_page_out_textures(appendingPM, true);
	model_free(appendingPM);
}