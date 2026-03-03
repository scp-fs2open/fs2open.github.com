
#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "io/timer.h"
#include "sound/audiostr.h"
#include "sound/IAudioFile.h"
#include "sound/sound.h"
#include "gamesnd/eventmusic.h"

#ifdef WITH_FFMPEG
#include "sound/ffmpeg/FFmpegWaveFile.h"
#endif


// status
#define ASF_FREE	0
#define ASF_USED	1

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

static int Audiostream_inited = 0;

static SDL_AudioDeviceID Audiostream_device = 0;
static SDL_AudioSpec Audiostream_spec;

class AudioStream
{
public:
	AudioStream ();
	~AudioStream ();
	bool Create (char *pszFilename);
	bool CreateMem (const uint8_t* snddata, size_t snd_len);
	bool Destroy ();
	void Play (float volume, bool looping);
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
	void Do_Frame();
	int	status;
	int	type;
	bool paused_via_sexp_or_script;

protected:
	bool prepareOpened(const char *filename);
	void Cue ();
	bool WriteWaveData(uint* num_bytes_written = nullptr);
	bool PlaybackDone();

	static void SDLCALL ServiceBuffer(void *userdata, SDL_AudioStream *stream,
									  int additional_amount, int total_amount);

	SDL_AudioStream *m_audio_stream;

	std::unique_ptr<sound::IAudioFile> m_pwavefile;	// ptr to WaveFile object
	sound::AudioFileProperties m_fileProps;
	bool m_fCued;			// semaphore (stream cued)
	bool m_fPlaying;		// semaphore (stream playing)
	SCP_vector<uint8_t> m_cbBuffer;	// sound buffer for reading wave data
	uint m_nTimeStarted;	// time (in system time) playback started

	bool	m_bLooping;				// whether or not to loop playback
	bool	m_bFade;				// fade out music 
	bool	m_bDestroy_when_faded;
	float	m_lVolume;				// volume of stream ( 0 -> 1 )
	float	m_lCutoffVolume;
	bool	m_bIsPaused;			// stream is stopped, but not rewinded
	bool	m_bReadingDone;			// no more bytes to be read from disk, still have remaining buffer to play
	int		m_fade_timer_id;		// timestamp so we know when to start fade
	int		m_finished_id;			// timestamp so we know when we've played #bytes required
	bool	m_bPastLimit;			// flag to show we've played past the number of bytes requred
	float	m_lDefaultVolume;

	size_t m_total_uncompressed_bytes_read;
	size_t m_max_uncompressed_bytes_to_read;
};


//
// AudioStream class implementation
//
////////////////////////////////////////////////////////////

// Constructor
AudioStream::AudioStream (void) : m_total_uncompressed_bytes_read(0), m_max_uncompressed_bytes_to_read(0)
{
}

// Destructor
AudioStream::~AudioStream (void)
{
}

void AudioStream::Init_Data ()
{
	m_audio_stream = nullptr;

	m_bLooping = false;
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
	m_nTimeStarted = 0;

	m_cbBuffer.clear();
	m_cbBuffer.shrink_to_fit();

	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = std::numeric_limits<size_t>::max();
}


bool AudioStream::prepareOpened(const char *filename)
{
	m_fileProps = m_pwavefile->getFileProperties();

	size_t buf_size = 0;

	// audio data rate in bytes for 1 second of audio
	buf_size = m_fileProps.bytes_per_sample * m_fileProps.num_channels * m_fileProps.sample_rate;
	// shouldn't need more than 250ms worth at a time
	buf_size /= 4;

	m_cbBuffer.reserve(buf_size);

//	nprintf(("SOUND", "SOUND => Stream buffer created using %d bytes\n", static_cast<int>(m_cbBuffer.capacity())));

	SDL_AudioSpec spec{};

	spec.channels = m_fileProps.num_channels;
	spec.freq = m_fileProps.sample_rate;
	spec.format = SDL_AUDIO_UNKNOWN;

	switch (m_fileProps.bytes_per_sample) {
		case 4:
			spec.format = SDL_AUDIO_F32LE;
			break;
		case 2:
			spec.format = SDL_AUDIO_S16LE;
			break;
		case 1:
			spec.format = SDL_AUDIO_U8;
			break;
		default:
			UNREACHABLE("Invalid audio format!");
			break;
	}

	m_audio_stream = SDL_CreateAudioStream(&spec, &Audiostream_spec);

	if ( !m_audio_stream ) {
		mprintf(("AUDIOSTR => ErrorExit for ::prepareOpened() on wave file: %s\n", filename));

		if (m_pwavefile) {
			m_pwavefile = nullptr;
		}

		return false;
	}

	Snd_sram += m_cbBuffer.capacity();

	return true;
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

	// Stop playback
	Stop ();

	// Release sound buffer
	SDL_DestroyAudioStream(m_audio_stream);
	m_audio_stream = nullptr;

	Snd_sram -= m_cbBuffer.capacity();

	m_cbBuffer.clear();
	m_cbBuffer.shrink_to_fit();

	// Delete WaveFile object
	m_pwavefile = nullptr;

	status = ASF_FREE;

	return fRtn;
}

