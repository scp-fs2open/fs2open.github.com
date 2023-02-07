

#ifdef _WIN32
#define VC_EXTRALEAN
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <mmsystem.h>
#endif

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/IAudioFile.h"
#include "sound/sound.h"
#include "sound/openal.h"
#include "gamesnd/eventmusic.h"

#ifdef WITH_FFMPEG
#include "sound/ffmpeg/FFmpegWaveFile.h"
#endif

#define MAX_STREAM_BUFFERS 4

// status
#define ASF_FREE	0
#define ASF_USED	1

// constants
#define BIGBUF_SIZE					176400
ubyte *Wavedata_load_buffer = NULL;		// buffer used for cueing audiostreams
ubyte *Wavedata_service_buffer = NULL;	// buffer used for servicing audiostreams

SDL_mutex* Global_service_lock;

typedef bool (*TIMERCALLBACK)(ptr_u);

#define COMPRESSED_BUFFER_SIZE	176400
ubyte *Compressed_buffer = NULL;				// Used to load in compressed data during a cueing interval
ubyte *Compressed_service_buffer = NULL;	// Used to read in compressed data during a service interval

// Globalize the list of audio extensions for use in several sound related files
const char *audio_ext_list[] = { ".ogg", ".wav" };
const int NUM_AUDIO_EXT = sizeof(audio_ext_list) / sizeof(char*);

static std::unique_ptr<sound::IAudioFile> openAudioFile(const char* fileName, bool keep_ext) {
#ifdef WITH_FFMPEG
	{
		std::unique_ptr<sound::IAudioFile> audio_file(new sound::ffmpeg::FFmpegWaveFile());

		// Open given file
		if (audio_file->Open(fileName, keep_ext)) {
			return audio_file;
		}
	}
#endif

	return nullptr;
}

static std::unique_ptr<sound::IAudioFile> openAudioMem(const uint8_t* snddata, size_t snd_len) {
#ifdef WITH_FFMPEG
	{
		std::unique_ptr<sound::IAudioFile> audio_file(new sound::ffmpeg::FFmpegWaveFile());

		// Open given in-memory file
		if (audio_file->OpenMem(snddata, snd_len)) {
			return audio_file;
		}
	}
#endif

	return nullptr;
}

int Audiostream_inited = 0;

class Timer
{
public:
	void constructor();
	void destructor();
	bool Create (uint nPeriod, uint nRes, ptr_u dwUser, TIMERCALLBACK pfnCallback);
protected:
	static uint TimeProc(uint interval, void *param);
	TIMERCALLBACK m_pfnCallback;
	ptr_u m_dwUser;
	uint m_nPeriod;
	uint m_nRes;
	SDL_TimerID m_nIDTimer;
};

class AudioStream
{
public:
	AudioStream ();
	~AudioStream ();
	bool Create (char *pszFilename);
	bool CreateMem (const uint8_t* snddata, size_t snd_len);
	bool Destroy ();
	void Play (float volume, int looping);
	bool Is_Playing(){ return m_fPlaying; }
	bool Is_Paused(){ return m_bIsPaused; }
	bool Is_Past_Limit() { return m_bPastLimit; }
	void Stop (int paused = 0);
	void Stop_and_Rewind ();
	void Fade_and_Destroy ();
	void Fade_and_Stop();
	void	Set_Volume(float vol);
	float	Get_Volume();
	double  Get_Duration();
	void	Init_Data();
	void	Set_Sample_Cutoff(uint sample_cutoff);
	void	Set_Default_Volume(float vol) { m_lDefaultVolume = vol; }
	float	Get_Default_Volume() { return m_lDefaultVolume; }
	uint	Get_Samples_Committed();
	int	Is_looping() { return m_bLooping; }
	int	status;
	int	type;
	bool paused_via_sexp_or_script;

protected:
	bool prepareOpened(const char *filename);
	void Cue ();
	bool WriteWaveData (uint cbSize, uint *num_bytes_written, int service = 1);
	uint GetMaxWriteSize ();
	bool ServiceBuffer ();
	static bool TimerCallback (ptr_u dwUser);
	bool PlaybackDone();

	ALuint m_source_id;	// name of openAL source
	ALuint m_buffer_ids[MAX_STREAM_BUFFERS];	// names of buffers

	Timer m_timer;			// ptr to Timer object
	std::unique_ptr<sound::IAudioFile> m_pwavefile;	// ptr to WaveFile object
	sound::AudioFileProperties m_fileProps;
	bool m_fCued;			// semaphore (stream cued)
	bool m_fPlaying;		// semaphore (stream playing)
	uint m_cbBufOffset;		// last write position
	uint m_cbBufSize;		// size of sound buffer in bytes
	uint m_nBufService;		// service interval in msec
	uint m_nTimeStarted;	// time (in system time) playback started

