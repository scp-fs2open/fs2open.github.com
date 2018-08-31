#define _CFILE_INTERNAL

#include "cfilemod.h"
#include "cfile.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"
#include "cfilesystem.h"

#ifdef SCP_UNIX
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#endif

namespace cfile {

namespace {

typedef struct cf_root_sort {
	SCP_string path;
	int cf_type;
} cf_root_sort;

// packfile sort function
bool cf_packfile_sort_func(const cf_root_sort& r1, const cf_root_sort& r2) {
	// if the 2 directory types are the same, do a string compare
	if (r1.cf_type == r2.cf_type) {
		return (stricmp(r1.path.c_str(), r2.path.c_str()) < 0);
	}

	// otherwise return them in order of CF_TYPE_* precedence
	return (r1.cf_type < r2.cf_type);
}

// Go through a root and look for pack files
void build_pack_list(SCP_vector<ModRoot>& mod_roots,
                     const SCP_string& search_path,
                     LocationType location_type,
                     bool user_location) {
	char filespec[MAX_PATH_LEN];

	// allocate a temporary array of temporary roots so we can easily sort them
	SCP_vector<cf_root_sort> temp_roots_sort;

	// now just setup all the root info
	for (int i = CF_TYPE_ROOT; i < CF_MAX_PATH_TYPES; i++) {
		strcpy_s(filespec, search_path.c_str());

		if (strlen(Pathtypes[i].path)) {
			strcat_s(filespec, Pathtypes[i].path);

			if (filespec[strlen(filespec) - 1] != DIR_SEPARATOR_CHAR)
				strcat_s(filespec, DIR_SEPARATOR_STR);
		}

#if defined _WIN32
			strcat_s( filespec, "*.vp" );

		intptr_t find_handle;
		_finddata_t find;

		find_handle = _findfirst( filespec, &find );

		 if (find_handle != -1) {
			do {
				// add the new item
				if (!(find.attrib & _A_SUBDIR)) {

					// get a temp pointer
					rptr_sort = &temp_roots_sort[root_index++];

					// fill in all the proper info
					strcpy_s(rptr_sort->path, root->path);

					if(strlen(Pathtypes[i].path)) {

						strcat_s(rptr_sort->path, Pathtypes[i].path );
						strcat_s(rptr_sort->path, DIR_SEPARATOR_STR);
					}

					strcat_s(rptr_sort->path, find.name );
					rptr_sort->roottype = CF_ROOTTYPE_PACK;
					rptr_sort->cf_type = i;
				}

			} while (!_findnext(find_handle, &find));

			_findclose( find_handle );
		}
#elif defined SCP_UNIX
		strcat_s(filespec, "*.[vV][pP]");
		glob_t globinfo;

		memset(&globinfo, 0, sizeof(globinfo));

		int status = glob(filespec, 0, NULL, &globinfo);

		if (status == 0) {
			for (uint j = 0; j < globinfo.gl_pathc; j++) {
				// Determine if this is a regular file
				struct stat statbuf;
				memset(&statbuf, 0, sizeof(statbuf));
				stat(globinfo.gl_pathv[j], &statbuf);

				if (S_ISREG(statbuf.st_mode)) {

					cf_root_sort sort;

					// fill in all the proper info
					sort.path = globinfo.gl_pathv[j];
					sort.cf_type = i;
					temp_roots_sort.push_back(sort);
				}
			}

			globfree(&globinfo);
		}
#endif
	}

	// sort the roots
	std::sort(temp_roots_sort.begin(), temp_roots_sort.end(), cf_packfile_sort_func);

	// now insert them all into the real root list properly
	for (const auto& pack_root : temp_roots_sort) {
		ModRoot root;
		root.type = ModRootType::Package;
		root.path = pack_root.path;
		root.user_location = user_location;
		root.location_type = location_type;

		mod_roots.push_back(root);
	}
}

char normalize_directory_separator(char in) {
	if (in == '/') {
		return DIR_SEPARATOR_CHAR;
	}

	return in;
}

void
add_mod_roots(SCP_vector<ModRoot>& mod_roots, const char* mod_cmdline, const SCP_string& rootDirectory, bool user_location) {
	if (mod_cmdline) {
		bool primary = true;
		for (const char* cur_pos = mod_cmdline; strlen(cur_pos) != 0; cur_pos += (strlen(cur_pos) + 1)) {
			SCP_stringstream ss;
			ss << rootDirectory;

			if (rootDirectory.back() != DIR_SEPARATOR_CHAR) {
				ss << DIR_SEPARATOR_CHAR;
			}

			ss << cur_pos << DIR_SEPARATOR_STR;

			SCP_string rootPath = ss.str();

			// normalize the path to the native path format
			std::transform(rootPath.begin(), rootPath.end(), rootPath.begin(), normalize_directory_separator);

			ModRoot root;
			root.type = ModRootType::Folder;
			root.path = rootPath;
			root.user_location = user_location;

			if (primary) {
				root.location_type = LocationType::PrimaryMod;
			} else {
				root.location_type = LocationType::SecondaryMod;
			}

			mod_roots.push_back(root);

			build_pack_list(mod_roots, rootPath, root.location_type, user_location);

			primary = false;
		}
	}
}

}

SCP_vector<ModRoot> getModRootsFromModCmdline(const SCP_string& content_root, const char* mod_cmdline) {
	SCP_vector<ModRoot> mod_roots;

	if (os_is_legacy_mode()) {
		// =========================================================================
#ifdef WIN32
		// Nothing to do here, Windows uses the current directory as the base
#else
		add_mod_roots(mod_roots, mod_cmdline, os_get_legacy_user_dir(), true);

		ModRoot root;
		root.type = ModRootType::Folder;
		root.path = os_get_legacy_user_dir();
		root.user_location = true;
		root.location_type = LocationType::GameRoot;

		// do we already have a slash? as in the case of a root directory install
		if (root.path.back() != DIR_SEPARATOR_CHAR) {
			root.path += DIR_SEPARATOR_STR;        // put trailing backslash on for easier path construction
		}

		mod_roots.push_back(root);

		// Next, check any VP files under the current directory.
		build_pack_list(mod_roots, root.path, root.location_type, root.user_location);
#endif
		// =========================================================================
	} else if (!Cmdline_portable_mode) {
		// =========================================================================
		// now look for mods under the users HOME directory to use before system ones
		add_mod_roots(mod_roots, mod_cmdline, os_get_config_path(), true);
		// =========================================================================

		// =========================================================================
		// set users HOME directory as default for loading and saving files
		ModRoot root;
		root.type = ModRootType::Folder;
		root.path = os_get_config_path();
		root.user_location = true;
		root.location_type = LocationType::GameRoot;

		// do we already have a slash? as in the case of a root directory install
		if (root.path.back() != DIR_SEPARATOR_CHAR) {
			root.path += DIR_SEPARATOR_STR;        // put trailing backslash on for easier path construction
		}

		mod_roots.push_back(root);

		// Next, check any VP files under the current directory.
		build_pack_list(mod_roots, root.path, root.location_type, root.user_location);
		// =========================================================================
	}

	add_mod_roots(mod_roots, mod_cmdline, content_root, false);

	ModRoot root;
	root.type = ModRootType::Folder;
	root.path = content_root;
	root.user_location = false;
	root.location_type = LocationType::GameRoot;

	// do we already have a slash? as in the case of a root directory install
	if (root.path.back() != DIR_SEPARATOR_CHAR) {
		root.path += DIR_SEPARATOR_STR;        // put trailing backslash on for easier path construction
	}

	mod_roots.push_back(root);

	//======================================================
	// Next, check any VP files under the current directory.
	build_pack_list(mod_roots, root.path, root.location_type, root.user_location);

	return mod_roots;
}

}


