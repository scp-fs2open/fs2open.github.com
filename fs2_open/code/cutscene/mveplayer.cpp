/*
 * $Logfile: /Freespace2/code/cutscene/mveplayer.cpp $
 * $Revision: 2.1 $
 * $Date: 2006-05-13 06:59:48 $
 * $Author: taylor $
 *
 * MVE movie playing routines
 *
 * $Log: not supported by cvs2svn $
 *
 * $NoKeywords: $
 */

#ifdef SCP_UNIX
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#endif

#include "graphics/gropengl.h"
#include "graphics/gropengltexture.h"

#include "globalincs/pstypes.h"
#include "cutscene/mvelib.h"
#include "cutscene/movie.h"
#include "graphics/2d.h"
#include "io/key.h"
#include "osapi/osapi.h"
#include "io/timer.h"
#include "sound/sound.h"
#include "sound/ds.h"
#include "bmpman/bmpman.h"
//#include "sound/audiostr.h"


extern int Cmdline_noscalevid;

static int mve_playing;

// timer variables
static int g_spdFactorNum = 0;
static int g_spdFactorDenom = 10;
static int micro_frame_delay = 0;
static int timer_started = 0;
#ifdef SCP_UNIX
static struct timeval timer_expire = { 0, 0 };
#else
static int timer_expire;
#endif

// audio variables
#define MVE_AUDIO_BUFFERS 64  // total buffers to interact with stream
static int mve_audio_buffer_curpos = 0;
static int mve_audio_buffer_head = 0;
static int mve_audio_buffer_tail = 0;
static int mve_audio_playing = 0;
static int mve_audio_canplay = 0;
static int mve_audio_compressed = 0;
static int audiobuf_created;

#ifdef USE_OPENAL
// struct for the audio stream information
typedef struct MVE_AUDIO_T {
	ALenum format;
	int sample_rate;
	int bytes_per_sec;
	int channels;
	int bitsize;
	ALuint audio_data[MVE_AUDIO_BUFFERS];
	ALuint source_id;
	ALuint audio_buffer[MVE_AUDIO_BUFFERS];
} mve_audio_t;

mve_audio_t *mas;  // mve_audio_stream

// audio decompressor
extern void mveaudio_uncompress(short *buffer, unsigned char *data, int length);
#endif // USE_OPENAL

// video variables
int g_width, g_height;
void *g_vBuffers = NULL;
void *g_vBackBuf1, *g_vBackBuf2;
ushort *pixelbuf = NULL;
static GLuint GLtex = 0;
static int g_screenWidth, g_screenHeight;
static int g_screenX, g_screenY;
static ubyte g_palette[768];
static ubyte *g_pCurMap=NULL;
static int g_nMapLength=0;
static int videobuf_created, video_inited;
static int hp2, wp2;
static uint mve_video_skiptimer = 0;
static int mve_scale_video = 0;

// video externs from API graphics functions
extern void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out);

// the decoder
extern void decodeFrame16(ubyte *pFrame, ubyte *pMap, int mapRemain, ubyte *pData, int dataRemain);


/*************************
 * general handlers
 *************************/
void mve_end_movie()
{
	mve_playing = 0;
}

/*************************
 * timer handlers
 *************************/

int mve_timer_create(ubyte *data)
{
	longlong temp;

	micro_frame_delay = mve_get_int(data) * (int)mve_get_short(data+4);

	if (g_spdFactorNum != 0) {
		temp = micro_frame_delay;
		temp *= g_spdFactorNum;
		temp /= g_spdFactorDenom;
		micro_frame_delay = (int)temp;
	}

	return 1;
}

static void mve_timer_start(void)
{
#ifdef SCP_UNIX
	int nsec = 0;

	gettimeofday(&timer_expire, NULL);

	timer_expire.tv_usec += micro_frame_delay;

	if (timer_expire.tv_usec > 1000000) {
		nsec = timer_expire.tv_usec / 1000000;
		timer_expire.tv_sec += nsec;
		timer_expire.tv_usec -= nsec * 1000000;
	}
#else
	timer_expire = timer_get_microseconds();
	timer_expire += micro_frame_delay;
#endif

	timer_started = 1;
}