	bool	m_bLooping;				// whether or not to loop playback
	bool	m_bFade;				// fade out music 
	bool	m_bDestroy_when_faded;
	float	m_lVolume;				// volume of stream ( 0 -> 1 )
	float	m_lCutoffVolume;
	bool	m_bIsPaused;			// stream is stopped, but not rewinded
	bool	m_bReadingDone;			// no more bytes to be read from disk, still have remaining buffer to play
	uint	m_fade_timer_id;		// timestamp so we know when to start fade
	uint	m_finished_id;			// timestamp so we know when we've played #bytes required
	bool	m_bPastLimit;			// flag to show we've played past the number of bytes requred
	float	m_lDefaultVolume;

	size_t m_total_uncompressed_bytes_read;
	size_t m_max_uncompressed_bytes_to_read;

	SDL_mutex* write_lock;

};


// Timer class implementation
//
////////////////////////////////////////////////////////////

// constructor
void Timer::constructor(void)
{
	m_nIDTimer = 0;
}


// Destructor
void Timer::destructor(void)
{
	if (m_nIDTimer) {
		SDL_RemoveTimer(m_nIDTimer);
		m_nIDTimer = 0;
	}
}

// Create
bool Timer::Create (uint nPeriod, uint nRes, ptr_u dwUser, TIMERCALLBACK pfnCallback)
{
	bool bRtn = true;	// assume success

	Assert(pfnCallback);
	Assert(nPeriod > 10);
	Assert(nPeriod >= nRes);

	m_nPeriod = nPeriod;
	m_nRes = nRes;
	m_dwUser = dwUser;
	m_pfnCallback = pfnCallback;

	if ((m_nIDTimer = SDL_AddTimer(m_nPeriod, TimeProc, (void*)this)) == 0) {
	  bRtn = false;
	}

	return (bRtn);
}


// Timer proc for multimedia timer callback set with timeSetTime().
//
// Calls procedure specified when Timer object was created. The 
// dwUser parameter contains "this" pointer for associated Timer object.
// 
uint Timer::TimeProc(uint interval, void *dwUser)
{
	// dwUser contains ptr to Timer object
	Timer * ptimer = (Timer *) dwUser;

	// Call user-specified callback and pass back user specified data
	(ptimer->m_pfnCallback) (ptimer->m_dwUser);

	if (ptimer->m_nPeriod) {
		return interval;
	} else {
		SDL_RemoveTimer(ptimer->m_nIDTimer);
		ptimer->m_nIDTimer = 0;
		return 0;
	}
}

//
// AudioStream class implementation
//
////////////////////////////////////////////////////////////

// The following constants are the defaults for our streaming buffer operation.
const ushort DefBufferServiceInterval = 250;  // default buffer service interval in msec

// Constructor
AudioStream::AudioStream (void) : m_total_uncompressed_bytes_read(0), m_max_uncompressed_bytes_to_read(0)
{
	write_lock = SDL_CreateMutex();
}

// Destructor
AudioStream::~AudioStream (void)
{
	SDL_DestroyMutex( write_lock );
}

void AudioStream::Init_Data ()
{
	m_bLooping = 0;
	m_bFade = false;
	m_fade_timer_id = 0;
	m_finished_id = 0;
	m_bPastLimit = false;
	
	m_bDestroy_when_faded = false;
	m_lVolume = 1.0f;
	m_lCutoffVolume = 0.0f;
	m_bIsPaused = false;
	m_bReadingDone = false;

	m_pwavefile = nullptr;
	m_fPlaying = m_fCued = false;
	m_cbBufOffset = 0;
	m_cbBufSize = 0;
	m_nBufService = DefBufferServiceInterval;
	m_nTimeStarted = 0;

	memset(m_buffer_ids, 0, sizeof(m_buffer_ids));
	m_source_id = 0;

	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = std::numeric_limits<size_t>::max();
}


