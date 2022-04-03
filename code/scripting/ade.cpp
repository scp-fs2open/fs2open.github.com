//
//

#include "scripting/ade.h"

#include "globalincs/version.h"

#include "ade.h"

#include "ade_api.h"
#include "ade_args.h"
#include "doc_parser.h"
#include "scripting.h"
#include "scripting_doc.h"

#include "def_files/def_files.h"
#include "mod_table/mod_table.h"
#include "scripting/api/objs/asteroid.h"
#include "scripting/api/objs/beam.h"
#include "scripting/api/objs/debris.h"
#include "scripting/api/objs/object.h"
#include "scripting/api/objs/ship.h"
#include "scripting/api/objs/waypoint.h"
#include "scripting/api/objs/weapon.h"
#include "scripting/lua/LuaFunction.h"
#include "ship/ship.h"

namespace {
using namespace scripting;

//1: Userspace variables (ie in object table)
//2: Handle-specific values
//3: Entries in metatable (ie defined by ADE)
//4: Virtual variables
//5: Use the indexer, if possible
//6: Set userspace variable
//7: Set handle-specific variables
//X: Mission failed.
//
//On the stack when this is called:
//Index 1 - Object (Can be anything with Lua 5.1; Number to a library)
//Index 2 - String (ie the key we're trying to access; Object.string, Object:string, Object['string'], etc)
//Index 3 - (Optional) Argument we are trying to set Object.String = Argument
int ade_index_handler(lua_State* L) {
	Assert(L != NULL);

	const int obj_ldx = 1;
	const int key_ldx = 2;
	const int arg_ldx = 3;
	int last_arg_ldx = lua_gettop(L);
	const char* type_name = NULL;
	uint ade_id = UINT_MAX;
	int mtb_ldx = INT_MAX;
	ade_table_entry* entry = 0;

	//*****STEP 1: Check for user-defined objects
	if (lua_istable(L, obj_ldx) && !ADE_SETTING_VAR) {
		lua_pushvalue(L, key_ldx);
		lua_rawget(L, obj_ldx);
		if (!lua_isnil(L, -1)) {
			return 1;
		} else
			lua_pop(L, 1);    //nil value
	}

	//*****STEP 1.5: Set-up metatable
	if (lua_getmetatable(L, obj_ldx)) {
		mtb_ldx = lua_gettop(L);
		lua_pushcfunction(L, ade_friendly_error);
		int err_ldx = lua_gettop(L);
		int i;

		//*****WMC - go for the type name
		lua_pushstring(L, "__adeid");
		lua_rawget(L, mtb_ldx);
		if (lua_isnumber(L, -1)) {
			ade_id = (uint) lua_tonumber(L, -1);
			if (ade_id < ade_manager::getInstance()->getNumEntries()) {
				entry = &ade_manager::getInstance()->getEntry(ade_id);
				type_name = entry->Name;
			}
		}
		lua_pop(L, 1);

		//*****STEP 2: Check for __ademember objects (ie defaults)
		lua_pushstring(L, "__ademembers");
		lua_rawget(L, mtb_ldx);
		if (lua_istable(L, -1)) {
			int amt_ldx = lua_gettop(L);
			lua_pushvalue(L, key_ldx);
			lua_rawget(L, amt_ldx);
			if (!lua_isnil(L, -1)) {
				return 1;
			} else
				lua_pop(L, 1);    //nil value
		}
		lua_pop(L, 1);    //member table

		//*****STEP 3: Check for virtual variables
		lua_pushstring(L, "__virtvars");
		lua_rawget(L, mtb_ldx);
		if (lua_istable(L, -1)) {
			//Index virtvar function
			int vvt_ldx = lua_gettop(L);
			lua_pushvalue(L, key_ldx);
			lua_rawget(L, vvt_ldx);
			if (lua_isfunction(L, -1)) {
				//Set upvalue
				lua_pushvalue(L, lua_upvalueindex(ADE_SETTING_UPVALUE_INDEX));
				if (lua_setupvalue(L, -2, ADE_SETTING_UPVALUE_INDEX) == NULL) {
					LuaError(L, "Unable to set upvalue for virtual variable");
				}

				//Set arguments
				//WMC - Skip setting the key
				lua_pushvalue(L, obj_ldx);
				int numargs = 1;
				for (i = arg_ldx; i <= last_arg_ldx; i++) {
					lua_pushvalue(L, i);
					numargs++;
				}

				//Execute function
				lua_pcall(L, numargs, LUA_MULTRET, err_ldx);

				return (lua_gettop(L) - vvt_ldx);
			} else {
				lua_pop(L, 1);    //non-function value
			}
		}
		lua_pop(L, 1);    //virtvar table

		//*****STEP 4: Use the indexer
		//NOTE: Requires metatable from step 1.5

		//Get indexer
		lua_pushstring(L, "__indexer");
		lua_rawget(L, mtb_ldx);
		if (lua_isfunction(L, -1)) {
			//Function already on stack
			//Set upvalue
			lua_pushvalue(L, lua_upvalueindex(ADE_SETTING_UPVALUE_INDEX));
			if (lua_setupvalue(L, -2, ADE_SETTING_UPVALUE_INDEX) == NULL) {
				LuaError(L, "Unable to set upvalue for indexer");
			}

			//Set arguments
			for (i = 1; i <= last_arg_ldx; i++) {
				lua_pushvalue(L, i);
			}

			//Execute function
			lua_pcall(L, last_arg_ldx, LUA_MULTRET, err_ldx);

			return (lua_gettop(L) - err_ldx);
		}
		lua_pop(L, 2);    //WMC - Don't need __indexer or error handler
	}

	//*****STEP 6: Set a new variable or die.
	if (ADE_SETTING_VAR && lua_istable(L, obj_ldx)) {
		lua_pushvalue(L, key_ldx);
		lua_pushvalue(L, arg_ldx);
		lua_rawset(L, obj_ldx);

		lua_pushvalue(L, key_ldx);
		lua_rawget(L, obj_ldx);
		return 1;
	}
	lua_pop(L, 1);    //WMC - metatable

	if (type_name != nullptr) {
		LuaError(L, "Could not find index '%s' in type '%s'", lua_tostring(L, key_ldx), type_name);
	} else {
		LuaError(L, "Could not find index '%s'", lua_tostring(L, key_ldx));
	}
	return 0;
}

ade_table_entry& getTableEntry(size_t idx) {
	return ade_manager::getInstance()->getEntry(idx);
}

//*************************Lua operators*************************
//These are the various types of operators you can
//set in Lua. Use these as function name to activate.
//
//Format string should be "*o" or "o*", where "*" is the type of
//variable you want to deal with.
//The order varies with order of variables
string_conv ade_Operators[] = {
	{"__add",		"+"},			//var +  obj
	{"__sub",		"-"},			//var -  obj
	{"__mul",		"*"},			//var *  obj
	{"__div",		"/"},			//var /  obj
	{"__mod",		"%"},			//var %  obj
	{"__pow",		"^"},			//var ^  obj
	{"__unm",		"~"},			//var ~  obj
	{"__concat",	".."},			//var .. obj
	{"__len",		"#"},			//#var
	{"__eq",		"=="},			//var == obj
	{"__lt",		"<"},			//var <  obj
	{"__le",		"<="},			//var <= obj
	{"__newindex",	"="},			//var =  obj
	{"__call",		""},			//*shrug*
	{"__gc",		"__gc"},		//Lua's equivelant of a destructor
	//WMC - Used with tostring() scripting operator.
	{"__tostring",	"(string)"},	//tostring(var)
	//WMC - This is NOT a Lua type, but for the LUA_INDEXER define
	{"__indexer",	"[]"},			//obj[var]
};

//WMC - Sometimes this gets out of sync between Lua versions
static const char *Lua_type_names[] = {
	"nil",
	"boolean",
	"light userdata",
	"number",
	"string",
	"table",
	"function",
	"userdata",
	"thread",
	"any", // Special type which means any possible type
	"void",
};

const ptrdiff_t Lua_type_names_num = std::distance(std::begin(Lua_type_names), std::end(Lua_type_names));

}

