//
//

#include "scripting/ade.h"
#include "ship/ship.h"
#include "ade_api.h"

#include <memory>

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

		//*****STEP 2: Check for handle signature-specific values
		if (lua_isuserdata(L, obj_ldx) && ade_id != UINT_MAX && !ADE_SETTING_VAR) {
			//WMC - I assume char is one byte
			static_assert(sizeof(char) == 1, "char needs to have size 1!");

			//Get userdata sig
			char* ud = (char*) lua_touserdata(L, obj_ldx);
			ODATA_SIG_TYPE sig = *(ODATA_SIG_TYPE*) (ud + entry->Value.Object.size);

			//Now use it to index the table with that #
			lua_pushnumber(L, sig);
			lua_rawget(L, mtb_ldx);
			if (lua_istable(L, -1)) {
				int hvt_ldx = lua_gettop(L);
				lua_pushvalue(L, key_ldx);
				lua_rawget(L, hvt_ldx);
				if (!lua_isnil(L, -1)) {
					return 1;
				} else
					lua_pop(L, 1);    //nil value
			}
			lua_pop(L, 1);    //sig table
		}

		//*****STEP 3: Check for __ademember objects (ie defaults)
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

		//*****STEP 4: Check for virtual variables
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

		//*****STEP 5: Use the indexer
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
		//*****STEP 7: Set sig thingie
	else if (ADE_SETTING_VAR && ade_id != UINT_MAX && mtb_ldx != INT_MAX && lua_isuserdata(L, obj_ldx)) {
		//WMC - I assume char is one byte
		Assert(sizeof(char) == 1);

		//Get userdata sig
		char* ud = (char*) lua_touserdata(L, obj_ldx);
		ODATA_SIG_TYPE sig = *(ODATA_SIG_TYPE*) (ud + entry->Value.Object.size);

		//Now use it to index the table with that #
		lua_pushnumber(L, sig);
		lua_rawget(L, mtb_ldx);

		//Create table, if necessary
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			lua_newtable(L);
			lua_pushnumber(L, sig);
			lua_pushvalue(L, -2);
			lua_rawset(L, mtb_ldx);
		}

		//Index the table
		if (lua_istable(L, -1)) {
			int hvt_ldx = lua_gettop(L);
			lua_pushvalue(L, key_ldx);
			lua_pushvalue(L, arg_ldx);
			lua_rawset(L, hvt_ldx);

			lua_pushvalue(L, key_ldx);
			lua_rawget(L, hvt_ldx);
			return 1;
		}
		lua_pop(L, 1);    //WMC - maybe-sig-table
	}
	lua_pop(L, 1);    //WMC - metatable

	if (type_name != NULL) {
		LuaError(L, "Could not find index '%s' in type '%s'", lua_tostring(L, key_ldx), type_name);
	} else {
		LuaError(L, "Could not find index '%s'", lua_tostring(L, key_ldx));
	}
	return 0;
}

ade_table_entry& getTableEntry(size_t idx) {
	return ade_manager::getInstance()->getEntry(idx);
}

//Struct for converting one string for another. whee!
struct string_conv {
	const char *src;
	const char *dest;
};

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

int ade_get_operator(const char *tablename)
{
	for(auto iter = std::begin(ade_Operators); iter != std::end(ade_Operators); ++iter)
	{
		if(!strcmp(tablename, iter->src))
			return (int)std::distance(std::begin(ade_Operators), iter);
	}

	return -1;
}

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
};

const ptrdiff_t Lua_type_names_num = std::distance(std::begin(Lua_type_names), std::end(Lua_type_names));

void ade_output_type_link(FILE *fp, const char *typestr)
{
	for(int i = 0; i < Lua_type_names_num; i++)
	{
		if(!stricmp(Lua_type_names[i], typestr))
		{
			fputs(typestr, fp);
			return;
		}
	}

	fprintf(fp, "<a href=\"#%s\">%s</a>", typestr, typestr);
}
}

