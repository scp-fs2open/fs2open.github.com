#include <vorbis/vorbisfile.h>

//Setup the OGG stuff to use cfile
extern ov_callbacks cfile_callbacks;

//Init the ogg system
int OGG_init();

//Similar to the stuff in mmreg.h
#define  OGG_FORMAT_VORBIS		0x3000	/* OGG Files */