bool AudioStream::prepareOpened(const char *filename)
{
	bool fRtn = true;

	m_fileProps = m_pwavefile->getFileProperties();

	m_cbBufSize = (m_fileProps.sample_rate * m_fileProps.bytes_per_sample * m_fileProps.num_channels) >> 2;
	// make sure that we are a multiple of the frame size
	m_cbBufSize -= (m_cbBufSize % (m_fileProps.bytes_per_sample * m_fileProps.num_channels));
	m_cbBufSize += (m_cbBufSize % 12) << 1;
	// if the requested buffer size is too big then cap it
	m_cbBufSize = (m_cbBufSize > BIGBUF_SIZE) ? BIGBUF_SIZE : m_cbBufSize;

	//				nprintf(("SOUND", "SOUND => Stream buffer created using %d bytes\n", m_cbBufSize));

	OpenAL_ErrorCheck( alGenSources(1, &m_source_id), { fRtn = false; goto ErrorExit; } );

	OpenAL_ErrorCheck( alGenBuffers(MAX_STREAM_BUFFERS, m_buffer_ids), { fRtn = false; goto ErrorExit; } );

	OpenAL_ErrorPrint( alSourcef(m_source_id, AL_ROLLOFF_FACTOR, 1.0f) );
	OpenAL_ErrorPrint( alSourcei(m_source_id, AL_SOURCE_RELATIVE, AL_TRUE) );

	OpenAL_ErrorPrint( alSource3f(m_source_id, AL_POSITION, 0.0f, 0.0f, 0.0f) );
	OpenAL_ErrorPrint( alSource3f(m_source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f) );

	OpenAL_ErrorPrint( alSourcef(m_source_id, AL_GAIN, 1.0f) );
	OpenAL_ErrorPrint( alSourcef(m_source_id, AL_PITCH, 1.0f) );

	// maybe set EFX
	if ( (type == ASF_SOUNDFX) && ds_eax_is_inited() ) {
		extern ALuint AL_EFX_aux_id;
		OpenAL_ErrorPrint( alSource3i(m_source_id, AL_AUXILIARY_SEND_FILTER, AL_EFX_aux_id, 0, AL_FILTER_NULL) );
	}

	Snd_sram += (m_cbBufSize * MAX_STREAM_BUFFERS);

ErrorExit:
	if ( (fRtn == false) && (m_pwavefile) ) {
		mprintf(("AUDIOSTR => ErrorExit for ::prepareOpened() on wave file: %s\n", filename));

		if (m_source_id)
			OpenAL_ErrorPrint( alDeleteSources(1, &m_source_id) );

		m_pwavefile = nullptr;
	}

	return fRtn;
}

// Create
bool AudioStream::Create (char *pszFilename)
{
	Assert(pszFilename);

	Init_Data();

	if ( ! pszFilename )
		return false;
	// make 100% sure we got a good filename
	if ( !strlen(pszFilename) )
		return false;

	// Create a new WaveFile object and open it
	m_pwavefile = openAudioFile(pszFilename, (type == ASF_EVENTMUSIC));
	if (m_pwavefile) {
		return prepareOpened(pszFilename);
	}
	else {
		// Error, unable to create WaveFile object
		nprintf(("Sound", "SOUND => Failed to open wave file %s\n", pszFilename));
		return false;
	}

}

bool AudioStream::CreateMem (const uint8_t* snddata, size_t snd_len)
{
	Init_Data();
	
	// Create a new WaveFile object and open it
	m_pwavefile = openAudioMem(snddata, snd_len);
	if (m_pwavefile) {
		return prepareOpened("in-memory");
	}
	else {
		// Error, unable to create WaveFile object
		nprintf(("Sound", "SOUND => Failed to open in-memory wave file \n"));
		return false;
	}
}

// Destroy
bool AudioStream::Destroy (void)
{
	bool fRtn = true;
	ALint buffers_processed = 0;

	SDL_LockMutex(write_lock);

	// Stop playback
	Stop ();

	OpenAL_ErrorPrint( alGetSourcei(m_source_id, AL_BUFFERS_PROCESSED, &buffers_processed) );

	while (buffers_processed) {
		ALuint buffer_id = 0;
		OpenAL_ErrorPrint( alSourceUnqueueBuffers(m_source_id, 1, &buffer_id) );
		buffers_processed--;
	}

	// Release sound sources and buffers
	OpenAL_ErrorPrint( alDeleteSources(1, &m_source_id) );
	OpenAL_ErrorPrint( alDeleteBuffers(MAX_STREAM_BUFFERS, m_buffer_ids) );

	Snd_sram -= (m_cbBufSize * MAX_STREAM_BUFFERS);

	// Delete WaveFile object
	m_pwavefile = nullptr;

	status = ASF_FREE;

	SDL_UnlockMutex(write_lock);

	return fRtn;
}

