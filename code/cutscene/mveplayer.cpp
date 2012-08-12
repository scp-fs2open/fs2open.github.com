

#ifdef SCP_UNIX
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#endif

#include "graphics/gropengl.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglstate.h"

#include "globalincs/pstypes.h"
#include "cutscene/mvelib.h"
#include "cutscene/movie.h"
#include "graphics/2d.h"
#include "io/key.h"
#include "osapi/osapi.h"
#include "io/timer.h"
#include "sound/sound.h"
#include "sound/openal.h"
#include "bmpman/bmpman.h"


extern int Cmdline_noscalevid;

static int mve_playing;

// timer variables
static int micro_frame_delay = 0;
static int timer_started = 0;
#ifdef SCP_UNIX
static struct timeval timer_expire = { 0, 0 };
#else
static int timer_expire;
#endif

// audio variables
#define MVE_AUDIO_BUFFERS 64  // total buffers to interact with stream
static int mve_audio_buffer_tail = 0;
static int mve_audio_playing = 0;
static int mve_audio_canplay = 0;
static int mve_audio_compressed = 0;
static int audiobuf_created;

// struct for the audio stream information
typedef struct MVE_AUDIO_T {
	ALenum format;
	int sample_rate;
	int bytes_per_sec;
	int channels;
	int bitsize;
	ALuint source_id;
	ALuint audio_buffer[MVE_AUDIO_BUFFERS];
} mve_audio_t;

mve_audio_t *mas;  // mve_audio_stream

// audio decompressor
extern void mveaudio_uncompress(short *buffer, unsigned char *data);

// video variables
int g_width, g_height;
void *g_vBuffers = NULL;
void *g_vBackBuf1, *g_vBackBuf2;
ushort *pixelbuf = NULL;
static GLuint GLtex = 0;
static GLfloat gl_screenYH = 0;
static GLfloat gl_screenXW = 0;
static GLfloat gl_screenU = 0;
static GLfloat gl_screenV = 0;
static GLfloat glVertices[4][4] = {{0}};
static int g_screenWidth, g_screenHeight;
static float g_screenX, g_screenY;
static int g_truecolor = 0;
static ubyte g_palette[768];
static ubyte *g_pCurMap = NULL;
static int g_nMapLength = 0;
static int videobuf_created, video_inited;
static int hp2, wp2;
static uint mve_video_skiptimer = 0;
static int mve_scale_video = 0;

// video externs from API graphics functions
extern void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out);
extern GLenum GL_previous_texture_target;

// the decoders
extern void decodeFrame16(ubyte *pFrame, ubyte *pMap, int mapRemain, ubyte *pData, int dataRemain);
extern void decodeFrame8(ubyte *pFrame, ubyte *pMap, int mapRemain, ubyte *pData, int dataRemain);


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
	int rate = mve_get_int(data);
	int subd = mve_get_short(data+4);

	micro_frame_delay = rate * subd;

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
	if (!timer_started) {
		return 0;
	}

#ifdef SCP_UNIX
	int nsec = 0;
	struct timespec ts, tsRem;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if (tv.tv_sec > timer_expire.tv_sec) {
		goto end;
	}

	if ( (tv.tv_sec == timer_expire.tv_sec) && (tv.tv_usec >= timer_expire.tv_usec) ) {
		goto end;
	}

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

	if (tv > timer_expire) {
		goto end;
	}

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
	if (audiobuf_created) {
		return;
	}

	// if game sound disabled don't try and play movie audio
	if (!Sound_enabled) {
		mve_audio_canplay = 0;
		return;
	}

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

	mve_audio_buffer_tail = 0;

	audiobuf_created = 1;
}

// play and stream the audio
void mve_audio_play()
{
	if (mve_audio_canplay) {
		ALint status, bqueued;

		OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_SOURCE_STATE, &status), return );

		OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_BUFFERS_QUEUED, &bqueued), return );

		mve_audio_playing = 1;

		if (status != AL_PLAYING && bqueued > 0) {
			OpenAL_ErrorPrint( alSourcePlay(mas->source_id) );
		}
	}
}

// call this in shutdown to stop and close audio
static void mve_audio_stop()
{
	if (!audiobuf_created) {
		return;
	}

	ALint p = 0;

	mve_audio_playing = 0;

	OpenAL_ErrorPrint( alSourceStop(mas->source_id) );
	OpenAL_ErrorPrint( alGetSourcei(mas->source_id, AL_BUFFERS_PROCESSED, &p) );
	OpenAL_ErrorPrint( alSourceUnqueueBuffers(mas->source_id, p, mas->audio_buffer) );
	OpenAL_ErrorPrint( alDeleteSources(1, &mas->source_id) );

	for (int i = 0; i < MVE_AUDIO_BUFFERS; i++) {
		// make sure that the buffer is real before trying to delete, it could crash for some otherwise
		if ( (mas->audio_buffer[i] != 0) && alIsBuffer(mas->audio_buffer[i]) ) {
			OpenAL_ErrorPrint( alDeleteBuffers(1, &mas->audio_buffer[i]) );
		}
	}

	if (mas != NULL) {
		vm_free(mas);
		mas = NULL;
	}
}