namespace scripting {

ade_manager* ade_manager::getInstance() {
	static std::unique_ptr<ade_manager> manager(new ade_manager());
	return manager.get();
}
ade_manager::ade_manager() {
	_type_names.insert(_type_names.end(), std::begin(Lua_type_names), std::end(Lua_type_names));
}
size_t ade_manager::addTableEntry(const ade_table_entry& entry) {
	_table_entries.push_back(entry);
	_table_entries.back().Idx = _table_entries.size() - 1;

	if (entry.Type == 'o' && !entry.Instanced) {
		// Collect valid type names
		_type_names.push_back(entry.Name);
	}

	return _table_entries.size() - 1;
}
ade_table_entry& ade_manager::getEntry(size_t idx) {
	Assertion(idx < _table_entries.size(), "Invalid index " SIZE_T_ARG " specified!", idx);
	return _table_entries[idx];
}
const ade_table_entry& ade_manager::getEntry(size_t idx) const
{
	Assertion(idx < _table_entries.size(), "Invalid index " SIZE_T_ARG " specified!", idx);
	return _table_entries[idx];
}
const SCP_vector<SCP_string>& ade_manager::getTypeNames() const { return _type_names; }

static int deprecatedFunctionHandler(lua_State* L)
{
	const char* functionName = lua_tostring(L, lua_upvalueindex(1));
	LuaError(L,
			 "Deprecated function '%s' has been called that is not available in the targeted engine version. Check "
			 "the documentation for a possible replacement.",
			 functionName);
	return 0;
}

ade_table_entry::ade_table_entry() { memset(Subentries, 0, sizeof(Subentries)); }
// Think of n_mtb_ldx as the parent metatable
int ade_table_entry::SetTable(lua_State* L, int p_amt_ldx, int p_mtb_ldx)
{
	uint i;
	int cleanup_items = 0;
	int mtb_ldx       = INT_MAX;
	int data_ldx      = INT_MAX;
	int desttable_ldx = INT_MAX;
	int amt_ldx       = INT_MAX;

	if (Instanced) {
		//Set any actual data
		int nset = 0;
		switch (Type) {
		case 'o': {
			// Create an empty userdata value and use it as the value for this library
			lua_newuserdata(L, 0);
			// Create or get object metatable
			luaL_getmetatable(L, Name);
			// Set the metatable for the object
			lua_setmetatable(L, -2);
			nset++;
			break;
		}
		case 'u':
		case 'v': {
			if (DeprecationVersion.isValid() &&
				mod_supports_version(DeprecationVersion.major, DeprecationVersion.minor, DeprecationVersion.build)) {
				// The deprecation mechanism. Set a function that always errors instead.
				if (Name != nullptr) {
					lua_pushstring(L, Name);
				} else if (ShortName != nullptr) {
					lua_pushstring(L, ShortName);
				} else {
					lua_pushliteral(L, "<UNNAMED FUNCTION>");
				}
				lua_pushcclosure(L, deprecatedFunctionHandler, 1);
			} else {
				// WMC - This hack by taylor is a necessary evil.
				// 64-bit function pointers do not get passed properly
				// when using va_args for some reason.
				lua_pushstring(L, "<UNNAMED FUNCTION>");
				lua_pushboolean(L, 0);
				lua_pushcclosure(L, Function, 2);
			}
			nset++;
			break;
		}

		default:
			UNREACHABLE("Unhandled value type '%c'!", Type);
			break;
		}

		if (nset) {
			data_ldx = lua_gettop(L);
		} else {
			LuaError(L, "ade_table_entry::SetTable - Could not set data for '%s' (" SIZE_T_ARG ")", GetName(), Idx);
		}

		if (data_ldx != INT_MAX) {
			// Remove data once we are done
			cleanup_items++;

			//WMC - Handle virtual variables by getting their table
			if (Type == 'v') {
				//Get virtvars table
				lua_pushstring(L, "__virtvars");
				lua_rawget(L, p_mtb_ldx);
				if (lua_istable(L, -1)) {
					cleanup_items++;

					//Virtual variables are stored in virtvar table,
					//rather than the parent table
					desttable_ldx = lua_gettop(L);
				} else {
					lua_pop(L, 1);
				}
			} else {
				//WMC - Member objects prefixed with __ are assumed to be metatable objects
				if (strnicmp("__", GetName(), 2) != 0 && lua_istable(L, p_amt_ldx)) {
					desttable_ldx = p_amt_ldx;
				} else if (lua_istable(L, p_mtb_ldx)) {
					desttable_ldx = p_mtb_ldx;
				}
			}

			if (desttable_ldx != INT_MAX) {
				//If we are setting a function...
				if (lua_isfunction(L, data_ldx)) {
					//Set the FIRST upvalue to its name,
					//so we can always find out what it is for debugging
					lua_pushstring(L, GetName());
					if (lua_setupvalue(L, data_ldx, 1) == NULL) {
						LuaError(L,
								 "ade_table_entry::SetTable - Could not set upvalue for '%s' (" SIZE_T_ARG ")",
								 GetName(),
								 Idx);
					}
				}

				//Register name and shortname
				if (Name != NULL) {
					lua_pushstring(L, Name);
					lua_pushvalue(L, data_ldx);
					lua_rawset(L, desttable_ldx);
				}
				if (ShortName != NULL) {
					lua_pushstring(L, ShortName);
					lua_pushvalue(L, data_ldx);
					lua_rawset(L, desttable_ldx);
				}
			} else {
				LuaError(L, "ade_table_entry::SetTable - Could not instance '%s' (" SIZE_T_ARG ")", GetName(), Idx);
			}
		}
	}

	//If subentries, create a metatable pointer and set it
	if (Num_subentries
		|| (DerivatorIdx != UINT_MAX && ade_manager::getInstance()->getEntry(DerivatorIdx).Num_subentries)) {
		//Create the new metatable
		if (!luaL_newmetatable(L, Name)) {
			LuaError(L,
					 "ade_table_entry::SetTable - Couldn't create metatable for table entry '%s' - does a Lua object already exist with this name?",
					 Name);
			return 0;
		}
		mtb_ldx = lua_gettop(L);
		cleanup_items++;

		//Push a copy of the metatable and set it for this object
		//WMC - Make sure it's instanced, too. This helps keep crashes from happening...
		if (data_ldx != INT_MAX) {
			lua_pushvalue(L, mtb_ldx);
			lua_setmetatable(L, data_ldx);
		}

		//***Create index handler entry
		lua_pushstring(L, "__index");
		lua_pushstring(L, "ade_index_handler(get)");    //upvalue(1) = function name
		lua_pushboolean(L, 0);                            //upvalue(2) = setting true/false
		lua_pushcclosure(L, ade_index_handler, 2);
		lua_rawset(L, mtb_ldx);

		//***Create newindex handler entry
		lua_pushstring(L, "__newindex");
		lua_pushstring(L, "ade_index_handler(set)");    //upvalue(1) = function name
		lua_pushboolean(L, 1);                            //upvalue(2) = setting true/false
		lua_pushcclosure(L, ade_index_handler, 2);
		lua_rawset(L, mtb_ldx);

		if (Destructor != nullptr) {
			// Set up the destructor of this type if it exists
			lua_pushstring(L, "__gc");
			lua_pushfstring(L, "%s Destructor", GetName()); // upvalue(1) = function name
			lua_pushboolean(L, 0);                          // upvalue(2) = setting true/false
			lua_pushlightuserdata(L, Destructor_upvalue);   // upvalue(3) = Reference to ade_obj
			lua_pushcclosure(L, Destructor, 3);
			lua_rawset(L, mtb_ldx);
		}

		//***Create virtvar storage facility
		lua_pushstring(L, "__virtvars");
		lua_newtable(L);
		lua_rawset(L, mtb_ldx);

		//***Create ade members table
		lua_createtable(L, 0, (int) Num_subentries);
		if (lua_istable(L, -1)) {
			//WMC - was lua_gettop(L) - 1 for soem
			amt_ldx = lua_gettop(L);
			cleanup_items++;

			//Set it
			lua_pushstring(L, "__ademembers");
			lua_pushvalue(L, amt_ldx);    //dup
			lua_rawset(L, mtb_ldx);
		}

		//***Create ID entries
		lua_pushstring(L, "__adeid");
		lua_pushnumber(L, static_cast<lua_Number>(Idx));
		lua_rawset(L, mtb_ldx);

		if (DerivatorIdx != UINT_MAX) {
			lua_pushstring(L, "__adederivid");
			lua_pushinteger(L, DerivatorIdx);
			lua_rawset(L, mtb_ldx);
		}
	}

	if (amt_ldx != INT_MAX) {
		//Fill out ze metatable
		if (DerivatorIdx != UINT_MAX) {
			for (i = 0; i < ade_manager::getInstance()->getEntry(DerivatorIdx).Num_subentries; i++) {
				ade_manager::getInstance()->getEntry(ade_manager::getInstance()->getEntry(DerivatorIdx).Subentries[i]).SetTable(
					L,
					amt_ldx,
					mtb_ldx);
			}
		}
		for (i = 0; i < Num_subentries; i++) {
			ade_manager::getInstance()->getEntry(Subentries[i]).SetTable(L, amt_ldx, mtb_ldx);
		}
	}

	//Pop the metatable and data (cleanup)
	lua_pop(L, cleanup_items);

	return 1;
}
size_t ade_table_entry::AddSubentry(ade_table_entry& n_ate) {
	ade_table_entry ate = n_ate;
	ate.ParentIdx = Idx;
	size_t new_idx = ade_manager::getInstance()->addTableEntry(ate);

	//WMC - Oi. Moving the Ade_table_entries vector
	//invalidates the "this" pointer. Workaround time.
	auto new_this = &ade_manager::getInstance()->getEntry(ate.ParentIdx);
	size_t idx = new_this->Num_subentries++;
	new_this->Subentries[idx] = new_idx;

	return new_idx;
}

std::unique_ptr<DocumentationElement> ade_table_entry::ToDocumentationElement(
	const scripting::DocumentationErrorReporter& errorReporter)
{
	using namespace scripting;

	if (Name == nullptr && ShortName == nullptr) {
		Warning(LOCATION, "Data entry with no name or shortname");
		return std::unique_ptr<DocumentationElement>();
	}

	std::unique_ptr<DocumentationElement> element;

	//***Begin entry
	switch (Type) {
	case 'o': {
		if (!Instanced) {
			// Classes
			std::unique_ptr<DocumentationElementClass> obj(new DocumentationElementClass());
			obj->type = ElementType::Class;

			if (DerivatorIdx != UINT_MAX) {
				obj->superClass = getTableEntry(DerivatorIdx).GetName();
			}

			element = std::move(obj);
		} else {
			// Libraries
			std::unique_ptr<DocumentationElement> obj(new DocumentationElement());
			obj->type = ElementType::Library;

			element = std::move(obj);
		}
		break;
	}
	case 'u': {
		// Functions
		std::unique_ptr<DocumentationElementFunction> obj(new DocumentationElementFunction());

		auto ao = ade_get_operator(Name);
		if (ao != nullptr) {
			obj->type = ElementType::Operator;
		} else {
			obj->type = ElementType::Function;
		}

		auto typeNames = ade_manager::getInstance()->getTypeNames();

		if (ReturnType != nullptr) {
			type_parser type_parser(typeNames);
			if (type_parser.parse(ReturnType)) {
				obj->returnType = type_parser.getType();
			} else {
				if (errorReporter) {
					errorReporter(GetFullPath() + "(Return Type):\n" + type_parser.getErrorMessage());
				}
				obj->returnType = ade_type_info();
			}
		} else {
			obj->returnType = ade_type_info();
		}

		for (const auto& overload : Arguments.overloads()) {
			DocumentationElementFunction::argument_list overloadArgList;
			argument_list_parser arg_parser(typeNames);
			if (arg_parser.parse(overload)) {
				bool optional = false;
				for (const auto& arg : arg_parser.getArgList()) {
					if (!optional && (arg.optional || !arg.def_val.empty())) {
						optional = true;
					}

					auto argCopy     = arg;
					argCopy.optional = optional;

					overloadArgList.arguments.push_back(std::move(argCopy));
				}
			} else {
				if (errorReporter) {
					errorReporter(GetFullPath() + "(Arguments):\n" + arg_parser.getErrorMessage());
				}
				overloadArgList.simple.assign(overload);
			}

			obj->overloads.push_back(overloadArgList);
		}

		if (ReturnDescription != nullptr) {
			obj->returnDocumentation = ReturnDescription;
		}

		element = std::move(obj);
		break;
	}
	case 'v': {
		std::unique_ptr<DocumentationElementProperty> obj(new DocumentationElementProperty());
		obj->type = ElementType::Property;

		auto typeNames = ade_manager::getInstance()->getTypeNames();

		//***Type Name(ShortName)
		if (ReturnType != nullptr) {
			type_parser type_parser(typeNames);
			if (type_parser.parse(ReturnType)) {
				obj->getterType = type_parser.getType();
			} else {
				if (errorReporter) {
					errorReporter(GetFullPath() + "(Getter Type):\n" + type_parser.getErrorMessage());
				}
				obj->getterType = ade_type_info();
			}
		} else {
			obj->getterType = ade_type_info();
		}

		const SCP_string setterType = Arguments.overloads().front();
		if (!setterType.empty()) {
			type_parser type_parser(typeNames);
			if (type_parser.parse(setterType)) {
				obj->setterType = type_parser.getType();
			} else {
				if (errorReporter) {
					errorReporter(GetFullPath() + "(Setter Type):\n" + type_parser.getErrorMessage());
				}
				obj->setterType = ade_type_info();
			}
		} else {
			obj->setterType = ade_type_info();
		}

		if (ReturnDescription != nullptr) {
			obj->returnDocumentation = ReturnDescription;
		}

		element = std::move(obj);
		break;
	}
	case 'b':
	case 'd':
	case 'f':
	case 'i':
	case 's':
	case 'x':
	default:
		UNREACHABLE("Unknown Type %c was used used!", Type);
		break;
	}

	if (Name != nullptr) {
		element->name = Name;
	}
	if (ShortName != nullptr) {
		element->shortName = ShortName;
	}
	if (Description != nullptr) {
		element->description = Description;
	}

	if (DeprecationVersion.isValid()) {
		element->deprecationVersion = DeprecationVersion;

		if (DeprecationMessage != nullptr) {
			element->deprecationMessage = DeprecationMessage;
		}
	}

	for (uint32_t i = 0; i < Num_subentries; i++) {
		element->children.emplace_back(getTableEntry(Subentries[i]).ToDocumentationElement(errorReporter));
	}

	return element;
}
const char* ade_table_entry::GetName() const
{
	if (Name != nullptr) {
		return Name;
	} else {
		return ShortName;
	}
}
SCP_string ade_table_entry::GetFullPath() const
{
	SCP_string path(GetName());

	size_t currentIdx = ParentIdx;
	while (currentIdx != UINT_MAX) {
		const auto entry = ade_manager::getInstance()->getEntry(currentIdx);

		SCP_string entryName(entry.GetName());

		// Not very efficient but should not be used at runtime so this is fine
		path.insert(path.begin(), '.');
		path.insert(path.begin(), entryName.cbegin(), entryName.cend());

		currentIdx = entry.ParentIdx;
	}

	return path;
}

ade_lib::ade_lib(const char* in_name, const ade_lib_handle* parent, const char* in_shortname, const char* in_desc) {
	ade_table_entry ate;

	ate.Name = in_name;
	ate.ShortName = in_shortname;
	ate.Instanced = true;

	//WMC - Here's a little hack.
	//Lua did not work with __len on standard table objects.
	//So instead, all FS2 libraries are now userdata.
	//This means that no new functions can be added from
	//within the scripting environment, but I don't think
	//there will be any catastrophic consequences.
	ate.Type = 'o';
	ate.Description = in_desc;

	if(parent != NULL)
		LibIdx = getTableEntry(parent->GetIdx()).AddSubentry(ate);
	else
		LibIdx = ade_manager::getInstance()->addTableEntry(ate);
}

const char *ade_lib::GetName() const
{
	if(GetIdx() == UINT_MAX)
		return "<Invalid>";

	return getTableEntry(GetIdx()).GetName();
}

ade_func::ade_func(const char* name,
	lua_CFunction func,
	const ade_lib_handle& parent,
	ade_overload_list args,
	const char* desc,
	const char* ret_type,
	const char* ret_desc,
	const gameversion::version& deprecation_version,
	const char* deprecation_message)
{
	Assertion(strcmp(name, "__gc") != 0, "__gc is a reserved function name! An API function may not use it!");

	ade_table_entry ate;

	ate.Name               = name;
	ate.Instanced          = true;
	ate.Type               = 'u';
	ate.Function           = func;
	ate.Arguments          = std::move(args);
	ate.Description        = desc;
	ate.ReturnType         = ret_type;
	ate.ReturnDescription  = ret_desc;
	ate.DeprecationVersion = deprecation_version;
	ate.DeprecationMessage = deprecation_message;

	LibIdx = ade_manager::getInstance()->getEntry(parent.GetIdx()).AddSubentry(ate);
}

ade_virtvar::ade_virtvar(const char* name,
	lua_CFunction func,
	const ade_lib_handle& parent,
	const char* args,
	const char* desc,
	const char* ret_type,
	const char* ret_desc,
	const gameversion::version& deprecation_version,
	const char* deprecation_message)
{
	Assertion(strcmp(name, "__gc") != 0, "__gc is a reserved function name! An API function may not use it!");

	ade_table_entry ate;

	ate.Name               = name;
	ate.Instanced          = true;
	ate.Type               = 'v';
	ate.Function           = func;
	ate.Arguments          = args;
	ate.Description        = desc;
	ate.ReturnType         = ret_type;
	ate.ReturnDescription  = ret_desc;
	ate.DeprecationVersion = deprecation_version;
	ate.DeprecationMessage = deprecation_message;

	LibIdx = ade_manager::getInstance()->getEntry(parent.GetIdx()).AddSubentry(ate);
}

ade_indexer::ade_indexer(lua_CFunction func,
	const ade_lib_handle& parent,
	ade_overload_list overloads,
	const char* desc,
	const char* ret_type,
	const char* ret_desc)
{
	// Add function for meta
	ade_table_entry ate;

	ate.Name              = "__indexer";
	ate.Instanced         = true;
	ate.Type              = 'u';
	ate.Function          = func;
	ate.Arguments         = std::move(overloads);
	ate.Description       = desc;
	ate.ReturnType        = ret_type;
	ate.ReturnDescription = ret_desc;

	LibIdx = ade_manager::getInstance()->getEntry(parent.GetIdx()).AddSubentry(ate);
}

//*************************Lua functions*************************
//WMC - Spits out the current Lua stack to "stackdump"
//This includes variable values, but not names
void ade_stackdump(lua_State *L, char *stackdump)
{
	char buf[512];
	int stacksize = lua_gettop(L);

	//Lua temps
	double d;
	int b;
	char *s;
	for(int argnum = 1; argnum <= stacksize; argnum++)
	{
		int type = lua_type(L, argnum);
		sprintf(buf, "\r\n%d: ", argnum);
		strcat(stackdump, buf);
		switch(type)
		{
			case LUA_TNIL:
				strcat(stackdump, "NIL");
				break;
			case LUA_TNUMBER:
				d = lua_tonumber(L, argnum);
				sprintf(buf, "Number [%f]",d);
				strcat(stackdump, buf);
				break;
			case LUA_TBOOLEAN:
				b = lua_toboolean(L, argnum);
				sprintf(buf, "Boolean [%d]",b);
				strcat(stackdump, buf);
				break;
			case LUA_TSTRING:
				s = (char *)lua_tostring(L, argnum);
				sprintf(buf, "String [%s]",s);
				strcat(stackdump, buf);
				break;
			case LUA_TTABLE:
			{
				if(lua_getmetatable(L, argnum))
				{
					lua_pushstring(L, "__adeid");
					lua_rawget(L, -2);
					if(lua_isnumber(L, -1))
					{
						sprintf(buf, "Table [%s]", getTableEntry((size_t)lua_tonumber(L, -1)).Name);
						strcat(stackdump, buf);
					}
					else
						strcat(stackdump, "non-default Table");
					lua_pop(L, 2);	//metatable and nil/adeid
				}
				else
					strcat(stackdump, "Table w/ no metatable");

				//Maybe get first key?
				char *firstkey = NULL;
				lua_pushnil(L);
				if(lua_next(L, argnum))
				{
					firstkey = (char *)lua_tostring(L, -2);
					if(firstkey != NULL)
					{
						strcat(stackdump, ", First key: [");
						strcat(stackdump, firstkey);
						strcat(stackdump, "]");
					}
					lua_pop(L, 1);	//Key
				}
				lua_pop(L, 1);	//Nil
			}
				break;
			case LUA_TFUNCTION:
				strcat(stackdump, "Function");
				{
					char *upname = (char*)lua_getupvalue(L, argnum, ADE_FUNCNAME_UPVALUE_INDEX);
					if(upname != NULL)
					{
						strcat(stackdump, " ");
						strcat(stackdump, lua_tostring(L, -1));
						strcat(stackdump, "()");
						lua_pop(L, 1);
					}
				}
				break;
			case LUA_TUSERDATA:
				if(lua_getmetatable(L, argnum))
				{
					lua_pushstring(L, "__adeid");
					lua_rawget(L, -2);
					if(lua_isnumber(L, -1))
					{
						sprintf(buf, "Userdata [%s]", getTableEntry((size_t)lua_tonumber(L, -1)).Name);
					}
					else
						sprintf(buf, "non-default Userdata");

					lua_pop(L, 2);	//metatable and nil/adeid
				}
				else
					sprintf(buf, "Userdata w/ no metatable");
				strcat(stackdump, buf);
				break;
			case LUA_TTHREAD:
				sprintf(buf, "Thread");
				strcat(stackdump, buf);
				break;
			case LUA_TLIGHTUSERDATA:
				sprintf(buf, "Light userdata");
				strcat(stackdump, buf);
				break;
			default:
				sprintf(buf, "<UNKNOWN>: %s (%f) (%s)", lua_typename(L, type), lua_tonumber(L, argnum), lua_tostring(L, argnum));
				strcat(stackdump, buf);
				break;
		}
	}
}

int ade_friendly_error(lua_State *L)
{
	LuaError(L);

	//WMC - According to documentation, this will always be the error
	//if error handler is called
	return LUA_ERRRUN;
}

bool ade_is_internal_type(const char* typeName) {
	for (int i = 0; i < Lua_type_names_num; i++) {
		if (!stricmp(Lua_type_names[i], typeName)) {
			return true;
		}
	}

	return false;
}

//WMC - Gets type of object
const char *ade_get_type_string(lua_State *L, int argnum)
{
	int type = lua_type(L, argnum);

	if(type < 0 || type >= Lua_type_names_num)
		return "Unknown";

	return Lua_type_names[type];
}

ade_odata_setter<object_h> ade_object_to_odata(int obj_idx)
{
	using namespace scripting::api;

	if(obj_idx < 0 || obj_idx >= MAX_OBJECTS)
		return l_Object.Set(object_h());

	object *objp = &Objects[obj_idx];

	switch(objp->type)
	{
	case OBJ_SHIP:
		return l_Ship.Set(object_h(objp));
	case OBJ_ASTEROID:
		return l_Asteroid.Set(object_h(objp));
	case OBJ_DEBRIS:
		return l_Debris.Set(object_h(objp));
	case OBJ_WAYPOINT:
		return l_Waypoint.Set(object_h(objp));
	case OBJ_WEAPON:
		return l_Weapon.Set(object_h(objp));
	case OBJ_BEAM:
		return l_Beam.Set(object_h(objp));
	default:
		return l_Object.Set(object_h(objp));
	}
}

int ade_set_object_with_breed(lua_State *L, int obj_idx)
{
	return ade_set_args(L, "o", ade_object_to_odata(obj_idx));
}

void load_default_script(lua_State* L, const char* name)
{
	using namespace luacpp;

	SCP_string source;
	SCP_string source_name;
	if (Enable_external_default_scripts && cf_exists(name, CF_TYPE_SCRIPTS)) {
		// Load from disk (or built-in file)
		source_name = name;

		auto cfp = cfopen(name, "rb", CFILE_NORMAL, CF_TYPE_SCRIPTS);
		Assertion(cfp != nullptr, "Failed to open default file!");

		auto length = cfilelength(cfp);

		source.resize((size_t)length);
		cfread(&source[0], 1, length, cfp);

		cfclose(cfp);
	} else {
		// Load from default files
		source_name = SCP_string("default ") + name;

		auto def = defaults_get_file(name);

		auto c = reinterpret_cast<const char*>(def.data);
		source.assign(c, c + def.size);
	}

	try {
		auto func = LuaFunction::createFromCode(L, source, source_name);
		func.setErrorFunction(LuaFunction::createFromCFunction(L, ade_friendly_error));
		try {
			func(L);
		} catch (const LuaException&) {
			// The execution of the function may also throw an exception but that should have been handled by a LuaError
			// before
		}
	} catch (const LuaException& e) {
		LuaError(L, "Error while loading default script: %s", e.what());
	}
}

const string_conv* ade_get_operator(const char *funcname)
{
	for(auto iter = std::begin(ade_Operators); iter != std::end(ade_Operators); ++iter)
	{
		if(!strcmp(funcname, iter->src))
			return &(*iter);
	}

	return nullptr;
}

ade_table_entry& internal::getTableEntry(size_t idx) { return ade_manager::getInstance()->getEntry(idx); }

}
