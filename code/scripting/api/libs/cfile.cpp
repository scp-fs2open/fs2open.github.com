//
//

#include "cfile.h"
#include "cfile/cfile.h"
#include "cfile/cfilesystem.h"
#include "scripting/api/objs/file.h"
#include "scripting/lua/LuaTable.h"

namespace scripting {
namespace api {

//**********LIBRARY: CFILE
//WMC - It's on my to-do list! (Well, if I had one anyway)
//WMC - Did it. I had to invent a to-do list first, though.
//Ironically, I never actually put this on it.
ADE_LIB(l_CFile, "CFile", "cf", "CFile FS2 filesystem access");

ADE_FUNC(deleteFile, l_CFile, "string Filename, string Path", "Deletes given file. Path must be specified. Use a slash for the root directory.", "boolean", "True if deleted, false")
{
	const char* n_filename = nullptr;
	const char *n_path = "";
	if(!ade_get_args(L, "ss", &n_filename, &n_path))
		return ade_set_error(L, "b", false);

	int path = CF_TYPE_INVALID;
	if(n_path != NULL && strlen(n_path))
		path = cfile_get_path_type(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", cf_delete(n_filename, path) != 0);
}

ADE_FUNC(fileExists, l_CFile, "string Filename, [string Path = \"\", boolean CheckVPs = false]", "Checks if a file exists. Use a blank string for path for any directory, or a slash for the root directory.", "boolean", "True if file exists, false or nil otherwise")
{
	const char* n_filename = nullptr;
	const char *n_path = "";
	bool check_vps = false;
	if(!ade_get_args(L, "s|sb", &n_filename, &n_path, &check_vps))
		return ADE_RETURN_NIL;

	int path = CF_TYPE_ANY;
	if(n_path != NULL && strlen(n_path))
		path = cfile_get_path_type(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	if(!check_vps)
		return ade_set_args(L, "b", cf_exists(n_filename, path) != 0);
	else
		return ade_set_args(L, "b", cf_exists_full(n_filename, path ) != 0);
}

ADE_FUNC(listFiles, l_CFile, "string directory, string filter",
         "Lists all the files in the specified directory and optionally applies a filter. The filter must have the "
         "format \"*<rest>\" (the wildcard has to appear at the start).",
         "string[]", "A table with all files in the directory or nil on error")
{
	using namespace luacpp;

	const char* dir;
	const char* filter;
	if (!ade_get_args(L, "ss", &dir, &filter)) {
		return ADE_RETURN_NIL;
	}

	SCP_string filter_str = filter;
	if (filter_str.empty()) {
		LuaError(L, "The filter \"%s\" is not valid! It may not be empty.", filter);
		return ADE_RETURN_NIL;
	}

	if (filter_str[0] != '*') {
		LuaError(L, "The filter \"%s\" is not valid! The first character must be a '*'.", filter);
		return ADE_RETURN_NIL;
	}

	SCP_string ext; // Extension with '.' if it exists
	auto dot_pos = filter_str.find('.');
	if (dot_pos != SCP_string::npos) {
		ext = filter_str.substr(dot_pos);
	}

	auto path_type = cfile_get_path_type(dir);
	if (path_type == CF_TYPE_INVALID) {
		LuaError(L, "The directory \"%s\" is not valid!", dir);
		return ADE_RETURN_NIL;
	}

	SCP_vector<SCP_string> files;
	cf_get_file_list(files, path_type, filter, CF_SORT_NAME);

	LuaTable table = LuaTable::create(L);
	for (size_t i = 0; i < files.size(); ++i) {
		// Add the extension since cf_get_file_list removes it. We use the filter without the preceeding '*' here
		table.addValue(i + 1, files[i] + ext);
	}

	return ade_set_args(L, "t", &table);
}

ADE_FUNC(openFile, l_CFile, "string Filename, [string Mode=\"r\", string Path = \"\"]",
		 "Opens a file. 'Mode' uses standard C fopen arguments. Use a blank string for path for any directory, or a slash for the root directory."
			 "Be EXTREMELY CAREFUL when using this function, as you may PERMANENTLY delete any file by accident",
		 "file",
		 "File handle, or invalid file handle if the specified file couldn't be opened")
{
	const char* n_filename = nullptr;
	const char *n_mode = "r";
	const char *n_path = "";
	if(!ade_get_args(L, "s|ss", &n_filename, &n_mode, &n_path))
		return ade_set_error(L, "o", l_File.Set(cfile_h()));

	int type = CFILE_NORMAL;

	int path = CF_TYPE_ANY;
	if(n_path != NULL && strlen(n_path))
		path = cfile_get_path_type(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "o", l_File.Set(cfile_h()));

	CFILE *cfp = cfopen(n_filename, n_mode, type, path);

	if(!cf_is_valid(cfp))
		return ade_set_error(L, "o", l_File.Set(cfile_h()));

	return ade_set_args(L, "o", l_File.Set(cfile_h(cfp)));
}

ADE_FUNC(openTempFile, l_CFile, NULL, "Opens a temp file that is automatically deleted when closed", "file", "File handle, or invalid file handle if tempfile couldn't be created")
{
	return ade_set_args(L, "o", l_File.Set(cfile_h(ctmpfile())));
}

ADE_FUNC(renameFile, l_CFile, "string CurrentFilename, string NewFilename, string Path", "Renames given file. Path must be specified. Use a slash for the root directory.", "boolean", "True if file was renamed, otherwise false")
{
	const char* n_filename     = nullptr;
	const char* n_new_filename = nullptr;
	const char *n_path = "";
	if(!ade_get_args(L, "ss|s", &n_filename, &n_new_filename, &n_path))
		return ade_set_error(L, "b", false);

	int path = CF_TYPE_INVALID;
	if(n_path != NULL && strlen(n_path))
		path = cfile_get_path_type(n_path);

	if(path == CF_TYPE_INVALID)
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", cf_rename(n_filename, n_new_filename, path) != 0);
}

}
}