int mve_audio_data(ubyte major, ubyte *data)
{
	static const int selected_chan=1;
	int chan;
	int nsamp;

	if (mve_audio_canplay) {
		chan = mve_get_ushort(data + 2);
		nsamp = mve_get_ushort(data + 4);

		if (chan & selected_chan) {
			ALint bprocessed, bqueued, status;
			ALuint bid;

			OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_BUFFERS_PROCESSED, &bprocessed), return 0 );

			while (bprocessed-- > 2) {
				OpenAL_ErrorPrint( alSourceUnqueueBuffers(mas->source_id, 1, &bid) );
			}

			OpenAL_ErrorCheck( alGetSourcei(mas->source_id, AL_BUFFERS_QUEUED, &bqueued), return 0 );

			if (bqueued == 0) {
				mprintf(("MVE: Buffer underun (First is normal)\n"));
			}

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
						mveaudio_uncompress(buf, data); /* XXX */
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

				if (++mve_audio_buffer_tail == MVE_AUDIO_BUFFERS) {
					mve_audio_buffer_tail = 0;
				}

				bqueued++;
				vm_free(buf);
			} else {
				mprintf(("MVE: Buffer overrun: Queue full\n"));
			}
		}
	}

	return 1;
}

/*************************
 * video handlers
 *************************/

int mve_video_createbuf(ubyte minor, ubyte *data)
{
	if (videobuf_created) {
		return 1;
	}

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

	if (truecolor) {
		g_vBackBuf2 = (ushort *)g_vBackBuf1 + (g_width * g_height);
	} else {
		g_vBackBuf2 = (ubyte *)g_vBackBuf1 + (g_width * g_height);
	}
	memset(g_vBackBuf1, 0, g_width * g_height * 4);

	g_truecolor = truecolor;
	videobuf_created = 1;

	if (gr_screen.mode == GR_OPENGL) {
		GLfloat scale_by = 1.0f;

		float screen_ratio = (float)gr_screen.max_w / (float)gr_screen.max_h;
		float movie_ratio = (float)g_width / (float)g_height;

		if (screen_ratio > movie_ratio) {
			scale_by = (float)gr_screen.max_h / (float)g_height;
		} else {
			scale_by = (float)gr_screen.max_w / (float)g_width;
		}

		// don't bother setting anything if we aren't going to need it
		if (!Cmdline_noscalevid && (scale_by != 1.0f)) {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glScalef( scale_by, scale_by, 1.0f );
			mve_scale_video = 1;
		}

		if (mve_scale_video) {
			g_screenX = ((ceil((gr_screen.max_w / scale_by) - 0.5f) - g_width) / 2);
			g_screenY = ((ceil((gr_screen.max_h / scale_by) - 0.5f) - g_height) / 2);
		} else {
			// centers on 1024x768, fills on 640x480
			g_screenX = ((float)(gr_screen.max_w - g_width) / 2.0f);
			g_screenY = ((float)(gr_screen.max_h - g_height) / 2.0f);
		}

		// set additional values for screen width/height and UV coords
		if (gr_screen.mode == GR_OPENGL) {
			gl_screenYH = g_screenY + g_height;
			gl_screenXW = g_screenX + g_width;

			gl_screenU = i2fl(g_width) / i2fl(wp2);
			gl_screenV = i2fl(g_height) / i2fl(hp2);
		}

		glVertices[0][0] = g_screenX;
		glVertices[0][1] = g_screenY;
		glVertices[0][2] = 0;
		glVertices[0][3] = 0;

		glVertices[1][0] = g_screenX;
		glVertices[1][1] = gl_screenYH;
		glVertices[1][2] = 0;
		glVertices[1][3] = gl_screenV;

		glVertices[2][0] = gl_screenXW;
		glVertices[2][1] = g_screenY;
		glVertices[2][2] = gl_screenU;
		glVertices[2][3] = 0;

		glVertices[3][0] = gl_screenXW;
		glVertices[3][1] = gl_screenYH;
		glVertices[3][2] = gl_screenU;
		glVertices[3][3] = gl_screenV;

		GL_state.Array.BindArrayBuffer(0);

		GL_state.Array.EnableClientVertex();
		GL_state.Array.VertexPointer(2, GL_FLOAT, sizeof(glVertices[0]), glVertices);

		GL_state.Array.SetActiveClientUnit(0);
		GL_state.Array.EnableClientTexture();
		GL_state.Array.TexPointer(2, GL_FLOAT, sizeof(glVertices[0]), &(glVertices[0][2]));
	}

	return 1;
}