static int mve_do_timer_wait(void)
{
	if (!timer_started)
		return 0;

#ifdef SCP_UNIX
	int nsec = 0;
	struct timespec ts, tsRem;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if (tv.tv_sec > timer_expire.tv_sec)
		goto end;

	if ( (tv.tv_sec == timer_expire.tv_sec) && (tv.tv_usec >= timer_expire.tv_usec) )
		goto end;

	ts.tv_sec = timer_expire.tv_sec - tv.tv_sec;
	ts.tv_nsec = 1000 * (timer_expire.tv_usec - tv.tv_usec);

	if (ts.tv_nsec < 0) {
		ts.tv_nsec += 1000000000UL;
		--ts.tv_sec;
	}

	if ( (nanosleep(&ts, &tsRem) == -1) && (errno == EINTR) ) {
		// so we got an error that was a signal interupt, try to sleep again with remainder of time
		if ( (nanosleep(&tsRem, NULL) == -1) && (errno == EINTR) ) {
			mprintf(("MVE: Timer error! Aborting movie playback!\n"));
			return 1;
		}
	}

end:
    timer_expire.tv_usec += micro_frame_delay;

    if (timer_expire.tv_usec > 1000000) {
        nsec = timer_expire.tv_usec / 1000000;
        timer_expire.tv_sec += nsec;
        timer_expire.tv_usec -= nsec * 1000000;
    }
#else
	int tv, ts, ts2;

	tv = timer_get_microseconds();

	if (tv > timer_expire)
		goto end;

	ts = timer_expire - tv;

	ts2 = ts/1000;

	Sleep(ts2);

end:
	timer_expire += micro_frame_delay;
#endif

	return 0;
}

static void mve_timer_stop()
{
#ifdef SCP_UNIX
	timer_expire.tv_sec = 0;
	timer_expire.tv_usec = 0;
#else
	timer_expire = 0;
#endif
	timer_started = 0;
}

/*************************
 * audio handlers
 *************************/

// setup the audio information from the data stream
void mve_audio_createbuf(ubyte minor, ubyte *data)
{
	if (audiobuf_created)
		return;

	// if game sound disabled don't try and play movie audio
	if (!Sound_enabled) {
		mve_audio_canplay = 0;
		return;
	}

#ifdef USE_OPENAL
    int flags, desired_buffer, sample_rate;

    mas = (mve_audio_t *) vm_malloc ( sizeof(mve_audio_t) );
	memset(mas, 0, sizeof(mve_audio_t));

	mas->format = AL_INVALID;

    flags = mve_get_ushort(data + 2);
    sample_rate = mve_get_ushort(data + 4);
    desired_buffer = mve_get_int(data + 6);

    mas->channels = (flags & 0x0001) ? 2 : 1;
	mas->bitsize = (flags & 0x0002) ? 16 : 8;

	mas->sample_rate = sample_rate;

    if (minor > 0) {
    	mve_audio_compressed = flags & 0x0004 ? 1 : 0;
    } else {
		mve_audio_compressed = 0;
    }

    if (mas->bitsize == 16) {
		if (mas->channels == 2) {
			mas->format = AL_FORMAT_STEREO16;
		} else if (mas->channels == 1) {
			mas->format = AL_FORMAT_MONO16;
		}
	} else if (mas->bitsize == 8) {
		if (mas->channels == 2) {
			mas->format = AL_FORMAT_STEREO8;
		} else if (mas->channels == 1) {
			mas->format = AL_FORMAT_MONO8;
		}
	}

	// somethings wrong, bail now
	if (mas->format == AL_INVALID) {
		mve_audio_canplay = 0;
		audiobuf_created = 1;
		return;
	}

	OpenAL_ErrorCheck( alGenSources(1, &mas->source_id), { mve_audio_canplay = 0; return; } );

	mve_audio_canplay = 1;

	OpenAL_ErrorPrint( alSourcef(mas->source_id, AL_GAIN, 1.0f) );

	memset(mas->audio_buffer, 0, MVE_AUDIO_BUFFERS * sizeof(ALuint));
#endif // USE_OPENAL

    mve_audio_buffer_head = 0;
    mve_audio_buffer_tail = 0;

	audiobuf_created = 1;
}

// play and stream the audio
void mve_audio_play()
{
	if (mve_audio_canplay) {
#ifdef USE_OPENAL
		ALint status, bqueued;

		OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_SOURCE_STATE, &status), return );
	
		OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_BUFFERS_QUEUED, &bqueued), return );
	
		mve_audio_playing = 1;

		if (status != AL_PLAYING && bqueued > 0) {
			OpenAL_ErrorPrint( alSourcePlay(mas->source_id) );
		}
#endif // USE_OPENAL
	}
}

