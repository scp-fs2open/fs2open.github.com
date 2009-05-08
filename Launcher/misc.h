#ifndef __MISC_H__
#define __MISC_H__

enum
{
	// First 6 flags must remain set they are
//	FLAG_D3D9		= 1 << 0,
	FLAG_OGL		= 1 << 0,
	FLAG_D3D8		= 1 << 1,
	FLAG_D3D5		= 1 << 2,
	FLAG_3DFX		= 1 << 3,
	FLAG_DD5		= 1 << 4,
	FLAG_SFT		= 1 << 5,
	FLAG_FS1		= 1 << 6,
	FLAG_FS2		= 1 << 7,
	FLAG_MULTI		= 1 << 8,
	FLAG_MOD		= 1 << 9,
	FLAG_FS2OPEN	= 1 << 10,
	FLAG_SCP		= /*FLAG_D3D9 |*/ FLAG_D3D8 | FLAG_OGL | FLAG_FS2 | FLAG_MOD | FLAG_FS2OPEN,
	FLAG_NEW		= 1 << 31,
};
	
enum
{
	EXE_TYPE_FS2,
	EXE_TYPE_FS2DEMO,
	EXE_TYPE_FS1,
	MAX_EXE_TYPES,
	EXE_TYPE_CUSTOM,
	EXE_TYPE_NONE = MAX_EXE_TYPES,
};

const char *INI_MAIN        = "launcher6.ini";
const char *INI_MOD_DEFAULT = "mod.ini";
const char *INI_MOD_CUSTOM  = "settings.ini";
const char *CFG_CMDLINE     = "cmdline_fso.cfg";


bool file_exists(const char *path);
int get_file_size(const char *path);
const char *get_filename_from_path(const char *path);
void remove_file_from_path(const char *path);

FILE *ini_open_for_write(const char *filepath, bool append, const char *comment);
void ini_write_type(FILE *fp, const char *type);
void ini_write_comment(FILE *fp, const char *comment);
void ini_write_data(FILE *fp, const char *type, const char *data);
void ini_write_data(FILE *fp, const char *type, bool data);
void ini_close(FILE *fp);

#endif