// WriteWaveData
//
// Writes wave data to sound buffer. This is a helper method used by Create and
// ServiceBuffer; it's not exposed to users of the AudioStream class.
bool AudioStream::WriteWaveData(uint *num_bytes_written)
{
	if (num_bytes_written) {
		*num_bytes_written = 0;
	}

	if (m_bReadingDone) {
		return true;
	}

	if ( !m_audio_stream || !m_pwavefile ) {
		return true;
	}

	int num_bytes_read = 0;

	num_bytes_read = m_pwavefile->Read(m_cbBuffer.data(), m_cbBuffer.capacity());

	// if looping then maybe reset wavefile and keep going
	if ((num_bytes_read < 0) && m_bLooping) {
		m_pwavefile->Cue();
		m_total_uncompressed_bytes_read = 0;
		num_bytes_read = m_pwavefile->Read(m_cbBuffer.data(), m_cbBuffer.capacity());
	}

	if (num_bytes_read < 0) {
		m_bReadingDone = true;
		// we're done adding more data so let SDL know to use all that's left
		SDL_FlushAudioStream(m_audio_stream);
	} else if (num_bytes_read > 0) {
		SDL_PutAudioStreamData(m_audio_stream, m_cbBuffer.data(), num_bytes_read);

		if (num_bytes_written) {
			*num_bytes_written += num_bytes_read;
		}
	}

	return true;
}

void SDLCALL AudioStream::ServiceBuffer(void *userdata, SDL_AudioStream *stream __UNUSED,
										int additional_amount, int total_amount __UNUSED)
{
	// stream doesn't actually need more data, so bail
	if (additional_amount <= 0) {
		return;
	}

	auto info = reinterpret_cast<AudioStream *>(userdata);

	// adjust buffer size if necessary
	if (static_cast<size_t>(additional_amount) > info->m_cbBuffer.capacity()) {
		// NOTE: Snd_ram isn't modified here so we can avoid any possible threading
		//       issues. Also, the chance of a resize being needed is quite small.
		info->m_cbBuffer.reserve(additional_amount);
	}

	// read in additional wave data
	info->WriteWaveData();
}

// Cue
void AudioStream::Cue (void)
{
	if (!m_fCued) {
		m_bFade = false;
		m_fade_timer_id = 0;
		m_finished_id = 0;
		m_bPastLimit = false;
		m_lVolume = 1.0f;
		m_lCutoffVolume = 0.0f;

		m_bDestroy_when_faded = false;

		// Reset file ptr, etc
		m_pwavefile->Cue ();

		// Unqueue all buffers
		SDL_ClearAudioStream(m_audio_stream);

		// Fill buffer with wave data
		WriteWaveData();

		m_fCued = true;

		// Init some of our data
		m_total_uncompressed_bytes_read = 0;
		m_max_uncompressed_bytes_to_read = std::numeric_limits<uint>::max();
	}
}

// Play
void AudioStream::Play (float volume, bool looping)
{
	if ( !m_audio_stream ) {
		return;
	}

	// If playing, stop
	if (m_fPlaying) {
		if ( !m_bIsPaused )
			Stop_and_Rewind();
	}

	// loop flag must be set before Cue()!
	m_bLooping = looping;

	// Cue for playback if necessary
	if ( !m_fCued )
		Cue ();

	m_nTimeStarted = timer_get_milliseconds();
	Set_Volume(volume);

	// once bound it should start playing immediately
	SDL_BindAudioStream(Audiostream_device, m_audio_stream);
	SDL_SetAudioStreamGetCallback(m_audio_stream, ServiceBuffer, this);

	// Playback begun, no longer cued
	m_fPlaying = true;
	m_bIsPaused = false;
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
		SDL_SetAudioStreamGetCallback(m_audio_stream, nullptr, nullptr);
		SDL_UnbindAudioStream(m_audio_stream);

		if ( !paused ) {
			SDL_ClearAudioStream(m_audio_stream);
		}

		m_fPlaying = false;
		m_bIsPaused = (paused != 0);
	}
}

