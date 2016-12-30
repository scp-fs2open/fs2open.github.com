
#include "file.h"
#include "cfile/cfilesystem.h"

extern int cfread_lua_number(double *buf, CFILE *cfile);
namespace scripting {
namespace api {

//**********HANDLE: File
ADE_OBJ(l_File, CFILE*, "file", "File handle");

ADE_FUNC(isValid, l_File, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	CFILE *cfp = NULL;
	if (!ade_get_args(L, "o", l_File.Get(&cfp)))
		return ADE_RETURN_NIL;

	if (!cf_is_valid(cfp))
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(close, l_File, NULL, "Instantly closes file and invalidates all file handles", NULL, NULL)
{
	CFILE *cfp;
	if (!ade_get_args(L, "o", l_File.Get(&cfp)))
		return ADE_RETURN_FALSE;

	if (!cf_is_valid(cfp))
		return ADE_RETURN_FALSE;

	int rval = cfclose(cfp);
	if (rval != 0)
	{
		LuaError(L, "Attempt to close file resulted in error %d", rval);
		return ADE_RETURN_FALSE;
	}
	return ADE_RETURN_TRUE;
}

ADE_FUNC(flush, l_File, NULL, "Flushes file buffer to disk.", "boolean", "True for success, false on failure")
{
	CFILE *cfp = NULL;
	if (!ade_get_args(L, "o", l_File.Get(&cfp)))
		return ADE_RETURN_FALSE;

	if (!cf_is_valid(cfp))
		return ADE_RETURN_FALSE;

	//WMC - this looks reversed, yes, it's right. Look at cflush.
	int cf_result = cflush(cfp);
	return ade_set_args(L, "b", cf_result ? false : true);
}

ADE_FUNC(getPath, l_File, NULL, "Determines path of the given file", "string", "Path string of the file handle, or an empty string if it doesn't have one, or the handle is invalid")
{
	CFILE *cfp = NULL;
	if (!ade_get_args(L, "o", l_File.Get(&cfp)))
		return ade_set_error(L, "s", "");

	if (!cf_is_valid(cfp))
		return ade_set_error(L, "s", "");

	int id = cf_get_dir_type(cfp);
	if (Pathtypes[id].path != NULL)
		return ade_set_args(L, "s", Pathtypes[id].path);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(read, l_File, "number or string, ...",
	"Reads part of or all of a file, depending on arguments passed. Based on basic Lua file:read function."
	"Returns nil when the end of the file is reached."
	"<br><ul><li>\"*n\" - Reads a number.</li>"
	"<li>\"*a\" - Reads the rest of the file and returns it as a string.</li>"
	"<li>\"*l\" - Reads a line. Skips the end of line markers.</li>"
	"<li>(number) - Reads given number of characters, then returns them as a string.</li></ul>",
	"number or string, ...",
	"Requested data, or nil if the function fails")
{
	CFILE *cfp = NULL;
	if (!ade_get_args(L, "o", l_File.Get(&cfp)))
		return ADE_RETURN_NIL;

	if (!cf_is_valid(cfp))
		return ADE_RETURN_NIL;

	int i;
	int num_returned = 0;
	int type = LUA_TNONE;

	//WMC - Since we push stuff onto the stack, we must get
	//the original arguments NOW.
	int lastarg = lua_gettop(L);
	for (i = 2; i <= lastarg; i++)
	{
		type = lua_type(L, i);
		char *fmt = NULL;
		//int num = 0;
		if (type == LUA_TSTRING)
		{
			fmt = (char*)lua_tostring(L, i);
			if (!stricmp(fmt, "*n"))
			{
				double d = 0.0f;
				if (cfread_lua_number(&d, cfp) == EOF)
					return ADE_RETURN_NIL;

				lua_pushnumber(L, d);
				num_returned++;
			}
			else if (!stricmp(fmt, "*a"))
			{
				int tell_res = cftell(cfp);
				if (tell_res < 0)
				{
					Error(LOCATION, "Critical error reading Lua file; could not cftell.");
				}
				int read_len = cfilelength(cfp) - tell_res;

				char *buf = (char *)vm_malloc(read_len + 1);
				int final_len = cfread(buf, 1, read_len, cfp);
				buf[final_len] = '\0';

				lua_pushstring(L, buf);
				vm_free(buf);
				num_returned++;
			}
			else if (!stricmp(fmt, "*l"))
			{
				char buf[10240];
				size_t idx;
				if (cfgets(buf, (int)(sizeof(buf) / sizeof(char)), cfp) == NULL)
				{
					lua_pushnil(L);
				}
				else
				{
					// Strip all newlines so this works like the Lua original
					// http://www.lua.org/source/5.1/liolib.c.html#g_read
					// Note: we also strip carriage return in WMC's implementation
					for (idx = 0; idx < strlen(buf); idx++)
					{
						if (buf[idx] == '\n' || buf[idx] == '\r')
							buf[idx] = '\0';
					}

					lua_pushstring(L, buf);
				}
				num_returned++;
			}
		}
		if (type == LUA_TNUMBER || (type == LUA_TSTRING && strpbrk(fmt, "1234567890")))
		{
			int num = 0;
			if (type == LUA_TSTRING)
				num = atoi(fmt);
			else
				num = fl2i(lua_tonumber(L, i));

			if (num < 1)
			{
				if (cfeof(cfp))
					lua_pushstring(L, "");
				else
					lua_pushnil(L);
			}

			char *buf = (char*)vm_malloc(num + 1);
			int total_read = cfread(buf, 1, num, cfp);
			if (total_read)
			{
				buf[total_read] = '\0';
				lua_pushstring(L, buf);
			}
			else
			{
				lua_pushnil(L);
			}
			vm_free(buf);
			num_returned++;
		}
		if (type != LUA_TNUMBER && type != LUA_TSTRING)
			LuaError(L, "Invalid argument passed to file:read");
	}

	return num_returned;
}

ADE_FUNC(seek, l_File, "[string Whence=\"cur\", number Offset=0]",
	"Changes position of file, or gets location."
	"Whence can be:"
	"<li>\"set\" - File start.</li>"
	"<li>\"cur\" - Current position in file.</li>"
	"<li>\"end\" - File end.</li></ul>",
	"number",
	"new offset, or false or nil on failure")
{
	char *w = NULL;
	int o = 0;

	CFILE *cfp = NULL;
	if (!ade_get_args(L, "o|si", l_File.Get(&cfp), &w, &o))
		return ADE_RETURN_NIL;

	if (!cf_is_valid(cfp))
		return ADE_RETURN_NIL;

	if (!(w == NULL || (!stricmp(w, "cur") && o != 0)))
	{
		int seek_type = CF_SEEK_CUR;
		if (!stricmp(w, "set"))
			seek_type = CF_SEEK_SET;
		else if (!stricmp(w, "cur"))
			seek_type = CF_SEEK_CUR;
		else if (!stricmp(w, "end"))
			seek_type = CF_SEEK_END;
		else
			LuaError(L, "Invalid where argument passed to seek() - '%s'", w);

		if (cfseek(cfp, o, seek_type))
			return ADE_RETURN_FALSE;
	}

	int res = cftell(cfp);
	if (res >= 0)
		return ade_set_args(L, "i", res);
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(write, l_File, "string or number, ...",
	"Writes a series of Lua strings or numbers to the current file.", "number", "Number of items successfully written.")
{
	CFILE *cfp = NULL;
	if (!ade_get_args(L, "o", l_File.Get(&cfp)))
		return ade_set_error(L, "i", 0);

	if (!cf_is_valid(cfp))
		return ade_set_error(L, "i", 0);

	int l_pos = 2;
	int type = LUA_TNONE;

	int num_successful = 0;
	while ((type = lua_type(L, l_pos)) != LUA_TNONE)
	{
		if (type == LUA_TSTRING)
		{
			char *s = (char*)lua_tostring(L, l_pos);
			if (cfwrite(s, (int)sizeof(char), (int)strlen(s), cfp))
				num_successful++;
		}
		else if (type == LUA_TNUMBER)
		{
			double d = lua_tonumber(L, l_pos);
			char buf[32] = { 0 };
			sprintf(buf, LUA_NUMBER_FMT, d);
			if (cfwrite(buf, (int)sizeof(char), (int)strlen(buf), cfp))
				num_successful++;
		}

		l_pos++;
	}

	return ade_set_args(L, "i", num_successful);
}

}
}