// call this in shutdown to stop and close audio
static void mve_audio_stop()
{
	if (!audiobuf_created)
		return;

#ifdef USE_OPENAL
	ALint p = 0;

	mve_audio_playing = 0;

	OpenAL_ErrorPrint( alSourceStop(mas->source_id) );
	OpenAL_ErrorPrint( alGetSourcei(mas->source_id, AL_BUFFERS_PROCESSED, &p) );
	OpenAL_ErrorPrint( alSourceUnqueueBuffers(mas->source_id, p, mas->audio_buffer) );

	for (int i = 0; i < MVE_AUDIO_BUFFERS; i++) {
		// make sure that the buffer is real before trying to delete, it could crash for some otherwise
		if ( (mas->audio_buffer[i] != 0) && alIsBuffer(mas->audio_buffer[i]) ) {
			OpenAL_ErrorPrint( alDeleteBuffers(1, &mas->audio_buffer[i]) );
		}
	}

	OpenAL_ErrorPrint( alDeleteSources(1, &mas->source_id) );

	if (mas != NULL) {
		vm_free(mas);
		mas = NULL;
	}
#endif // USE_OPENAL
}

int mve_audio_data(ubyte major, ubyte *data)
{
	static const int selected_chan=1;
	int chan;
	int nsamp;

	if (mve_audio_canplay) {
#ifdef USE_OPENAL
		chan = mve_get_ushort(data + 2);
		nsamp = mve_get_ushort(data + 4);

		if (chan & selected_chan) {
			ALint bprocessed, bqueued, status;
			ALuint bid;

			OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_BUFFERS_PROCESSED, &bprocessed), return 0 );

			while (bprocessed-- > 2) {
				OpenAL_ErrorPrint( alSourceUnqueueBuffers(mas->source_id, 1, &bid) );
		
				if (++mve_audio_buffer_head == MVE_AUDIO_BUFFERS)
					mve_audio_buffer_head = 0;
			}

			OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_BUFFERS_QUEUED, &bqueued), return 0 );
		    
			if (bqueued == 0) 
				mprintf(("MVE: Buffer underun (First is normal)\n"));

			OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_SOURCE_STATE, &status), return 0 );

			if ( (mve_audio_playing) && (status != AL_PLAYING) && (bqueued > 0) ) {
				OpenAL_ErrorCheck( alSourcePlay(mas->source_id), return 0 );
			}

			if (bqueued < MVE_AUDIO_BUFFERS) {
				short *buf = NULL;

				/* HACK: +4 mveaudio_uncompress adds 4 more bytes */
				if (major == 8) {
				    if (mve_audio_compressed) {
						nsamp += 4;

						buf = (short *)vm_malloc(nsamp);
						mveaudio_uncompress(buf, data, -1); /* XXX */
					} else {
						nsamp -= 8;
						data += 8;

						buf = (short *)vm_malloc(nsamp);
						memcpy(buf, data, nsamp);
					}	              
				} else {
					buf = (short *)vm_malloc(nsamp);

					memset(buf, 0, nsamp); /* XXX */
				}


				if (!mas->audio_buffer[mve_audio_buffer_tail]) {
					OpenAL_ErrorCheck( alGenBuffers(1,&mas->audio_buffer[mve_audio_buffer_tail]), { vm_free(buf); return 0; } );
				}

				OpenAL_ErrorCheck( alBufferData(mas->audio_buffer[mve_audio_buffer_tail], mas->format, buf, nsamp, mas->sample_rate), { vm_free(buf); return 0; } );
	    
				OpenAL_ErrorCheck( alSourceQueueBuffers(mas->source_id, 1, &mas->audio_buffer[mve_audio_buffer_tail]), { vm_free(buf); return 0;} );

				//fprintf(stderr,"Queued buffer %d(%d)\n", mve_audio_buftail, mas->audio_buffer[mve_audio_buftail]);

				if (++mve_audio_buffer_tail == MVE_AUDIO_BUFFERS)
					mve_audio_buffer_tail = 0;

				bqueued++;
				vm_free(buf);
			} else {
				mprintf(("MVE: Buffer overrun: Queue full\n"));
			}
		}
#endif // USE_OPENAL
	}

	return 1;
}

/*************************
 * video handlers
 *************************/

