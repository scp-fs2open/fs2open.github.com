#include "modelreplace.h"

#include "cfile/cfile.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

static std::unordered_map<SCP_string, std::vector<VirtualPOFDefinition>> virtual_pofs;
static std::unordered_map<SCP_string, std::function<std::unique_ptr<VirtualPOFOperation>()>> virtual_pof_operations = { 
	{"$Replace Props:", &std::make_unique<VirtualPOFOperationReplaceProps> }
};

/*
* forward declares for internal modelread functions
*/
extern void set_subsystem_info(int model_num, model_subsystem* subsystemp, const char* props, const char* dname);




bool model_exists(const SCP_string& filename) {
	auto it = virtual_pofs.find(filename);
	if (it != virtual_pofs.end() && !it->second.empty()) {
		return true;
	}

	return cf_exists_full(filename.c_str(), CF_TYPE_MODELS);
}

bool model_read_virtual(polymodel* pm, const SCP_string& filename, int depth) {
	return false;
}

void parse_virtual_pof() {

}

void parse_virtual_pof_table(const char* filename) {
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


VirtualPOFOperationReplaceProps::VirtualPOFOperationReplaceProps() {

}

void VirtualPOFOperationReplaceProps::process(polymodel* pm) const {
	int submodel = model_find_submodel_index(pm->id, subobjName.c_str());
}