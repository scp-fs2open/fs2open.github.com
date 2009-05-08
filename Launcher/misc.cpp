
#include "stdafx.h"
#include <sys/stat.h>

#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"


#include "misc.h"


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