int mve_video_createbuf(ubyte minor, ubyte *data)
{
	if (videobuf_created)
		return 1;

	short w, h;
	short count, truecolor;
	w = mve_get_short(data);
	h = mve_get_short(data+2);
	
	if (minor > 0) {
		count = mve_get_short(data+4);
	} else {
		count = 1;
	}
	
	if (minor > 1) {
		truecolor = mve_get_short(data+6);
	} else {
		truecolor = 0;
	}

	g_width = w << 3;
	g_height = h << 3;

	g_vBackBuf1 = g_vBuffers = vm_malloc(g_width * g_height * 4);

	if (g_vBackBuf1 == NULL) {
		nprintf(("MOVIE", "ERROR: Can't allocate video buffer"));
		videobuf_created = 1;
		return 0;
	}

	g_vBackBuf2 = (ushort *)g_vBackBuf1 + (g_width * g_height);
		
	memset(g_vBackBuf1, 0, g_width * g_height * 4);

	videobuf_created = 1;

	return 1;
}

static void mve_convert_and_draw()
{
	ushort *pDests;
	ushort *pSrcs;
	ushort *pixels = (ushort *)g_vBackBuf1;
	int x, y;

	pSrcs = pixels;

	pDests = pixelbuf;

	if (g_screenWidth > g_width) {
		pDests += ((g_screenWidth - g_width) / 2) / 2;
	}
	if (g_screenHeight > g_height) {
		pDests += ((g_screenHeight - g_height) / 2) * g_screenWidth;
	}

	for (y=0; y<g_height; y++) {
		for (x = 0; x < g_width; x++) {
			pDests[x] = (1<<15)|*pSrcs;

			pSrcs++;
		}
		pDests += g_screenWidth;
	}
}

void mve_video_display()
{
	fix t1 = timer_get_fixed_seconds();

	mve_convert_and_draw();

	// this thing is basically stupid but I don't care
	// micro_frame_delay is divided by 10 to match mve_video_skiptimer overflow catch
	if ( mve_video_skiptimer > (uint)(micro_frame_delay/10) ) {
		// we are running slow so subtract desired time from actual and skip this frame
		mve_video_skiptimer -= (micro_frame_delay/10);
		return;
	} else {
		// zero out so we can get a new count
		mve_video_skiptimer = 0;
	}

	// because of something freaky in the Windows version, we need to clear the color buffer each frame
	gr_clear();

	if (gr_screen.mode == GR_OPENGL) {
		glBindTexture(GL_TEXTURE_2D, GLtex);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_screenWidth, g_screenHeight, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, pixelbuf);

		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
				glVertex2i(g_screenX, g_screenY);

			glTexCoord2f(0, i2fl(g_screenHeight)/i2fl(hp2));
				glVertex2i(g_screenX, g_screenY+g_screenHeight);

			glTexCoord2f(i2fl(g_screenWidth)/i2fl(wp2), i2fl(g_screenHeight)/i2fl(hp2));
				glVertex2i(g_screenX+g_screenWidth, g_screenY+g_screenHeight);

			glTexCoord2f(i2fl(g_screenWidth)/i2fl(wp2), 0);
				glVertex2i(g_screenX+g_screenWidth, g_screenY);
		glEnd();
	} else {
		// DDOI - This is probably really fricking slow
		int bitmap = bm_create (16, g_screenWidth, g_screenHeight, pixelbuf, 0);
		gr_set_bitmap (bitmap);
		gr_bitmap (g_screenX, g_screenY, true);
		bm_release (bitmap);
	}

	gr_flip();
	os_poll();

	fix t2 = timer_get_fixed_seconds();

	// only get a new count if we are definitely through with old count
	if ( mve_video_skiptimer == 0 ) {
		// for a more accurate count convert the frame rate to a float and multiply
		// by one-hundred-thousand before converting to an uint.
		mve_video_skiptimer = (uint)(f2fl(t2-t1) * 100000);
	}

	int k = key_inkey();
	switch (k) {
		case KEY_ESC:
		case KEY_ENTER:
		case KEY_SPACEBAR:
			mve_playing = 0;
	}

	//mprintf(("mve frame took this long: %.6f\n", f2fl(t2-t1) / 1000.0f));

}

