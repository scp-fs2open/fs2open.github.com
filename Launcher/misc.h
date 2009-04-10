#ifndef __MISC_H__
#define __MISC_H__

bool file_exists(char *path);
int get_file_size(char *path);
char *get_filename_from_path(char *path);
void remove_file_from_path(char *path);

FILE *ini_open_for_write(char *filepath, bool append, char *comment);
void ini_write_type(FILE *fp, char *type);
void ini_write_comment(FILE *fp, char *comment);
void ini_write_data(FILE *fp, char *type, char *data);
void ini_close(FILE *fp);


#endif