// WriteWaveData
//
// Writes wave data to sound buffer. This is a helper method used by Create and
// ServiceBuffer; it's not exposed to users of the AudioStream class.
bool AudioStream::WriteWaveData (uint size, uint *num_bytes_written, int service)
{
	bool fRtn = true;
	ubyte *uncompressed_wave_data;

	*num_bytes_written = 0;

	if ( size == 0 || m_bReadingDone ) {
		return fRtn;
	}

	if ( (m_buffer_ids[0] == 0) || !m_pwavefile ) {
		return fRtn;
	}

	if ( service ) {
		SDL_LockMutex(Global_service_lock);
	}

	if ( service ) {
		uncompressed_wave_data = Wavedata_service_buffer;
	} else {
		uncompressed_wave_data = Wavedata_load_buffer;
	}

	int num_bytes_read = 0;

	const auto alFormat = openal_get_format(m_fileProps.bytes_per_sample * 8, m_fileProps.num_channels);

	if ( !service ) {
		for (int ib = 0; ib < MAX_STREAM_BUFFERS; ib++) {
			num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, m_cbBufSize);

			// if looping then maybe reset wavefile and keep going
			if ( (num_bytes_read < 0) && m_bLooping) {
				m_pwavefile->Cue();
				m_total_uncompressed_bytes_read = 0;
				num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, m_cbBufSize);
			}

			if (num_bytes_read < 0) {
				m_bReadingDone = 1;
				break;
			} else if (num_bytes_read > 0) {
				OpenAL_ErrorCheck( alBufferData(m_buffer_ids[ib], alFormat, uncompressed_wave_data, num_bytes_read, m_fileProps.sample_rate), { fRtn = false; goto ErrorExit; } );
				OpenAL_ErrorCheck( alSourceQueueBuffers(m_source_id, 1, &m_buffer_ids[ib]), { fRtn = false; goto ErrorExit; } );

				*num_bytes_written += num_bytes_read;
			}
		}
	} else {
		ALint buffers_processed = 0;
		OpenAL_ErrorPrint( alGetSourcei(m_source_id, AL_BUFFERS_PROCESSED, &buffers_processed) );

		while (buffers_processed) {
			ALuint buffer_id = 0;
			OpenAL_ErrorPrint( alSourceUnqueueBuffers(m_source_id, 1, &buffer_id) );

			num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, m_cbBufSize);

			// if looping then maybe reset wavefile and keep going
			if ( (num_bytes_read < 0) && m_bLooping) {
				m_pwavefile->Cue();
				m_total_uncompressed_bytes_read = 0;
				num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, m_cbBufSize);
			}

			if (num_bytes_read < 0) {
				m_bReadingDone = 1;
			} else if (num_bytes_read > 0) {
				OpenAL_ErrorPrint( alBufferData(buffer_id, alFormat, uncompressed_wave_data, num_bytes_read, m_fileProps.sample_rate) );
				OpenAL_ErrorPrint( alSourceQueueBuffers(m_source_id, 1, &buffer_id) );

				*num_bytes_written += num_bytes_read;
			}

			buffers_processed--;
		}
	}

ErrorExit:
	m_total_uncompressed_bytes_read += *num_bytes_written;

	if ( service ) {
		SDL_UnlockMutex(Global_service_lock);
	}

	return (fRtn);
}

// GetMaxWriteSize
//
// Helper function to calculate max size of sound buffer write operation, i.e. how much
// free space there is in buffer.
uint AudioStream::GetMaxWriteSize (void)
{
	uint dwMaxSize = m_cbBufSize;
	ALint n, q;

	OpenAL_ErrorCheck( alGetSourcei(m_source_id, AL_BUFFERS_PROCESSED, &n), return 0 );

	OpenAL_ErrorCheck( alGetSourcei(m_source_id, AL_BUFFERS_QUEUED, &q), return 0 );

	if (!n && (q >= MAX_STREAM_BUFFERS)) //all buffers queued
		dwMaxSize = 0;

	//	nprintf(("Alan","Max write size: %d\n", dwMaxSize));
	return (dwMaxSize);
}