static void mve_convert_and_draw()
{
	ushort *pDests;
	ushort *pSrcs = NULL;
	ushort *pixels = (ushort *)g_vBackBuf1;
	int x, y;

	ubyte *pSrcs8 = NULL;
	ubyte *pixels8 = (ubyte *)g_vBackBuf1;
	ubyte r, g, b;
	ushort bit_16;

	if (g_truecolor) {
		pSrcs = pixels;
	} else {
		pSrcs8 = pixels8;
	}
	pDests = pixelbuf;

	if (gr_screen.mode != GR_OPENGL) {
		if (g_screenWidth > g_width) {
			pDests += ((g_screenWidth - g_width) / 2) / 2;
		}
		if (g_screenHeight > g_height) {
			pDests += ((g_screenHeight - g_height) / 2) * g_screenWidth;
		}
	}

	for (y=0; y<g_height; y++) {
		for (x = 0; x < g_width; x++) {
			if (g_truecolor) {
				Assert( pSrcs != NULL );
				pDests[x] = (1<<15)|*pSrcs;
				pSrcs++;
			} else {
				Assert( pSrcs8 != NULL );

				// grab rgb color
				r = (g_palette[(*pSrcs8)*3] / 2);
				g = (g_palette[(*pSrcs8)*3 + 1] / 2);
				b = (g_palette[(*pSrcs8)*3 + 2] / 2);

				// stuff the color
				bit_16 = (ushort)(r << Gr_t_red.shift);
				bit_16 |= (ushort)(g << Gr_t_green.shift);
				bit_16 |= (ushort)(b << Gr_t_blue.shift);
				bit_16 |= (ushort)(Gr_t_alpha.mask);

				// stuff the pixel
				pDests[x] = (1<<15)|bit_16;

				pSrcs8++;
			}
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

	if (gr_screen.mode == GR_OPENGL) {
		glTexSubImage2D(GL_state.Texture.GetTarget(), 0, 0, 0, g_width, g_height, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, pixelbuf);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	} else {
		// DDOI - This is probably really fricking slow
		int bitmap = bm_create (16, g_screenWidth, g_screenHeight, pixelbuf, 0);
		gr_set_bitmap (bitmap);
		gr_bitmap ((int)g_screenX, (int)g_screenY, true);
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
	if (video_inited) {
		return 1;
	}

	short width, height;

	width = mve_get_short(data);
	height = mve_get_short(data+2);

	g_screenWidth = width;
	g_screenHeight = height;

	// DDOI - Allocate RGB565 pixel buffer
	pixelbuf = (ushort *)vm_malloc(g_screenWidth * g_screenHeight * 2);

	if (pixelbuf == NULL) {
		nprintf(("MOVIE", "ERROR: Can't allocate memory for pixelbuf"));
		video_inited = 1;
		return 0;
	}

	memset(pixelbuf, 0, g_screenWidth * g_screenHeight * 2);

	if (gr_screen.mode == GR_OPENGL) {
		opengl_set_texture_target(GL_TEXTURE_2D);
		opengl_tcache_get_adjusted_texture_size(g_screenWidth, g_screenHeight, &wp2, &hp2);

		glGenTextures(1, &GLtex);

		Assert( GLtex != 0 );

		if ( GLtex == 0 ) {
			nprintf(("MOVIE", "ERROR: Can't create a GL texture"));
			video_inited = 1;
			return 0;
		}

		gr_set_lighting(false, false);
		GL_state.Texture.DisableAll();

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_texture_target);
		GL_state.Texture.Enable(GLtex);

		GL_state.SetTextureSource(TEXTURE_SOURCE_DECAL);
		GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

		glTexParameteri(GL_texture_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_texture_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_texture_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_texture_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// NOTE: using NULL instead of pixelbuf crashes some drivers, but then so does pixelbuf
		glTexImage2D(GL_state.Texture.GetTarget(), 0, GL_RGB5_A1, wp2, hp2, 0, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, NULL);

		// set our color so that we can make sure that it's correct
		glColor3f(1.0f, 1.0f, 1.0f);
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

	if (g_truecolor) {
		decodeFrame16((ubyte *)g_vBackBuf1, g_pCurMap, g_nMapLength, data+14, len-14);
	} else {
		decodeFrame8((ubyte *)g_vBackBuf1, g_pCurMap, g_nMapLength, data+14, len-14);
	}
}

void mve_end_chunk()
{
	g_pCurMap = NULL;
}

void mve_init(MVESTREAM *mve)
{
	// reset to default values
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

	if (!timer_started) {
		mve_timer_start();
	}

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
		GL_state.Array.DisableClientVertex();
		GL_state.Array.DisableClientTexture();

		if (mve_scale_video) {
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		}

		GL_state.Texture.Disable();
		GL_state.Texture.Delete(GLtex);
		glDeleteTextures(1, &GLtex);
		GLtex = 0;
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
