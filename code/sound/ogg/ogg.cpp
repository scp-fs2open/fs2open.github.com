#include "cfile/cfile.h"
#include "ogg.h"

int ogg_inited = 0;
ov_callbacks cfile_callbacks;

//Encapsulation funcs to please the almighty ov_callbacks struct
__inline size_t ogg_cfread(void *buf, size_t elsize, size_t elnem, void* cfile)
{
	return cfread(buf, elsize, elnem, (CFILE*)cfile);
}

__inline int ogg_cfseek(void* cfile, ogg_int64_t offset, int where)
{
	return cfseek((CFILE*)cfile, (int) offset, where);
}

__inline int ogg_cfclose(void* cfile)
{
	return cfclose((CFILE*) cfile);
}

__inline long ogg_cftell(void* cfile)
{
	return cftell((CFILE*) cfile);
}

int OGG_init()
{
	//Setup the cfile_callbacks stuct
	cfile_callbacks.read_func = ogg_cfread;
	cfile_callbacks.seek_func = ogg_cfseek;
	cfile_callbacks.close_func = ogg_cfclose;
	cfile_callbacks.tell_func = ogg_cftell;

	return 0;
}