// Stop_and_Rewind
void AudioStream::Stop_and_Rewind (void)
{
	if (m_fPlaying) {
		// Stop playback
		SDL_SetAudioStreamGetCallback(m_audio_stream, nullptr, nullptr);
		SDL_UnbindAudioStream(m_audio_stream);
		SDL_ClearAudioStream(m_audio_stream);

		m_fPlaying = false;
		m_bIsPaused = false;
	}

	m_fCued = false;	// this will cause wave file to start from beginning
	m_bReadingDone = false;
}

// Set_Volume
void AudioStream::Set_Volume(float vol)
{
	CAP(vol, 0.0f, 1.0f);

	SDL_SetAudioStreamGain(m_audio_stream, vol);

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
	if (m_bReadingDone && (SDL_GetAudioStreamQueued(m_audio_stream) < 1))
		return true;
	else
		return false;
}

#define VOLUME_ATTENUATION_BEFORE_CUTOFF			0.03f
#define VOLUME_ATTENUATION							0.65f

// Things that should be looked after at regular intervals (such as every frame)
void AudioStream::Do_Frame()
{
	float vol;

	if (m_bFade) {
		if (m_lCutoffVolume == 0.0f) {
			vol = Get_Volume();
			//			nprintf(("Alan","Volume is: %d\n",vol));
			m_lCutoffVolume = vol * VOLUME_ATTENUATION_BEFORE_CUTOFF;
		}

		vol = Get_Volume() * VOLUME_ATTENUATION;
		//		nprintf(("Alan","Volume is now: %d\n",vol));
		Set_Volume(vol);

		//		nprintf(("Sound","SOUND => Volume for stream sound is %d\n",vol));
		//		nprintf(("Alan","Cuttoff Volume is: %d\n",m_lCutoffVolume));
		if (vol < m_lCutoffVolume) {
			m_bFade = false;
			m_lCutoffVolume = 0.0f;

			if (m_bDestroy_when_faded) {
				Destroy();
			} else {
				Stop_and_Rewind();
			}

			return;
		}
	}

	if (m_total_uncompressed_bytes_read >= m_max_uncompressed_bytes_to_read) {
		m_fade_timer_id = timer_get_milliseconds() + 1700;		// start fading 1.7 seconds from now
		m_finished_id = timer_get_milliseconds() + 2000;		// 2 seconds left to play out buffer
		m_max_uncompressed_bytes_to_read = std::numeric_limits<uint>::max();;
	}

	if ( (m_fade_timer_id > 0) && (timer_get_milliseconds() > m_fade_timer_id) ) {
		m_fade_timer_id = 0;
		Fade_and_Stop();
	}

	if ( (m_finished_id > 0) && (timer_get_milliseconds() > m_finished_id) ) {
		m_finished_id = 0;
		m_bPastLimit = true;
	}

	// see if we're done
	if ( m_bReadingDone && (SDL_GetAudioStreamQueued(m_audio_stream) < 1) ) {
		if (m_bDestroy_when_faded) {
			// All of sound has played, and we're done with it
			Destroy();
		} else if (m_bLooping && !m_bFade) {
			// All of sound has played, loop again
			Play(m_lVolume, m_bLooping);
		} else {
			// All of sound has played, stop playback
			Stop_and_Rewind();
		}
	}
}


static AudioStream Audio_streams[MAX_AUDIO_STREAMS];


void audiostream_init()
{
	int i;

	if ( Audiostream_inited == 1 )
		return;

	// request a format that matches the best possible quality we'll play
	Audiostream_spec.freq = 48000;
	Audiostream_spec.channels = 2;
	Audiostream_spec.format = SDL_AUDIO_F32LE;

	Audiostream_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
											 &Audiostream_spec);

	if ( !Audiostream_device ) {
		return;
	}

	// now get actual format
	SDL_GetAudioDeviceFormat(Audiostream_device, &Audiostream_spec, nullptr);

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		Audio_streams[i].Init_Data();
		Audio_streams[i].status = ASF_FREE;
		Audio_streams[i].type = ASF_NONE;
		Audio_streams[i].paused_via_sexp_or_script = false;
	}

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

	if (Audiostream_device) {
		SDL_CloseAudioDevice(Audiostream_device);
		Audiostream_device = 0;
	}

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
	Audio_streams[i].Play(volume, looping != 0);
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

double audiostream_get_duration(int i)
{
	if (i == -1)
		return 0;

	Assert(i >= 0 && i < MAX_AUDIO_STREAMS);

	if (Audio_streams[i].status == ASF_FREE)
		return -1;
	
	return Audio_streams[i].Get_Duration();
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

void audiostream_do_frame()
{
	for (auto &item : Audio_streams) {
		if (item.status == ASF_FREE)
			continue;

		item.Do_Frame();
	}
}
