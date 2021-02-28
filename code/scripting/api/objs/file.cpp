
#include "file.h"

#include "bytearray.h"

#include "cfile/cfilesystem.h"

extern int cfread_lua_number(double* buf, CFILE* cfile);
namespace scripting {
namespace api {

cfile_h::cfile_h(CFILE* cfp) : _cfp(cfp) {}
cfile_h::~cfile_h()
{
	if (_cfp && cf_is_valid(_cfp.get())) {
		Warning(LOCATION, "Lua cfile handle of file %s has been left open!\nYou should close the file with the object's close() method explicitly.\n", cf_get_filename(_cfp.get()));
	}
}
bool cfile_h::isValid() const { return _cfp != nullptr; }
CFILE* cfile_h::get() const { return _cfp.get(); }
void cfile_h::close()
{
	_cfp = nullptr; // Automatically closes the handle
}

//**********HANDLE: File
ADE_OBJ(l_File, cfile_h, "file", "File handle");

ADE_FUNC(isValid, l_File, NULL, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ADE_RETURN_NIL;

	if (cfp == nullptr || !cfp->isValid())
		return ADE_RETURN_FALSE;

	return ADE_RETURN_TRUE;
}

ADE_FUNC(close, l_File, NULL, "Instantly closes file and invalidates all file handles", NULL, NULL)
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ADE_RETURN_FALSE;

	if (cfp == nullptr || !cfp->isValid())
		return ADE_RETURN_FALSE;

	cfp->close();
	return ADE_RETURN_TRUE;
}

ADE_FUNC(flush, l_File, NULL, "Flushes file buffer to disk.", "boolean", "True for success, false on failure")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ADE_RETURN_FALSE;

	if (cfp == nullptr || !cfp->isValid())
		return ADE_RETURN_FALSE;

	//WMC - this looks reversed, yes, it's right. Look at cflush.
	int cf_result = cflush(cfp->get());
	return ade_set_args(L, "b", cf_result == 0);
}

ADE_FUNC(getName, l_File, nullptr, "Returns the name of the given file", "string", "Name of the file handle, or an empty string if it doesn't have one, or the handle is invalid")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ade_set_error(L, "s", "");

	if (cfp == nullptr || !cfp->isValid())
		return ade_set_error(L, "s", "");

	auto filename = cf_get_filename(cfp->get());
	if (filename != nullptr)
		return ade_set_args(L, "s", filename);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(getPath, l_File, nullptr, "Determines path of the given file", "string", "Path string of the file handle, or an empty string if it doesn't have one, or the handle is invalid")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ade_set_error(L, "s", "");

	if (cfp == nullptr || !cfp->isValid())
		return ade_set_error(L, "s", "");

	int id = cf_get_dir_type(cfp->get());
	if (id == CF_TYPE_ANY)
		return ade_set_args(L, "s", "<any path>");
	else if (Pathtypes[id].path != nullptr)
		return ade_set_args(L, "s", Pathtypes[id].path);
	else
		return ade_set_args(L, "s", "");
}

