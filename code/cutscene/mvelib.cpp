

#include "cfile/cfile.h"
#include "cutscene/mvelib.h"
#include "globalincs/pstypes.h"


static const char MVE_HEADER[]  = "Interplay MVE File\x1A";
static const short MVE_HDRCONST1 = 0x001A;
static const short MVE_HDRCONST2 = 0x0100;
static const short MVE_HDRCONST3 = 0x1133;


// -----------------------------------------------------------
// public MVEFILE functions
// -----------------------------------------------------------

// utility functions for mvefile and mveplayer
short mve_get_short(ubyte *data)
{
	short value;
	value = data[0] | (data[1] << 8);
	return value;
}

ushort mve_get_ushort(ubyte *data)
{
	ushort value;
	value = data[0] | (data[1] << 8);
	return value;
}

int mve_get_int(ubyte *data)
{
	int value;
	value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return value;
}

// open an MVE file
MVEFILE *mvefile_open(char *filename)
{
	int cf_opened = 0;
	int mve_valid = 1;
	char lower_name[MAX_FILENAME_LEN];
	char buffer[20];
	MVEFILE *file;

	// create the file
	file = (MVEFILE *)vm_malloc(sizeof(MVEFILE));

	// set defaults
	file->stream = NULL;
	file->cur_chunk = NULL;
	file->buf_size = 0;
	file->cur_fill = 0;
	file->next_segment = 0;

	// lower case filename for checking
	strncpy(lower_name, filename, strlen(filename)+1);
	strlwr(lower_name);
	
	char *p = strchr( lower_name, '.' );
	if ( p ) *p = 0;

	strcat_s( lower_name, ".mve" );

	// NOTE: CF_TYPE *must* be ANY to get movies off of the CDs
	// assume lower case filename for *nix
	file->stream = cfopen(lower_name, "rb", CFILE_NORMAL, CF_TYPE_ANY);
	if ( file->stream ) {
		cf_opened = 1;
	}

	if (!cf_opened) {
		mvefile_close(file);
		return NULL;
	}

	// initialize the buffer
	file->cur_chunk = (ubyte *)vm_malloc(100 + 1024);
	file->buf_size = 100 + 1024;

	// verify the file's header
	cfread_string(buffer, 20, file->stream);
	
	if (strcmp(buffer, MVE_HEADER))
		mve_valid = 0;

	if (cfread_short(file->stream) != MVE_HDRCONST1)
		mve_valid = 0;

	if (cfread_short(file->stream) != MVE_HDRCONST2)
		mve_valid = 0;

	if (cfread_short(file->stream) != MVE_HDRCONST3)
		mve_valid = 0;

	if (!mve_valid) {
		mvefile_close(file);
		return NULL;
	}

	// now, prefetch the next chunk
	mvefile_fetch_next_chunk(file);

	return file;
}

// close a MVE file
void mvefile_close(MVEFILE *file)
{
	// free the stream
	if (file->stream)
		cfclose(file->stream);

	file->stream = NULL;

	// free the buffer
	if (file->cur_chunk)
		vm_free(file->cur_chunk);

	file->cur_chunk = NULL;

	// not strictly necessary
	file->buf_size = 0;
	file->cur_fill = 0;
	file->next_segment = 0;

	// free the struct
	vm_free(file);
}

// get the size of the next segment
int mvefile_get_next_segment_size(MVEFILE *file)
{
	// if nothing is cached, fail
	if (file->cur_chunk == NULL || file->next_segment >= file->cur_fill)
		return -1;

	// if we don't have enough data to get a segment, fail
	if (file->cur_fill - file->next_segment < 4)
		return -1;

	// otherwise, get the data length
	return mve_get_short(file->cur_chunk + file->next_segment);
}

// get type of next segment in chunk (0xff if no more segments in chunk)
ubyte mvefile_get_next_segment_major(MVEFILE *file)
{
	// if nothing is cached, fail
	if (file->cur_chunk == NULL || file->next_segment >= file->cur_fill)
		return 0xff;

	// if we don't have enough data to get a segment, fail
	if (file->cur_fill - file->next_segment < 4)
		return 0xff;

	// otherwise, get the data length
	return file->cur_chunk[file->next_segment + 2];
}

// get subtype (version) of next segment in chunk (0xff if no more segments in chunk)
ubyte mvefile_get_next_segment_minor(MVEFILE *file)
{
	// if nothing is cached, fail
	if (file->cur_chunk == NULL || file->next_segment >= file->cur_fill)
		return 0xff;

	// if we don't have enough data to get a segment, fail
	if (file->cur_fill - file->next_segment < 4)
		return 0xff;

	// otherwise, get the data length
	return file->cur_chunk[file->next_segment + 3];
}

