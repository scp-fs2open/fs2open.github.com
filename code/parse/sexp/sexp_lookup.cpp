
#include "parse/sexp/sexp_lookup.h"
#include "parse/sexp/LuaSEXP.h"

#include "parse/sexp.h"
#include "parse/parselo.h"

#include <memory>

namespace {
using namespace sexp;

SCP_unordered_map<int, std::unique_ptr<sexp::DynamicSEXP>> operator_const_mapping;

// This contains which operator is the next free operator in a specific category
SCP_unordered_map<int, int> next_free_operator_mapping;

int get_next_free_operator(int category) {
	Assertion(category != OP_CATEGORY_CHANGE, "The primary change category is full so it can't be used for new operators!");

	auto iter = next_free_operator_mapping.find(category);

	if (iter == next_free_operator_mapping.end()) {
		// The next operator number isn't initialized yet so we initialize that here
		int max_op_value = 0;
		for (auto& oper : Operators) {
			// We need to do some ugly bit masking here since the SEXP system uses different bytes of the operator number for storing various things
			if ((oper.value & OP_CATEGORY_MASK) == category) {
				// This operator has the right category
				// We only store the number part which is the lowest byte of the operator number
				max_op_value = std::max(max_op_value, oper.value & 0xFF);
			}
		}
		iter = next_free_operator_mapping.insert(std::make_pair(category, max_op_value + 1)).first;
	}

	auto value = iter->second;
	iter->second = value + 1; // Update the value inside the iterator
	return value;
}

void parse_sexp_table(const char* filename) {
	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		required_string("#Lua SEXPs");

		// These characters may not appear in a SEXP name
		const char* INVALID_CHARS = "()\"'\t ";

		while (optional_string("$Operator:")) {
			SCP_string name;
			stuff_string(name, F_NAME);

			if (std::strpbrk(name.c_str(), INVALID_CHARS) != nullptr) {
				error_display(0, "Found invalid SEXP name '%s'!", name.c_str());

				// Skip the invalid entry
				skip_to_start_of_string_either("$Operator:", "#End");
				continue;
			}
			if (get_operator_index(name.c_str()) >= 0) {
				error_display(0, "The SEXP '%s' is already defined!", name.c_str());

				// Skip the invalid entry
				skip_to_start_of_string_either("$Operator:", "#End");
				continue;
			}

			std::unique_ptr<DynamicSEXP> instance(new LuaSEXP(name));
			auto luaSexp = static_cast<LuaSEXP*>(instance.get());
			luaSexp->parseTable();

			add_dynamic_sexp(std::move(instance));
		}

		required_string("#End");
	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

}

namespace sexp {

void add_dynamic_sexp(std::unique_ptr<DynamicSEXP>&& sexp) {
	auto name = sexp->getName();
	auto help_text = sexp->getHelpText();

	sexp_oper new_op;
	new_op.text = name;
	new_op.min = sexp->getMinimumArguments();
	new_op.max = sexp->getMaximumArguments();

	// For now, all dynamic SEXPS are only valid in missions
	new_op.value = get_next_free_operator(sexp->getCategory()) | sexp->getCategory() | OP_NONCAMPAIGN_FLAG;
	new_op.type = SEXP_ACTION_OPERATOR;

	operator_const_mapping.insert(std::make_pair(new_op.value, std::move(sexp)));

	// Now actually add the operator to the SEXP containers
	Operators.push_back(new_op);

	sexp_help_struct new_help;
	new_help.id = new_op.value;
	new_help.help = help_text;
	Sexp_help.push_back(new_help);
}
DynamicSEXP* get_dynamic_sexp(int operator_const) {
	auto iter = operator_const_mapping.find(operator_const);
	if (iter == operator_const_mapping.end()) {
		return nullptr;
	}

	return iter->second.get();
}
void dynamic_sexp_init() {
	parse_modular_table("*-sexp.tbm", parse_sexp_table, CF_TYPE_TABLES);
}
void dynamic_sexp_shutdown() {
	operator_const_mapping.clear();
}
int add_subcategory(int parent_category, const SCP_string& name) {
	// Another hack to make sure change2 is interpreted as the normal change category
	if (parent_category == OP_CATEGORY_CHANGE2) {
		parent_category = OP_CATEGORY_CHANGE;
	}

	int max_subcategory_value = 0;
	for (auto& subcat : op_submenu) {
		if ((subcat.id & OP_CATEGORY_MASK) == parent_category) {
			max_subcategory_value = std::max(max_subcategory_value, subcat.id & SUBCATEGORY_MASK);
		}
	}

	if (max_subcategory_value >= SUBCATEGORY_MASK) {
		// No more entries left in this subcategory!
		return -1;
	}

	int subcategory_id = (max_subcategory_value + 1) | parent_category;
	op_submenu.push_back({ name, subcategory_id });
	return subcategory_id;
}

}
