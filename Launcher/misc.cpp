
#include "stdafx.h"
#include <sys/stat.h>
#include <direct.h>

#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"


#include "misc.h"
#include "launcher_settings.h"


/**
 * Checks if a file exists using fopen
 *
 * @param char *path - Full path and name of exe
 * @return bool      - true if file exists, otherwise false (also if file is not avaliable, ie currently open)
 */
bool file_exists(const char *path)
{
	if(strlen(path) == 0) return false;

	if(path != NULL)
	{
		FILE *fp = fopen(path, "r");

		if(fp != NULL)
		{
			fclose(fp);
			return true;
		}
	}

	return false;
}

bool check_cfg_file(char *dest_buffer, bool create_dir)
{
	strcpy(dest_buffer, LauncherSettings::get_exe_pathonly());
	strcat(dest_buffer, "\\data");

	if (create_dir) {
		_mkdir(dest_buffer);
	}

	if (LauncherSettings::get_exe_type() != EXE_TYPE_CUSTOM) {
		strcat(dest_buffer, "\\cmdline.cfg");
	} else {
		strcat(dest_buffer, "\\cmdline_fso.cfg");
	}
			
	return file_exists(dest_buffer);
}

/**
 * Gets a files size using stat
 *
 * @param char *path - Full path of file to check
 * @return 			 - Size of file or -1 on error
 */
int get_file_size(const char *path)
{
	// This helps us find the size of the exe
	struct _stat stat_buffer;
	if(_stat( path, &stat_buffer ) == -1)
	{
		return -1;
	}

	return stat_buffer.st_size; 
}

/**
 * Find the file name given a path
 *
 * @param char *path - Path of file
 * @return 			 - Pointer to position in GIVEN PATH (CHAR *) where the actual filename starts
 *
 */
const char *get_filename_from_path(const char *path)
{
	if(path == NULL)
	{
		return NULL;
	}

	// Find the exe name only
	char *filename = strrchr(path, '\\');

	// No last slash is found, assume full path is filename
	if(filename == NULL)
	{
		return path;
	}
	else
	{
		return filename+1;
	}
}

/**
 * Find the file name given a path
 *
 * @param char *path - Path of file
 * @return 			 - Pointer to position in GIVEN PATH (CHAR *) where the actual filename starts
 *
 */
void remove_file_from_path(const char *path)
{
	if(path == NULL)
	{
		return;
	}

	// Find the exe name only
	char *filename = strrchr(path, '\\');

	// No last slash is found, assume full path is filename
	if(filename != NULL)
	{
		*filename = '\0';
	}
}

bool is_whitespace(char ch)
{
	return ch == ' ' || ch == '\n' || ch == '\t';
}

/**
 * Remove whitespace surrounding a string, in-place.
 */
void trim(char *str)
{
	// shortcut if the first char isn't whitespace
	if (!is_whitespace(*str))
	{
		char *ch = str + strlen(str) - 1;
		while (is_whitespace(*ch))
			ch--;

		*(ch+1) = 0;
	}
	// otherwise copy the non-whitespace part
	else
	{
		char *temp = strdup(str);

		char *start_ch = temp;
		while (is_whitespace(*start_ch))
			start_ch++;

		char *end_ch = temp + strlen(temp) - 1;
		while (is_whitespace(*end_ch))
			end_ch--;

		int len = end_ch - start_ch + 1;
		strncpy(str, start_ch, len);
		str[len] = 0;

		free(temp);
	}
}

/**
 * Remove whitespace surrounding a string, in-place.
 */
void trim(CString str)
{
	str.TrimLeft();
	str.TrimRight();
}

FILE *ini_open_for_write(const char *filepath, bool append, const char *comment)
{
	char *open_type = append ? "a" : "w";

	FILE *fp = fopen(filepath, open_type);

	if(fp && comment)
	{
		fprintf(fp, "\n"
		"#\n"
		"# %s\n"
		"#\n", comment);
	}

	return fp;
}

void ini_write_type(FILE *fp, const char *type)
{
	if(type)
	{
		fprintf(fp, "%s\n", type);
	}
}

void ini_write_comment(FILE *fp, const char *comment)
{
	if(comment)
	{
		fprintf(fp, "\n"
		"#\n"
		"# %s\n"
		"#\n", comment);
	}

}

void ini_write_data(FILE *fp, const char *type, const char *data)
{
	if(type)
	{
		if(data == NULL)
		{
			data = "";
		}

		fprintf(fp, "%s = %s;\n", type, data);
	}
}

void ini_write_data(FILE *fp, const char *type, bool data)
{
	ini_write_data(fp, type, data ? "true" : "false");
}

void ini_close(FILE *fp)
{
	fclose(fp);
}