// see next segment (return NULL if no next segment)
ubyte *mvefile_get_next_segment(MVEFILE *file)
{
	// if nothing is cached, fail
	if (file->cur_chunk == NULL || file->next_segment >= file->cur_fill)
		return NULL;

	// if we don't have enough data to get a segment, fail
	if (file->cur_fill - file->next_segment < 4)
		return NULL;

	// otherwise, get the data length
	return file->cur_chunk + file->next_segment + 4;
}

// advance to next segment
void mvefile_advance_segment(MVEFILE *file)
{
	// if nothing is cached, fail
	if (file->cur_chunk == NULL || file->next_segment >= file->cur_fill)
		return;

	// if we don't have enough data to get a segment, fail
	if (file->cur_fill - file->next_segment < 4)
		return;

	// else, advance to next segment
	file->next_segment += (4 + mve_get_ushort(file->cur_chunk + file->next_segment));
}

// fetch the next chunk (return 0 if at end of stream)
int mvefile_fetch_next_chunk(MVEFILE *file)
{
	ubyte buffer[4];
	ubyte *new_buffer;
	ushort length;

	// fail if not open
	if (file->stream == NULL)
		return 0;

	// fail if we can't read the next segment descriptor
	if (cfread(buffer, 1, 4, file->stream) < 4)
		return 0;

	// pull out the next length
	length = mve_get_short(buffer);

	// setup a new buffer if needed --
	// only allocate new buffer is old one is too small
	if (length > file->buf_size) {
		// allocate new buffer
		new_buffer = (ubyte *)vm_malloc(100 + length);

		// copy old data
		if (file->cur_chunk && file->cur_fill)
			memcpy(new_buffer, file->cur_chunk, file->cur_fill);

		// free old buffer
		if (file->cur_chunk) {
			vm_free(file->cur_chunk);
			file->cur_chunk = NULL;
		}

		// install new buffer
		file->cur_chunk = new_buffer;
		file->buf_size = 100 + length;
	}

	// read the chunk
	if (length > 0) {
		if (cfread(file->cur_chunk, 1, length, file->stream) < length)
			return 0;
	}

	file->cur_fill = length;
	file->next_segment = 0;

	return 1;
}

// -----------------------------------------------------------
// public MVESTREAM functions
// -----------------------------------------------------------

// open an MVE stream
MVESTREAM *mve_open(char *filename)
{
	MVESTREAM *stream;

	// allocate
	stream = (MVESTREAM *)vm_malloc(sizeof(MVESTREAM));

	// defaults
	stream->movie = NULL;

	// open
	stream->movie = mvefile_open(filename);

	if (stream->movie == NULL) {
		mve_close(stream);
		return NULL;
	}

	return stream;
}

// close an MVE stream
void mve_close(MVESTREAM *stream)
{
	// close MVEFILE
	if (stream->movie)
		mvefile_close(stream->movie);

	stream->movie = NULL;

	vm_free(stream);
}

// play next chunk
int mve_play_next_chunk(MVESTREAM *stream)
{
	ubyte major, minor;
	ubyte *data;
	int len;

	// loop over segments
	major = mvefile_get_next_segment_major(stream->movie);
	while (major != 0xff) {
		// check whether to handle the segment
		if (major < 32) {
			minor = mvefile_get_next_segment_minor(stream->movie);
			len = mvefile_get_next_segment_size(stream->movie);
			data = mvefile_get_next_segment(stream->movie);

			switch (major) {
				case 0x00:
					mve_end_movie();
					break;
				case 0x01:
					mve_end_chunk();
					break;
				case 0x02:
					mve_timer_create(data);
					break;
				case 0x03:
					mve_audio_createbuf(minor, data);
					break;
				case 0x04:
					mve_audio_play();
					break;
				case 0x05:
					if (!mve_video_createbuf(minor, data))
						return 0;
					break;
				case 0x07:
					mve_video_display();
					break;
				case 0x08:
					mve_audio_data(major, data);
					break;
				case 0x09:
					mve_audio_data(major, data);
					break;
				case 0x0a:
					if (!mve_video_init(data))
						return 0;
					break;
				case 0x0c:
					mve_video_palette(data);
					break;
				case 0x0f:
					mve_video_codemap(data, len);
					break;
				case 0x11:
					mve_video_data(data, len);
					break;
				default:
					break;
			}
		}

		// advance to next segment
		mvefile_advance_segment(stream->movie);
		major = mvefile_get_next_segment_major(stream->movie);
	}

	if (!mvefile_fetch_next_chunk(stream->movie))
		return 0;

	// return status
	return 1;
}