ADE_FUNC(read,
	l_File,
	"number|string, any...",
	"Reads part of or all of a file, depending on arguments passed. Based on basic Lua file:read function."
	"Returns nil when the end of the file is reached."
	"<br><ul><li>\"*n\" - Reads a number.</li>"
	"<li>\"*a\" - Reads the rest of the file and returns it as a string.</li>"
	"<li>\"*l\" - Reads a line. Skips the end of line markers.</li>"
	"<li>(number) - Reads given number of characters, then returns them as a string.</li></ul>",
	"number|string...",
	"Requested data, or nil if the function fails")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ADE_RETURN_NIL;

	if (cfp == nullptr || !cfp->isValid())
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
		const char* fmt = nullptr;
		//int num = 0;
		if (type == LUA_TSTRING)
		{
			fmt = lua_tostring(L, i);
			if (!stricmp(fmt, "*n"))
			{
				double d = 0.0f;
				if (cfread_lua_number(&d, cfp->get()) == EOF)
					return ADE_RETURN_NIL;

				lua_pushnumber(L, d);
				num_returned++;
			}
			else if (!stricmp(fmt, "*a"))
			{
				int tell_res = cftell(cfp->get());
				if (tell_res < 0)
				{
					Error(LOCATION, "Critical error reading Lua file; could not cftell.");
				}
				int read_len = cfilelength(cfp->get()) - tell_res;

				char *buf = (char *)vm_malloc(read_len + 1);
				int final_len = cfread(buf, 1, read_len, cfp->get());
				buf[final_len] = '\0';

				lua_pushstring(L, buf);
				vm_free(buf);
				num_returned++;
			}
			else if (!stricmp(fmt, "*l"))
			{
				char buf[10240];
				size_t idx;
				if (cfgets(buf, (int)(sizeof(buf) / sizeof(char)), cfp->get()) == nullptr)
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
				if (cfeof(cfp->get()))
					lua_pushstring(L, "");
				else
					lua_pushnil(L);
			}

			char *buf = (char*)vm_malloc(num + 1);
			int total_read = cfread(buf, 1, num, cfp->get());
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
	const char* w = nullptr;
	int o = 0;

	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o|si", l_File.GetPtr(&cfp), &w, &o))
		return ADE_RETURN_NIL;

	if (cfp == nullptr || !cfp->isValid())
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

		if (cfseek(cfp->get(), o, seek_type))
			return ADE_RETURN_FALSE;
	}

	int res = cftell(cfp->get());
	if (res >= 0)
		return ade_set_args(L, "i", res);
	else
		return ADE_RETURN_FALSE;
}

ADE_FUNC(write, l_File, "string|number, any...",
	"Writes a series of Lua strings or numbers to the current file.", "number", "Number of items successfully written.")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ade_set_error(L, "i", 0);

	if (cfp == nullptr || !cfp->isValid())
		return ade_set_error(L, "i", 0);

	int l_pos = 2;
	int type = LUA_TNONE;

	int num_successful = 0;
	while ((type = lua_type(L, l_pos)) != LUA_TNONE)
	{
		if (type == LUA_TSTRING)
		{
			auto s = lua_tostring(L, l_pos);
			if (cfwrite(s, (int)sizeof(char), (int)strlen(s), cfp->get()))
				num_successful++;
		}
		else if (type == LUA_TNUMBER)
		{
			double d = lua_tonumber(L, l_pos);
			char buf[32] = { 0 };
			sprintf(buf, LUA_NUMBER_FMT, d);
			if (cfwrite(buf, (int)sizeof(char), (int)strlen(buf), cfp->get()))
				num_successful++;
		}

		l_pos++;
	}

	return ade_set_args(L, "i", num_successful);
}

ADE_FUNC(writeBytes,
	l_File,
	"bytearray bytes",
	"Writes the specified data to the file",
	"number",
	"Number of bytes successfully written.")
{
	cfile_h* cfp = nullptr;
	bytearray_h* array = nullptr;
	if (!ade_get_args(L, "oo", l_File.GetPtr(&cfp), l_Bytearray.GetPtr(&array)))
		return ade_set_error(L, "i", 0);

	if (cfp == nullptr || !cfp->isValid())
		return ade_set_error(L, "i", 0);

	auto written = cfwrite(array->data().data(), 1, static_cast<int>(array->data().size()), cfp->get());

	return ade_set_args(L, "i", written);
}

ADE_FUNC(readBytes,
	l_File,
	nullptr,
	"Reads the entire contents of the file as a byte array.<br><b>Warning:</b> This may change the position inside the "
	"file.",
	"bytearray",
	"The bytes read from the file or empty array on error")
{
	cfile_h* cfp = nullptr;
	if (!ade_get_args(L, "o", l_File.GetPtr(&cfp)))
		return ade_set_error(L, "o", l_Bytearray.Set(bytearray_h()));

	if (cfp == nullptr || !cfp->isValid())
		return ade_set_error(L, "o", l_Bytearray.Set(bytearray_h()));

	cfseek(cfp->get(), 0, CF_SEEK_SET);
	SCP_vector<uint8_t> data;
	data.resize(cfilelength(cfp->get()));

	cfread(data.data(), 1, cfilelength(cfp->get()), cfp->get());

	return ade_set_error(L, "o", l_Bytearray.Set(bytearray_h(std::move(data))));
}

} // namespace api
} // namespace scripting