#define VOLUME_ATTENUATION_BEFORE_CUTOFF			0.03f
#define VOLUME_ATTENUATION							0.65f
bool AudioStream::ServiceBuffer (void)
{
	float vol;
	bool fRtn = true;

	if ( status != ASF_USED )
		return false;

	SDL_LockMutex( write_lock );

	// status may have changed, so lets check once again
	if ( status != ASF_USED ){
		SDL_UnlockMutex( write_lock );

		return false;
	}

	if ( m_bFade == true ) {
		if ( m_lCutoffVolume == 0.0f ) {
			vol = Get_Volume();
//			nprintf(("Alan","Volume is: %d\n",vol));
			m_lCutoffVolume = vol * VOLUME_ATTENUATION_BEFORE_CUTOFF;
		}

		vol = Get_Volume() * VOLUME_ATTENUATION;
//		nprintf(("Alan","Volume is now: %d\n",vol));
		Set_Volume(vol);

//		nprintf(("Sound","SOUND => Volume for stream sound is %d\n",vol));
//		nprintf(("Alan","Cuttoff Volume is: %d\n",m_lCutoffVolume));
		if ( vol < m_lCutoffVolume ) {
			m_bFade = false;
			m_lCutoffVolume = 0.0f;

			if ( m_bDestroy_when_faded == true ) {
				SDL_UnlockMutex( write_lock );

				Destroy();	
				// Reset reentrancy semaphore

				return false;
			} else {
				Stop_and_Rewind();
				// Reset reentrancy semaphore
				SDL_UnlockMutex( write_lock );

				return true;
			}
		}
	}

	// All of sound not played yet, send more data to buffer
	uint dwFreeSpace = GetMaxWriteSize ();

	// Determine free space in sound buffer
	if (dwFreeSpace) {

		// Some wave data remains, but not enough to fill free space
		// Send wave data to buffer, fill remainder of free space with silence
		uint num_bytes_written;

		if (WriteWaveData (dwFreeSpace, &num_bytes_written) == true) {
//			nprintf(("Alan","Num bytes written: %d\n", num_bytes_written));

			if ( m_total_uncompressed_bytes_read >= m_max_uncompressed_bytes_to_read ) {
				m_fade_timer_id = timer_get_milliseconds() + 1700;		// start fading 1.7 seconds from now
				m_finished_id = timer_get_milliseconds() + 2000;		// 2 seconds left to play out buffer
				m_max_uncompressed_bytes_to_read = std::numeric_limits<uint>::max();
			}

			if ( (m_fade_timer_id>0) && ((uint)timer_get_milliseconds() > m_fade_timer_id) ) {
				m_fade_timer_id = 0;
				Fade_and_Stop();
			}

			if ( (m_finished_id>0) && ((uint)timer_get_milliseconds() > m_finished_id) ) {
				m_finished_id = 0;
				m_bPastLimit = true;
			}

			if ( PlaybackDone() ) {
				if ( m_bDestroy_when_faded == true ) {
					SDL_UnlockMutex( write_lock );

					Destroy();
					// Reset reentrancy semaphore

					return false;
				}
				// All of sound has played, stop playback or loop again
				if ( m_bLooping && !m_bFade) {
					Play(m_lVolume, m_bLooping);
				} else {
					Stop_and_Rewind();
				}
			}
		}
		else {
			// Error writing wave data
			fRtn = false;
			Int3(); 
		}
	}

	SDL_UnlockMutex( write_lock );

	return (fRtn);
}

// Cue
void AudioStream::Cue (void)
{
	uint num_bytes_written;

	if (!m_fCued) {
		m_bFade = false;
		m_fade_timer_id = 0;
		m_finished_id = 0;
		m_bPastLimit = false;
		m_lVolume = 1.0f;
		m_lCutoffVolume = 0.0f;

		m_bDestroy_when_faded = false;

		// Reset buffer ptr
		m_cbBufOffset = 0;

		// Reset file ptr, etc
		m_pwavefile->Cue ();

		// Unqueue all buffers
		ALint buffers_processed = 0;
		OpenAL_ErrorPrint( alGetSourcei(m_source_id, AL_BUFFERS_PROCESSED, &buffers_processed) );

		while (buffers_processed) {
			ALuint buffer_id = 0;
			OpenAL_ErrorPrint( alSourceUnqueueBuffers(m_source_id, 1, &buffer_id) );
			buffers_processed--;
		}

		// Fill buffer with wave data
		WriteWaveData (m_cbBufSize, &num_bytes_written, 0);

		m_fCued = true;

		// Init some of our data
		m_total_uncompressed_bytes_read = 0;
		m_max_uncompressed_bytes_to_read = std::numeric_limits<uint>::max();
	}
}

// Play
void AudioStream::Play (float volume, int looping)
{
	if (m_buffer_ids[0] != 0) {
		// If playing, stop
		if (m_fPlaying) {
			if ( m_bIsPaused == false)
				Stop_and_Rewind();
		}

		// loop flag must be set before Cue()!
		if ( looping )
			m_bLooping = 1;
		else
			m_bLooping = 0;

		// Cue for playback if necessary
		if ( !m_fCued )
			Cue ();

		OpenAL_ErrorPrint( alSourcePlay(m_source_id) );

		m_nTimeStarted = timer_get_milliseconds();
		Set_Volume(volume);

		// Kick off timer to service buffer
		m_timer.constructor();

		m_timer.Create (m_nBufService, m_nBufService, ptr_u (this), TimerCallback);

		// Playback begun, no longer cued
		m_fPlaying = true;
		m_bIsPaused = false;
	}
}

