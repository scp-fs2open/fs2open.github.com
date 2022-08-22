#include "modelreplace.h"

#include "cfile/cfile.h"

#include <map>
#include <vector>


std::unordered_map<SCP_string, std::vector<VirtualPOFDefinition>> virtual_pofs;

bool model_exists(const SCP_string& filename) {
	auto it = virtual_pofs.find(filename);
	if (it != virtual_pofs.end() && !it->second.empty()) {
		return true;
	}

	return cf_exists_full(filename.c_str(), CF_TYPE_MODELS);
}

bool model_load_virtual(polymodel* pm, const SCP_string& filename, int depth) {
	return false;
}

void parse_virtual_pof_table(const char* filename) {
	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		
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


void VirtualPOFOperationReplaceProps::process(polymodel* pm) const {

}