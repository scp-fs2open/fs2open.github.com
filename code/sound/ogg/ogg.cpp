#include "cfile/cfile.h"
#include "ogg.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Mmsystem.h>
#endif

int ogg_inited = 0;
ov_callbacks cfile_callbacks;
ov_callbacks mmio_callbacks;

//Encapsulation funcs to please the almighty ov_callbacks struct
size_t ogg_cfread(void *buf, size_t elsize, size_t elnem, void* cfile)
{
	return cfread(buf, elsize, elnem, (CFILE*)cfile);
}

int ogg_cfseek(void* cfile, ogg_int64_t offset, int where)
{
	return cfseek((CFILE*)cfile, (int) offset, where);
}

int ogg_cfclose(void* cfile)
{
	return cfclose((CFILE*) cfile);
}

long ogg_cftell(void* cfile)
{
	return cftell((CFILE*) cfile);
}

#ifdef WIN32
size_t ogg_mmio_read(void *buf, size_t elsize, size_t elnem, void* mmfp)
{
	return mmioRead((HMMIO) mmfp, (HPSTR) buf, elsize * elnem);
}

int ogg_mmio_seek(void* mmfp, ogg_int64_t offset, int where)
{
	//return (int) mmioSeek((HMMIO) mmfp, (LONG) offset, where);
	return -1;
}

int ogg_mmio_close(void* mmfp)
{
	return mmioClose((HMMIO) mmfp, 0);
}

long ogg_mmio_tell(void* mmfp)
{
	//return mmioSeek((HMMIO) mmfp, 0, SEEK_CUR);
	return -1;
}
#endif
int OGG_init()
{
	//Setup the cfile_callbacks stucts
	cfile_callbacks.read_func = ogg_cfread;
	cfile_callbacks.seek_func = ogg_cfseek;
	cfile_callbacks.close_func = ogg_cfclose;
	cfile_callbacks.tell_func = ogg_cftell;

#ifdef WIN32
	mmio_callbacks.read_func = ogg_mmio_read;
	mmio_callbacks.seek_func = ogg_mmio_seek;
	mmio_callbacks.close_func= ogg_mmio_close;
	mmio_callbacks.tell_func = ogg_mmio_tell;
#endif

	return 0;
}