// Timer callback for Timer object created by ::Play method.
bool AudioStream::TimerCallback (ptr_u dwUser)
{
	// dwUser contains ptr to AudioStream object
	AudioStream * pas = (AudioStream *) dwUser;

	return (pas->ServiceBuffer ());
}

void AudioStream::Set_Sample_Cutoff(unsigned int sample_cutoff)
{
	if ( m_pwavefile == NULL )
		return;

	m_max_uncompressed_bytes_to_read = (sample_cutoff * m_fileProps.bytes_per_sample);
}

uint AudioStream::Get_Samples_Committed(void)
{
	if ( m_pwavefile == NULL )
		return 0;

	return (uint) (m_total_uncompressed_bytes_read / m_fileProps.bytes_per_sample);
}


/** Have stream fade out and be destroyed when inaudabile.
If stream is already done or never started just destroy it now.
*/
void AudioStream::Fade_and_Destroy (void)
{
	if (!m_fPlaying || PlaybackDone())
	{
		Destroy();
	}
	else
	{
		m_bFade = true;
		m_bDestroy_when_faded = true;
	}
}

// Fade_and_Destroy
void AudioStream::Fade_and_Stop (void)
{
	m_bFade = true;
	m_bDestroy_when_faded = false;
}


// Stop
void AudioStream::Stop(int paused)
{
	if (m_fPlaying) {
		if (paused) {
			OpenAL_ErrorPrint( alSourcePause(m_source_id) );
		} else {
			OpenAL_ErrorPrint( alSourceStop(m_source_id) );
		}

		m_fPlaying = false;
		m_bIsPaused = (paused != 0);

		// Delete Timer object
		m_timer.destructor();
	}
}

// Stop_and_Rewind
void AudioStream::Stop_and_Rewind (void)
{
	if (m_fPlaying) {
		// Stop playback
		OpenAL_ErrorPrint( alSourceStop(m_source_id) );

		// Delete Timer object
		m_timer.destructor();

		m_fPlaying = false;
		m_bIsPaused = false;
	}

	// Unqueue all buffers
	ALint buffers_processed = 0;
	OpenAL_ErrorPrint( alGetSourcei(m_source_id, AL_BUFFERS_PROCESSED, &buffers_processed) );

	while (buffers_processed) {
		ALuint buffer_id = 0;
		OpenAL_ErrorPrint( alSourceUnqueueBuffers(m_source_id, 1, &buffer_id) );
		buffers_processed--;
	}

	m_fCued = false;	// this will cause wave file to start from beginning
	m_bReadingDone = false;
}

// Set_Volume
void AudioStream::Set_Volume(float vol)
{
	CAP(vol, 0.0f, 1.0f);

	OpenAL_ErrorPrint( alSourcef(m_source_id, AL_GAIN, vol) );

	m_lVolume = vol;
}


// Set_Volume
float AudioStream::Get_Volume()
{
	return m_lVolume;
}

double AudioStream::Get_Duration()
{
	return m_fileProps.duration;
}

bool AudioStream::PlaybackDone()
{
	ALint state = 0;
	OpenAL_ErrorPrint( alGetSourcei(m_source_id, AL_SOURCE_STATE, &state) );

	if (m_bReadingDone && (state != AL_PLAYING))
		return true;
	else
		return false;
}


AudioStream Audio_streams[MAX_AUDIO_STREAMS];


void audiostream_init()
{
	int i;

	if ( Audiostream_inited == 1 )
		return;

	// Allocate memory for the buffer which holds the uncompressed wave data that is streamed from the
	// disk during a load/cue
	if ( Wavedata_load_buffer == NULL ) {
		Wavedata_load_buffer = (ubyte*)vm_malloc(BIGBUF_SIZE);
		Assert(Wavedata_load_buffer != NULL);
	}

	// Allocate memory for the buffer which holds the uncompressed wave data that is streamed from the
	// disk during a service interval
	if ( Wavedata_service_buffer == NULL ) {
		Wavedata_service_buffer = (ubyte*)vm_malloc(BIGBUF_SIZE);
		Assert(Wavedata_service_buffer != NULL);
	}

	// Allocate memory for the buffer which holds the compressed wave data that is read from the hard disk
	if ( Compressed_buffer == NULL ) {
		Compressed_buffer = (ubyte*)vm_malloc(COMPRESSED_BUFFER_SIZE);
		Assert(Compressed_buffer != NULL);
	}

	if ( Compressed_service_buffer == NULL ) {
		Compressed_service_buffer = (ubyte*)vm_malloc(COMPRESSED_BUFFER_SIZE);
		Assert(Compressed_service_buffer != NULL);
	}

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		Audio_streams[i].Init_Data();
		Audio_streams[i].status = ASF_FREE;
		Audio_streams[i].type = ASF_NONE;
		Audio_streams[i].paused_via_sexp_or_script = false;
	}

	SDL_InitSubSystem(SDL_INIT_TIMER);

	Global_service_lock = SDL_CreateMutex();

	Audiostream_inited = 1;
}