int mve_video_init(ubyte *data)
{
	if (video_inited)
		return 1;

	short width, height;

	width = mve_get_short(data);
	height = mve_get_short(data+2);

	// DDOI - Allocate RGB565 pixel buffer
	pixelbuf = (ushort *)vm_malloc(width * height * 2);

	if (pixelbuf == NULL) {
		nprintf(("MOVIE", "ERROR: Can't allocate memory for pixelbuf"));
		video_inited = 1;
		return 0;
	}

	memset(pixelbuf, 0, width * height * 2);

	g_screenWidth = width;
	g_screenHeight = height;

	if (gr_screen.mode == GR_OPENGL) {
		opengl_tcache_get_adjusted_texture_size(g_screenWidth, g_screenHeight, &wp2, &hp2);

		glGenTextures(1, &GLtex);

		Assert( GLtex != 0 );

		if ( GLtex == 0 ) {
			nprintf(("MOVIE", "ERROR: Can't create a GL texture"));
			video_inited = 1;
			return 0;
		}

		// disable everything but what we need
		opengl_switch_arb(-1, 0);
		opengl_switch_arb(0, 1);

		gr_set_lighting(false, false);
	
		glBindTexture(GL_TEXTURE_2D, GLtex);
	
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		float scale_by = (float)gr_screen.max_w / (float)g_screenWidth;

		// don't bother setting anything if we aren't going to need it
		if (!Cmdline_noscalevid && (scale_by != 1.0f)) {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glScalef( scale_by, scale_by, 1.0f );
			mve_scale_video = 1;
		}

		// NOTE: using NULL instead of pixelbuf crashes some drivers, but then so does pixelbuf
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, wp2, hp2, 0, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);
	}
#ifndef NO_DIRECT3D
	else if (gr_screen.mode == GR_DIRECT3D) {
		// TODO: is there a fast way to do frames in D3D too?
	}
#endif

	if (mve_scale_video) {
		g_screenX = g_screenY = 0;
	} else {
		// centers on 1024x768, fills on 640x480
		g_screenX = ((gr_screen.max_w - g_screenWidth) / 2);
		g_screenY = ((gr_screen.max_h - g_screenHeight) / 2);
	}

	memset(g_palette, 0, 768);
	
	video_inited = 1;
	
	return 1;
}

void mve_video_palette(ubyte *data)
{
	short start, count;
	start = mve_get_short(data);
	count = mve_get_short(data+2);
	memcpy(g_palette + 3*start, data+4, 3*count);
}

void mve_video_codemap(ubyte *data, int len)
{
	g_pCurMap = data;
	g_nMapLength = len;
}

void mve_video_data(ubyte *data, int len)
{
	short nFrameHot, nFrameCold;
	short nXoffset, nYoffset;
	short nXsize, nYsize;
	ushort nFlags;
	ubyte *temp;

	nFrameHot = mve_get_short(data);
	nFrameCold = mve_get_short(data+2);
	nXoffset = mve_get_short(data+4);
	nYoffset = mve_get_short(data+6);
	nXsize = mve_get_short(data+8);
	nYsize = mve_get_short(data+10);
	nFlags = mve_get_ushort(data+12);

	if (nFlags & 1) {
		temp = (ubyte *)g_vBackBuf1;
		g_vBackBuf1 = g_vBackBuf2;
		g_vBackBuf2 = temp;
	}

	decodeFrame16((ubyte *)g_vBackBuf1, g_pCurMap, g_nMapLength, data+14, len-14);
}

void mve_end_chunk()
{
	g_pCurMap = NULL;
}

void mve_init(MVESTREAM *mve)
{
	// reset to default values
	mve_audio_buffer_curpos = 0;
	mve_audio_buffer_head = 0;
	mve_audio_buffer_tail = 0;
	mve_audio_playing = 0;
	mve_audio_canplay = 0;
	mve_audio_compressed = 0;
	audiobuf_created = 0;

	videobuf_created = 0;
	video_inited = 0;
	mve_scale_video = 0;

	mve_playing = 1;
}

void mve_play(MVESTREAM *mve)
{
	int init_timer = 0, timer_error = 0;
	int cont = 1;

	if (!timer_started)
		mve_timer_start();

	while (cont && mve_playing && !timer_error) {
		cont = mve_play_next_chunk(mve);

		if (micro_frame_delay && !init_timer) {
			mve_timer_start();
			init_timer = 1;
		}

		timer_error = mve_do_timer_wait();
	}
}

void mve_shutdown()
{
	if (gr_screen.mode == GR_OPENGL) {
		if (mve_scale_video) {
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glLoadIdentity();
		}

		opengl_switch_arb(-1, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &GLtex);
		GLtex = 0;
	
		glEnable(GL_DEPTH_TEST);
	}

	if (pixelbuf != NULL) {
		vm_free(pixelbuf);
		pixelbuf = NULL;
	}

	if (g_vBuffers != NULL) {
		vm_free(g_vBuffers);
		g_vBuffers = NULL;
	}

	mve_audio_stop();

	mve_timer_stop();
}
