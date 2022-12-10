
#include "ai/ailua.h"

#include "parse/sexp/sexp_lookup.h"

#include "parse/parselo.h"
#include "parse/sexp.h"
#include "parse/sexp/LuaSEXP.h"
#include "parse/sexp/LuaAISEXP.h"
#include "scripting/scripting.h"

#include <memory>

namespace {
using namespace sexp;

struct global_state {
	// Track if we already have called our init function
	bool initialized = false;
	// Before we are initialized it is not safe to add sexps to the system so we add them to a pending list which we
	// process while initializing
	SCP_vector<std::unique_ptr<DynamicSEXP>> pending_sexps;

	SCP_unordered_map<int, std::unique_ptr<sexp::DynamicSEXP>> operator_const_mapping;
	SCP_unordered_map<int, int> subcategory_to_category;

	// These IDs are now just incremented
	int next_free_operator_id = First_available_operator_id;
	int next_free_category_id = First_available_category_id;
	int next_free_subcategory_id = First_available_subcategory_id;
};

// Static initialization to avoid initialization order issues
global_state& globals()
{
	static global_state state;
	return state;
}

void parse_sexp_table(const char* filename) {
	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// These characters may not appear in a SEXP name
		constexpr const char* INVALID_CHARS = "()\"'\t ";
		if (optional_string("#Lua SEXPs")) {

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
		}

		if (optional_string("#Lua AI")) {
			while (optional_string("$Operator:")) {
				SCP_string name;
				stuff_string(name, F_NAME);

				if (std::strpbrk(name.c_str(), INVALID_CHARS) != nullptr) {
					error_display(0, "Found invalid AI-SEXP name '%s'!", name.c_str());

					// Skip the invalid entry
					skip_to_start_of_string_either("$Operator:", "#End");
					continue;
				}
				if (get_operator_index(name.c_str()) >= 0) {
					error_display(0, "The AI-SEXP '%s' is already defined!", name.c_str());

					// Skip the invalid entry
					skip_to_start_of_string_either("$Operator:", "#End");
					continue;
				}

				std::unique_ptr<DynamicSEXP> instance(new LuaAISEXP(name));
				auto luaSexp = static_cast<LuaAISEXP*>(instance.get());
				luaSexp->parseTable();

				int op = add_dynamic_sexp(std::move(instance), sexp_oper_type::GOAL);
				if (op >= 0) {
					luaSexp->registerAIMode(op);
					luaSexp->maybeRegisterPlayerOrder(op);
				}
			}
			required_string("#End");
		}

	} catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void free_lua_sexps(lua_State* /*L*/)
{
	auto& global = globals();

	// Remove all Lua sexps from our list so that there are no dangling pointers on the lua state
	for (auto iter = global.operator_const_mapping.begin(); iter != global.operator_const_mapping.end();) {
		auto lua_sexp = dynamic_cast<LuaSEXP*>(iter->second.get());
		if (lua_sexp == nullptr) {
			++iter;
			continue;
		}

		iter = global.operator_const_mapping.erase(iter);
	}
}

} // namespace

namespace sexp {

int add_dynamic_sexp(std::unique_ptr<DynamicSEXP>&& sexp, sexp_oper_type type)
{
	auto& global = globals();

	if (!global.initialized) {
		// Do nothing now and delay this until we know that we are properly initialized
		global.pending_sexps.emplace_back(std::move(sexp));
		return -1;
	}

	sexp->initialize();

	auto name = sexp->getName();
	auto help_text = sexp->getHelpText();

	sexp_oper new_op;
	new_op.text = name;
	new_op.min = sexp->getMinimumArguments();
	new_op.max = sexp->getMaximumArguments();

	int free_op_index = global.next_free_operator_id++;

	if (Operators.size() >= FIRST_OP) {
		Warning(LOCATION, "There are too many total SEXPs.  The SEXP %s will not be added.", sexp->getName().c_str());
		return -1;
	}

	// For now, all dynamic SEXPS are only valid in missions
	new_op.value = free_op_index;
	new_op.type = type;

	global.operator_const_mapping.insert(std::make_pair(new_op.value, std::move(sexp)));

	// Now actually add the operator to the SEXP containers
	Operators.push_back(new_op);

	sexp_help_struct new_help;
	new_help.id = new_op.value;
	new_help.help = help_text;
	Sexp_help.push_back(new_help);

	return new_op.value;
}
DynamicSEXP* get_dynamic_sexp(int operator_const)
{
	auto& global = globals();

	auto iter = global.operator_const_mapping.find(operator_const);
	if (iter == global.operator_const_mapping.end()) {
		return nullptr;
	}

	return iter->second.get();
}
int get_category_of_subcategory(int subcategory_id)
{
	const auto& global = globals();

	auto iter = global.subcategory_to_category.find(subcategory_id);
	if (iter == global.subcategory_to_category.end()) {
		return OP_CATEGORY_NONE;
	}

	return iter->second;
}
void dynamic_sexp_init()
{
	auto& global = globals();
	global.initialized = true;

	// Add built-in subcategories
	for (auto& item : op_submenu) {
		int category = category_of_subcategory(item.id);
		global.subcategory_to_category.emplace(item.id, category);
	}

	// Add pending sexps now when it is safe to do so
	for (auto&& pending : global.pending_sexps) {
		add_dynamic_sexp(std::move(pending));
	}
	global.pending_sexps.clear();

	parse_modular_table("*-sexp.tbm", parse_sexp_table, CF_TYPE_TABLES);

	Script_system.OnStateDestroy.add(free_lua_sexps);
}
void dynamic_sexp_shutdown()
{
	auto& global = globals();

	global.operator_const_mapping.clear();
	global.subcategory_to_category.clear();

	global.next_free_operator_id = First_available_operator_id;
	global.next_free_category_id = First_available_category_id;
	global.next_free_subcategory_id = First_available_subcategory_id;

	global.initialized = false;
}
int add_category(const SCP_string& name)
{
	auto& global = globals();

	int category_id = global.next_free_category_id++;
	op_menu.push_back({ name, category_id });
	return category_id;
}
int add_subcategory(int parent_category, const SCP_string& name)
{
	auto& global = globals();

	int subcategory_id = global.next_free_subcategory_id++;
	op_submenu.push_back({ name, subcategory_id });
	global.subcategory_to_category.emplace(subcategory_id, parent_category);
	return subcategory_id;
}

}