// Close down the audiostream system.  Must call audiostream_init() before any audiostream functions can
// be used.
void audiostream_close()
{
	if ( Audiostream_inited == 0 )
		return;

	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_USED ) {
			Audio_streams[i].status = ASF_FREE;
			Audio_streams[i].Destroy();
		}
	}

	// free global buffers
	if ( Wavedata_load_buffer ) {
		vm_free(Wavedata_load_buffer);
		Wavedata_load_buffer = NULL;
	}

	if ( Wavedata_service_buffer ) {
		vm_free(Wavedata_service_buffer);
		Wavedata_service_buffer = NULL;
	}

	if ( Compressed_buffer ) {
		vm_free(Compressed_buffer);
		Compressed_buffer = NULL;
	}

	if ( Compressed_service_buffer ) {
		vm_free(Compressed_service_buffer);
		Compressed_service_buffer = NULL;
	}

	SDL_DestroyMutex( Global_service_lock );

	Audiostream_inited = 0;

}

static int audiostream_use_next_free( int type )
{
	if ( !Audiostream_inited || !snd_is_inited() )
		return -1;

	int i;
	for (i = 0; i < MAX_AUDIO_STREAMS; i++)
		if (Audio_streams[i].status == ASF_FREE)
			break;

	if (i == MAX_AUDIO_STREAMS) {
		nprintf(("Sound", "SOUND => No more audio streams available!\n"));
		return -1;
	}

	Audio_streams[i].status = ASF_USED;
	Audio_streams[i].type = type;

	switch (type) {
		case ASF_SOUNDFX: // As in: sound.cpp:590
			Audio_streams[i].Set_Default_Volume(Master_sound_volume * aav_effect_volume);
			break;
		case ASF_EVENTMUSIC: // As in: sexp.cpp:11562
			Audio_streams[i].Set_Default_Volume(Master_event_music_volume * aav_music_volume);
			break;
		case ASF_MENUMUSIC: // As in: mainhallmenu.cpp:1170
			Audio_streams[i].Set_Default_Volume(Master_event_music_volume);
			break;
		case ASF_VOICE: // As in: sound.cpp:590
			Audio_streams[i].Set_Default_Volume(Master_voice_volume * aav_voice_volume);
			break;
		default:
			Audio_streams[i].status = ASF_FREE;
			return -1;
	}

	return i;
}

// Open a digital sound file for streaming
//
// input:	filename	=>	disk filename of sound file
//				type	=>	what type of audio stream do we want to open:
//								ASF_SOUNDFX
//								ASF_EVENTMUSIC
//								ASF_MENUMUSIC
//								ASF_VOICE
//	
// returns:	success => handle to identify streaming sound
//				failure => -1
int audiostream_open( const char *filename, int type )
{
	int i = audiostream_use_next_free(type);
	if ( i == -1 )
		return -1;

	char fname[MAX_FILENAME_LEN];
	// copy filename, since we might modify it
	strcpy_s(fname, filename);

	switch (type)
	{
		case ASF_VOICE:
		case ASF_SOUNDFX:
		case ASF_MENUMUSIC:
		{
			// go ahead and strip off file extension
			char *p = strrchr(fname, '.');
			if ( p && (strlen(p) > 2) )
				(*p) = 0;

			break;
		}

		case ASF_EVENTMUSIC:
			break;

		default:
			Audio_streams[i].status = ASF_FREE;
			return -1;
	}

	int rc = Audio_streams[i].Create(fname);

	if ( rc == 0 ) {
		Audio_streams[i].status = ASF_FREE;
		return -1;
	} else {
		return i;
	}
}

// Open wave file contents previously loaded into memory for streaming
//
//input:	snddata	=>	reference of an in-memory file
//			snd_len	=>	length of loaded file
//			type	=>	what type of audio stream do we want to open:
//							ASF_SOUNDFX
//							ASF_EVENTMUSIC
//							ASF_MENUMUSIC
//							ASF_VOICE
//	
// returns:	success => handle to identify streaming sound
//				failure => -1
int audiostream_open_mem( const uint8_t* snddata, size_t snd_len, int type )
{
	int i = audiostream_use_next_free(type);
	if ( i == -1 )
		return -1;

	int rc = Audio_streams[i].CreateMem(snddata, snd_len);

	if ( rc == 0 ) {
		Audio_streams[i].status = ASF_FREE;
		return -1;
	} else {
		return i;
	}
}