namespace scripting {

ade_manager* ade_manager::getInstance() {
	static std::unique_ptr<ade_manager> manager(new ade_manager());
	return manager.get();
}
size_t ade_manager::addTableEntry(const ade_table_entry& entry) {
	_table_entries.push_back(entry);
	_table_entries.back().Idx = _table_entries.size() - 1;
	return _table_entries.size() - 1;
}
ade_table_entry& ade_manager::getEntry(size_t idx) {
	Assertion(idx < _table_entries.size(), "Invalid index "
		SIZE_T_ARG
		" specified!", idx);
	return _table_entries[idx];
}
const ade_table_entry& ade_manager::getEntry(size_t idx) const {
	Assertion(idx < _table_entries.size(), "Invalid index "
		SIZE_T_ARG
		" specified!", idx);
	return _table_entries[idx];
}

ade_table_entry::ade_table_entry()
	: Name(NULL), ShortName(NULL), Idx(UINT_MAX), ParentIdx(UINT_MAX), DerivatorIdx(UINT_MAX), Instanced(false),
	  Size(0), Arguments(NULL), Description(NULL), ReturnType(NULL), ReturnDescription(NULL), Num_subentries(0) {
	Type = '\0';
	memset(Subentries, 0, sizeof(Subentries));
}
//Think of n_mtb_ldx as the parent metatable
int ade_table_entry::SetTable(lua_State* L, int p_amt_ldx, int p_mtb_ldx) {
	uint i;
	int cleanup_items = 0;
	int mtb_ldx = INT_MAX;
	int data_ldx = INT_MAX;
	int desttable_ldx = INT_MAX;
	int amt_ldx = INT_MAX;

	if (Instanced) {
		//Set any actual data
		int nset = 0;
		switch (Type) {
			//WMC - This hack by taylor is a necessary evil.
			//64-bit function pointers do not get passed properly
			//when using va_args for some reason.
			case 'u':
			case 'v':
				lua_pushstring(L, "<UNNAMED FUNCTION>");
				lua_pushboolean(L, 0);
				lua_pushcclosure(L, Value.Function, 2);
				nset++;
				break;

			default:
				char typestr[2] = {
					Type,
					'\0'
				};
				nset = ade_set_args(L, typestr, Value);
				break;
		}

		if (nset) {
			data_ldx = lua_gettop(L);
		} else {
			LuaError(L, "ade_table_entry::SetTable - Could not set data for '%s' (" SIZE_T_ARG ")", GetName(), Idx);
		}

		if (data_ldx != INT_MAX) {
			//WMC - Cannot delete libs and stuff off here.
			if (p_amt_ldx != LUA_GLOBALSINDEX) {
				cleanup_items++;
			}

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
				if (strnicmp("__", GetName(), 2) && lua_istable(L, p_amt_ldx)) {
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

void ade_table_entry::OutputMeta(FILE *fp)
{
	if(Name == NULL && ShortName == NULL) {
		Warning(LOCATION, "Data entry with no name or shortname");
		return;
	}

	uint i;
	bool skip_this = false;

	//WMC - Hack
	if(ParentIdx != UINT_MAX)
	{
		for(i = 0; i < Num_subentries; i++)
		{
			if(!stricmp(ade_manager::getInstance()->getEntry(Subentries[i]).Name, "__indexer"))
			{
				ade_manager::getInstance()->getEntry(Subentries[i]).OutputMeta(fp);
				skip_this = true;
				break;
			}
		}
	}


	//***Begin entry
	if(!skip_this)
	{
		fputs("<dd><dl>", fp);

		switch(Type)
		{
			case 'o':
			{
				//***Name (ShortName)
				if(ParentIdx == UINT_MAX) {
					fprintf(fp, "<dt id=\"%s\">", GetName());
				}
				if(Name == NULL)
				{
					if(DerivatorIdx == UINT_MAX)
						fprintf(fp, "<h2>%s</h2>\n", ShortName);
					else
						fprintf(fp, "<h2>%s:<a href=\"#%s\">%s</a></h2>\n", getTableEntry(DerivatorIdx).GetName(), getTableEntry(DerivatorIdx).GetName(), getTableEntry(DerivatorIdx).GetName());
				}
				else
				{
					fprintf(fp, "<h2>%s", Name);

					if(ShortName != NULL)
						fprintf(fp, " (%s)", ShortName);
					if(DerivatorIdx != UINT_MAX)
						fprintf(fp, ":<a href=\"#%s\">%s</a>", getTableEntry(DerivatorIdx).GetName(), getTableEntry(DerivatorIdx).GetName());

					fputs("</h2>\n", fp);
				}
				fputs("</dt>\n", fp);

				//***Description
				if(Description != NULL) {
					fprintf(fp, "<dd>%s</dd>\n", this->Description);
				}

				//***Type: ReturnType
				if(ReturnType != NULL) {
					fprintf(fp, "<dd><b>Return Type: </b> %s<br>&nbsp;</dd>\n", ReturnType);
				}

				if(ReturnDescription != NULL) {
					fprintf(fp, "<dd><b>Return Description: </b> %s<br>&nbsp;</dd>\n", ReturnDescription);
				}
			}
				break;
			case 'u':
			{
				//***Name(ShortName)(Arguments)
				fputs("<dt>", fp);
				int ao = -1;

				fputs("<i>", fp);
				if(ReturnType != NULL)
					ade_output_type_link(fp, ReturnType);
				else
					fputs("nil", fp);
				fputs(" </i>", fp);
				if(Name == NULL) {
					fprintf(fp, "<b>%s", ShortName);
				}
				else
				{
					ao = ade_get_operator(Name);

					//WMC - Do we have an operator?
					if(ao < 0)
					{
						fprintf(fp, "<b>%s", Name);
						if(ShortName != NULL)
						{
							int ao2 = ade_get_operator(ShortName);
							if(ao2 < 0)
								fprintf(fp, "(%s)", ShortName);
							else
								fprintf(fp, "(%s)", ade_Operators[ao2].dest);
						}
					}
					else
					{
						//WMC - Hack
						if(ParentIdx != UINT_MAX && getTableEntry(ParentIdx).ParentIdx != UINT_MAX && getTableEntry(ParentIdx).Name != NULL && !stricmp(Name, "__indexer"))
						{
							fprintf(fp, "<b>%s%s", getTableEntry(ParentIdx).Name, ade_Operators[ao].dest);
						}
						else
							fprintf(fp, "<b>%s", ade_Operators[ao].dest);
					}
				}
				if(ao < 0)
				{
					if(Arguments != NULL) {
						fprintf(fp, "(</b><i>%s</i><b>)</b>\n", Arguments);
					} else {
						fprintf(fp, "()</b>\n");
					}
				}
				else
				{
					fputs("</b>", fp);
					if(Arguments != NULL) {
						fprintf(fp, " <i>%s</i>\n", Arguments);
					}
				}
				fputs("</dt>\n", fp);

				//***Description
				if(Description != NULL) {
					fprintf(fp, "<dd>%s</dd>\n", Description);
				}

				//***Result: ReturnDescription
				if(ReturnDescription != NULL) {
					fprintf(fp, "<dd><b>Returns:</b> %s<br>&nbsp;</dd>\n", ReturnDescription);
				} else {
					fputs("<dd><b>Returns:</b> Nothing<br>&nbsp;</dd>\n", fp);
				}
			}
				break;
			default:
				Warning(LOCATION, "Unknown type '%c' passed to ade_table_entry::OutputMeta", Type);
			case 'b':
			case 'd':
			case 'f':
			case 'i':
			case 's':
			case 'x':
			case 'v':
			{
				//***Type Name(ShortName)
				fputs("<dt>\n", fp);
				if(ReturnType != NULL)
				{
					fputs("<i>", fp);
					ade_output_type_link(fp, ReturnType);
					fputs(" </i>", fp);
				}

				if(Name == NULL) {
					fprintf(fp, "<b>%s</b>\n", ShortName);
				}
				else
				{
					fprintf(fp, "<b>%s", Name);
					if(ShortName != NULL)
						fputs(ShortName, fp);
					fputs("</b>\n", fp);
				}
				if(Arguments != NULL)
					fprintf(fp, " = <i>%s</i>", Arguments);
				fputs("</dt>\n", fp);

				//***Description
				if(Description != NULL)
					fprintf(fp, "<dd>%s</dd>\n", Description);

				//***Also settable with: Arguments
				if(ReturnDescription != NULL)
					fprintf(fp, "<dd><b>Value:</b> %s</b></dd>\n", ReturnDescription);
			}
				break;
		}
	}

	fputs("<dd><dl>\n", fp);
	for(i = 0; i < Num_subentries; i++)
	{
		if(ParentIdx == UINT_MAX || stricmp(getTableEntry(Subentries[i]).Name, "__indexer"))
			getTableEntry(Subentries[i]).OutputMeta(fp);
	}
	fputs("</dl></dd>\n", fp);

	if(!skip_this)
		fputs("<br></dl></dd>\n", fp);
}

ade_lib::ade_lib(const char* in_name, ade_lib_handle* parent, const char* in_shortname, const char* in_desc) {
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
	ate.Value.Object.idx = ade_manager::getInstance()->getNumEntries();
	ate.Value.Object.sig = NULL;
	ate.Value.Object.buf = &Num_reinforcements;	//WMC - I just chose Num_ship_classes randomly. MageKing17 - changed to Num_reinforcements, likewise at random, due to the removal of Num_ship_classes
	ate.Value.Object.size = sizeof(Num_reinforcements);
	ate.Description = in_desc;

	if(parent != NULL)
		LibIdx = getTableEntry(parent->GetIdx()).AddSubentry(ate);
	else
		LibIdx = ade_manager::getInstance()->addTableEntry(ate);
}

const char *ade_lib::GetName()
{
	if(GetIdx() == UINT_MAX)
		return "<Invalid>";

	return getTableEntry(GetIdx()).GetName();
}

ade_func::ade_func(const char* name,
				   lua_CFunction func,
				   ade_lib_handle& parent,
				   const char* args,
				   const char* desc,
				   const char* ret_type,
				   const char* ret_desc) {
	ade_table_entry ate;

	ate.Name = name;
	ate.Instanced = true;
	ate.Type = 'u';
	ate.Value.Function = func;
	ate.Arguments = args;
	ate.Description = desc;
	ate.ReturnType = ret_type;
	ate.ReturnDescription = ret_desc;

	LibIdx = ade_manager::getInstance()->getEntry(parent.GetIdx()).AddSubentry(ate);
}

ade_virtvar::ade_virtvar(const char* name,
						 lua_CFunction func,
						 ade_lib_handle& parent,
						 const char* args,
						 const char* desc,
						 const char* ret_type,
						 const char* ret_desc) {
	ade_table_entry ate;

	ate.Name = name;
	ate.Instanced = true;
	ate.Type = 'v';
	ate.Value.Function = func;
	ate.Arguments = args;
	ate.Description = desc;
	ate.ReturnType = ret_type;
	ate.ReturnDescription = ret_desc;

	LibIdx = ade_manager::getInstance()->getEntry(parent.GetIdx()).AddSubentry(ate);
}

ade_indexer::ade_indexer(lua_CFunction func,
						 ade_lib_handle& parent,
						 const char* args,
						 const char* desc,
						 const char* ret_type,
						 const char* ret_desc) {
	//Add function for meta
	ade_table_entry ate;

	ate.Name = "__indexer";
	ate.Instanced = true;
	ate.Type = 'u';
	ate.Value.Function = func;
	ate.Arguments = args;
	ate.Description = desc;
	ate.ReturnType = ret_type;
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

//WMC - Gets type of object
const char *ade_get_type_string(lua_State *L, int argnum)
{
	int type = lua_type(L, argnum);

	if(type < 0 || type >= Lua_type_names_num)
		return "Unknown";

	return Lua_type_names[type];
}

}
