
#include "ai/ailua.h"
#include "mission/missionmessage.h"
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
	int next_free_enum_list_id = First_available_opf_id;
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

		if (optional_string("#Lua Enums")) {
			while (optional_string("$Name:")) {
				SCP_string name;
				stuff_string(name, F_NAME);

				if (get_dynamic_enum_position(name) >= 0) {
					error_display(0, "Lua Sexp Enum %s already exists! Skipping!\n", name.c_str());
					continue;
				}


				dynamic_sexp_enum_list thisList;
				thisList.name = std::move(name);

				while (optional_string("+Enum:")) {
					SCP_string item;
					stuff_string(item, F_NAME);

					// These characters may not appear in an Enum item
					constexpr const char* ENUM_INVALID_CHARS = "()\"'\\/";
					if (std::strpbrk(item.c_str(), ENUM_INVALID_CHARS) != nullptr) {
						error_display(0, "ENUM item '%s' cannot include these characters [(,),\",',\\,/]. Skipping!\n", item.c_str());

						// Skip the invalid entry
						continue;

					}

					if (item.length() >= NAME_LENGTH) {
						error_display(0, "Enum item '%s' is longer than %i characters. Truncating!\n", item.c_str(), NAME_LENGTH);
						item.resize(NAME_LENGTH - 1);
					}

					bool skip = false;
					// Case insensitive check if the item already exists in the list
					for (int i = 0; i < (int)thisList.list.size(); i++) {
						if (lcase_equal(item, thisList.list[i])) {
							error_display(0, "Enum item '%s' already exists in list %s. Skipping!\n", item.c_str(), thisList.name.c_str());
							skip = true;
							break;
						}
					}

					if (skip)
						continue;

					thisList.list.push_back(item);
				}

				if (thisList.list.size() > 0) {
					Dynamic_enums.push_back(std::move(thisList));
					increment_enum_list_id();
				} else {
					error_display(0, "Parsed empty enum list '%s'. Ignoring!\n", thisList.name.c_str());
				}
			}
			required_string("#End");
		}

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

int operator_upper_bound()
{
	return globals().next_free_operator_id;
}

int add_dynamic_sexp(std::unique_ptr<DynamicSEXP>&& sexp, sexp_oper_type type)
{
	auto& global = globals();

	if (!global.initialized) {
		// Do nothing now and delay this until we know that we are properly initialized
		global.pending_sexps.emplace_back(std::move(sexp));
		return -1;
	}

	sexp->initialize();

	sexp_oper new_op;
	new_op.text = sexp->getName();
	new_op.min = sexp->getMinimumArguments();
	new_op.max = sexp->getMaximumArguments();

	int free_op_index = global.next_free_operator_id++;

	if (Operators.size() >= FIRST_OP) {
		Warning(LOCATION, "There are too many total SEXPs.  The SEXP %s will not be added.", new_op.text.c_str());
		return -1;
	}

	// sanity check
	int subcategory = sexp->getSubcategory();
	if (subcategory != OP_SUBCATEGORY_NONE)
	{
		int category = sexp->getCategory();
		int implied_category = category_of_subcategory(subcategory);

		if (category != implied_category)
			Warning(LOCATION, "Operator %s has a category that is not a parent of its subcategory!", new_op.text.c_str());
	}

	// For now, all dynamic SEXPS are only valid in missions
	new_op.value = free_op_index;
	new_op.type = type;

	sexp_help_struct new_help;
	new_help.id = free_op_index;
	new_help.help = sexp->getHelpText();

	global.operator_const_mapping.insert(std::make_pair(free_op_index, std::move(sexp)));

	// Now actually add the operator to the SEXP containers
	Operators.push_back(std::move(new_op));
	Sexp_help.push_back(std::move(new_help));

	return free_op_index;
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
int get_category(const SCP_string& name)
{
	for (auto& cat : op_menu) {
		if (lcase_equal(cat.name, name)) {
			return cat.id;
		}
	}

	return OP_CATEGORY_NONE;
}
int get_subcategory(const SCP_string& name, int category)
{
	for (auto& subcat : op_submenu) {
		if (lcase_equal(subcat.name, name) && get_category_of_subcategory(subcat.id) == category) {
			return subcat.id;
		}
	}

	return OP_SUBCATEGORY_NONE;
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
int increment_enum_list_id()
{
	auto& global = globals();
	return global.next_free_enum_list_id++;
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

	message_types_init();
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