void audiostream_close_file(int i, bool fade)
{
	if (!Audiostream_inited)
		return;

	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_USED ) {
		if ( fade )
			Audio_streams[i].Fade_and_Destroy();
		else
			Audio_streams[i].Destroy();
	}

}

void audiostream_close_all(bool fade)
{
	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE )
			continue;

		audiostream_close_file(i, fade);
	}
}

void audiostream_play(int i, float volume, int looping)
{
	if (!Audiostream_inited)
		return;

	if ( i == -1 )
		return;

	Assert(looping >= 0);
	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if (volume == -1.0f) {
		volume = Audio_streams[i].Get_Default_Volume();
	}

	Assert(volume >= 0.0f && volume <= 1.0f );
	CAP(volume, 0.0f, 1.0f);

	Assert( Audio_streams[i].status == ASF_USED );
	Audio_streams[i].Set_Default_Volume(volume);
	Audio_streams[i].Play(volume, looping);
}

// use as buffer service function
int audiostream_is_playing(int i)
{
	if ( i == -1 )
		return 0;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status != ASF_USED )
		return 0;

	return (int)Audio_streams[i].Is_Playing();
}

void audiostream_stop(int i, int rewind, int paused)
{
	if (!Audiostream_inited)
		return;

	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );
	Assert( Audio_streams[i].status == ASF_USED );

	if ( rewind )
		Audio_streams[i].Stop_and_Rewind();
	else
		Audio_streams[i].Stop(paused);
}

void audiostream_set_volume_all(float volume, int type)
{
	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE )
			continue;

		if ( (Audio_streams[i].type == type) || ((Audio_streams[i].type == ASF_MENUMUSIC) && (type == ASF_EVENTMUSIC)) ) {
			Audio_streams[i].Set_Volume(volume);
		}
	}
}

void audiostream_set_volume(int i, float volume)
{
	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );
	Assert( volume >= 0.0f && volume <= 1.0f);

	if ( Audio_streams[i].status == ASF_FREE )
		return;

	Audio_streams[i].Set_Volume(volume);
}

int audiostream_is_paused(int i)
{
	if ( i == -1 )
		return 0;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return -1;

	return (int) Audio_streams[i].Is_Paused();
}

int audiostream_get_duration(int i)
{
	if (i == -1)
		return 0;

	Assert(i >= 0 && i < MAX_AUDIO_STREAMS);

	if (Audio_streams[i].status == ASF_FREE)
		return -1;
	
	return fl2i(1000.0f * Audio_streams[i].Get_Duration());
}

void audiostream_set_sample_cutoff(int i, uint cutoff)
{
	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );
	Assert( cutoff > 0 );

	if ( Audio_streams[i].status == ASF_FREE )
		return;

	Audio_streams[i].Set_Sample_Cutoff(cutoff);
}

uint audiostream_get_samples_committed(int i)
{
	if ( i == -1 )
		return 0;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return 0;

	return Audio_streams[i].Get_Samples_Committed();
}

int audiostream_done_reading(int i)
{
	if ( i == -1 )
		return 0;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return 0;

	return Audio_streams[i].Is_Past_Limit();
}

int audiostream_is_inited()
{
	return Audiostream_inited;
}

void audiostream_pause(int i, bool via_sexp_or_script)
{
	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return;

	if ( audiostream_is_playing(i) == (int)true )
		audiostream_stop(i, 0, 1);

	if (via_sexp_or_script)
		Audio_streams[i].paused_via_sexp_or_script = true;
}

void audiostream_unpause(int i, bool via_sexp_or_script)
{
	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return;

	if ( audiostream_is_paused(i) == (int)true ) {
		audiostream_play(i, Audio_streams[i].Get_Volume(), Audio_streams[i].Is_looping());
	}

	if (via_sexp_or_script)
		Audio_streams[i].paused_via_sexp_or_script = false;
}

void audiostream_pause_all(bool via_sexp_or_script)
{
	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE )
			continue;

		audiostream_pause(i, via_sexp_or_script);
	}
}

void audiostream_unpause_all(bool via_sexp_or_script)
{
	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE )
			continue;

		// if we explicitly paused this and we are not explicitly unpausing, skip this stream
		if ( Audio_streams[i].paused_via_sexp_or_script && !via_sexp_or_script )
			continue;

		audiostream_unpause(i, via_sexp_or_script);
	}
}
