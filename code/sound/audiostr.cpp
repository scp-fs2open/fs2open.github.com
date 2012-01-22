

#ifdef _WIN32
#define VC_EXTRALEAN
#define STRICT

#include <windows.h>
#include <mmsystem.h>
#endif

#define NEED_STRHDL		// for STRHTL struct in audiostr.h

#include "globalincs/pstypes.h"
#include "sound/openal.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/acm.h"
#include "cfile/cfile.h"
#include "sound/sound.h"
#include "sound/ogg/ogg.h"
#include "io/timer.h"

#define THREADED
#include "osapi/osapi.h"


#define MAX_STREAM_BUFFERS 4

// status
#define ASF_FREE	0
#define ASF_USED	1

// constants
#define BIGBUF_SIZE					176400
ubyte *Wavedata_load_buffer = NULL;		// buffer used for cueing audiostreams
ubyte *Wavedata_service_buffer = NULL;	// buffer used for servicing audiostreams

CRITICAL_SECTION Global_service_lock;

typedef bool (*TIMERCALLBACK)(ptr_u);

#define COMPRESSED_BUFFER_SIZE	176400
ubyte *Compressed_buffer = NULL;				// Used to load in compressed data during a cueing interval
ubyte *Compressed_service_buffer = NULL;	// Used to read in compressed data during a service interval

#define AS_HIGHEST_MAX	999999999	// max uncompressed filesize supported is 999 meg


int Audiostream_inited = 0;


static int dbg_print_ogg_error(const char *filename, int rc)
{
	int fatal = 0;
	char err_msg[100];
	memset( &err_msg, 0, sizeof(err_msg) );

	Assert( filename != NULL );

	switch (rc) {
		case OV_FALSE:
			strncpy(err_msg, "A false status was returned", 99);
			// should this be fatal?
			break;
		case OV_EOF:
			strncpy(err_msg, "End-of-file reached", 99);
			fatal = 1;
			break;
		case OV_HOLE:
			strncpy(err_msg, "Data interruption (hole)", 99);
			// special handling
			break;
		case OV_EREAD:
			strncpy(err_msg, "Media read error", 99);
			fatal = 1;
			break;
		case OV_EFAULT:
			strncpy(err_msg, "Internal logic fault", 99);
			fatal = 1;
			break;
		case OV_EIMPL:
			strncpy(err_msg, "Attempted to use a feature that's not supported", 99);
			fatal = 1;
			break;
		case OV_EINVAL:
			strncpy(err_msg, "Invalid argument value", 99);
			// doesn't appear to be fatal
			break;
		case OV_ENOTVORBIS:
			strncpy(err_msg, "File contains non-Vorbis data, or is not a Vorbis file", 99);
			fatal = 1;
			break;
		case OV_EBADHEADER:
			strncpy(err_msg, "Invalid bitstream header", 99);
			fatal = 1;
			break;
		case OV_EVERSION:
			strncpy(err_msg, "Vorbis version mismatch", 99);
			fatal = 1;
			break;
		case OV_ENOTAUDIO:
			strncpy(err_msg, "Submitted data is not audio", 99);
			fatal = 1;
			break;
		case OV_EBADPACKET:
			strncpy(err_msg, "An invalid packet was submitted", 99);
			// is this fatal?
			break;
		case OV_EBADLINK:
			strncpy(err_msg, "Invalid stream section supplied, or corrupt link", 99);
			fatal = 1; // is this really fatal or does the lib compensate?
			break;
		case OV_ENOSEEK:
			strncpy(err_msg, "Bitstream is not seekable", 99);
			fatal = 1;
			break;
		default:
			strncpy(err_msg, "Unknown error occurred", 99);
			fatal = 1; // assume fatal
			break;
	}

	// only dump fatal errors, everything else should be handled silently by default
	if (fatal)
		mprintf(("OGG ERROR: \"%s\" in %s\n", err_msg, filename));
//	else
//		nprintf(("OGGISH", "OGG ERROR: \"%s\" in %s\n", err_msg, filename));

	return fatal;
}

static int audiostr_read_uint(HMMIO rw, uint *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(uint) );

	if (rc != sizeof(uint))
		return 0;

	*i = INTEL_INT(*i); //-V570

	return 1;
}

static int audiostr_read_word(HMMIO rw, WORD *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(WORD) );

	if (rc != sizeof(WORD))
		return 0;

	*i = INTEL_SHORT(*i); //-V570

	return 1;
}

static int audiostr_read_dword(HMMIO rw, DWORD *i)
{
	int rc = mmioRead( rw, (char *)i, sizeof(DWORD) );

	if (rc != sizeof(DWORD))
		return 0;

	*i = INTEL_INT(*i); //-V570

	return 1;
}

class Timer
{
public:
    void constructor(void);
    void destructor(void);
    bool Create (uint nPeriod, uint nRes, ptr_u dwUser, TIMERCALLBACK pfnCallback);
protected:
#ifndef SCP_UNIX
    static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#else
    static uint TimeProc(uint interval, void *param);
#endif
    TIMERCALLBACK m_pfnCallback;
    ptr_u m_dwUser;
    uint m_nPeriod;
    uint m_nRes;
#ifndef SCP_UNIX
    uint m_nIDTimer;
#else
    SDL_TimerID m_nIDTimer;
#endif
};

class WaveFile
{
public:
	void Init(void);
	void Close(void);
	bool Open (char *pszFilename, bool keep_ext = true);
	bool Cue (void);
	int	Read (ubyte *pbDest, uint cbSize, int service=1);
	uint GetNumBytesRemaining (void) { return (m_nDataSize - m_nBytesPlayed); }
	uint GetUncompressedAvgDataRate (void) { return (m_nUncompressedAvgDataRate); }
	uint GetDataSize (void) { return (m_nDataSize); }
	uint GetNumBytesPlayed (void) { return (m_nBytesPlayed); }
	ALenum GetALFormat() { return (m_al_format); }
	WAVEFORMATEX m_wfmt;					// format of wave file used by Direct Sound
	WAVEFORMATEX *m_pwfmt_original;	// foramt of wave file from actual wave source
	uint m_total_uncompressed_bytes_read;
	uint m_max_uncompressed_bytes_to_read;
	uint m_bits_per_sample_uncompressed;

protected:
	uint m_data_offset;						// number of bytes to actual wave data
	int  m_data_bytes_left;

	uint m_wave_format;						// format of wave source (ie WAVE_FORMAT_PCM, WAVE_FORMAT_ADPCM)
	uint m_nBlockAlign;						// wave data block alignment spec
	uint m_nUncompressedAvgDataRate;		// average wave data rate
	uint m_nDataSize;							// size of data chunk
	uint m_nBytesPlayed;						// offset into data chunk
	bool m_abort_next_read;
	ALenum m_al_format;

	STRHDL m_snd_info;

	void			*m_hStream;
	int				m_hStream_open;
	WAVEFORMATEX	m_wfxDest;
	char			m_wFilename[MAX_FILENAME_LEN];
};

class AudioStream
{
public:
	AudioStream (void);
	~AudioStream (void);
	bool Create (char *pszFilename);
	bool Destroy (void);
	void Play (float volume, int looping);
	bool Is_Playing(){ return m_fPlaying; }
	bool Is_Paused(){ return m_bIsPaused; }
	bool Is_Past_Limit() { return m_bPastLimit; }
	void Stop (int paused = 0);
	void Stop_and_Rewind (void);
	void Fade_and_Destroy (void);
	void Fade_and_Stop(void);
	void	Set_Volume(float vol);
	float	Get_Volume();
	void	Init_Data();
	void	Set_Sample_Cutoff(uint sample_cutoff);
	void	Set_Default_Volume(float vol) { m_lDefaultVolume = vol; }
	float	Get_Default_Volume() { return m_lDefaultVolume; }
	uint	Get_Samples_Committed(void);
	int	Is_looping() { return m_bLooping; }
	int	status;
	int	type;
	ushort m_bits_per_sample_uncompressed;

protected:
	void Cue (void);
	bool WriteWaveData (uint cbSize, uint *num_bytes_written, int service = 1);
	uint GetMaxWriteSize (void);
	bool ServiceBuffer (void);
	static bool TimerCallback (ptr_u dwUser);

	ALuint m_source_id;	// name of openAL source
	ALuint m_buffer_ids[MAX_STREAM_BUFFERS];	// names of buffers

	Timer m_timer;			// ptr to Timer object
	WaveFile *m_pwavefile;	// ptr to WaveFile object
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

	CRITICAL_SECTION write_lock;
};


// Timer class implementation
//
////////////////////////////////////////////////////////////

// constructor
void Timer::constructor(void)
{
	m_nIDTimer = NULL;
}


// Destructor
void Timer::destructor(void)
{
	if (m_nIDTimer) {
#ifndef SCP_UNIX
		timeKillEvent (m_nIDTimer);
#else
		SDL_RemoveTimer(m_nIDTimer);
#endif
		m_nIDTimer = NULL;
	}
}

// Create
bool Timer::Create (uint nPeriod, uint nRes, ptr_u dwUser, TIMERCALLBACK pfnCallback)
{
	bool bRtn = true;    // assume success

	Assert(pfnCallback);
	Assert(nPeriod > 10);
	Assert(nPeriod >= nRes);

	m_nPeriod = nPeriod;
	m_nRes = nRes;
	m_dwUser = dwUser;
	m_pfnCallback = pfnCallback;

#ifndef SCP_UNIX
	if ((m_nIDTimer = timeSetEvent ((UINT)m_nPeriod, (UINT)m_nRes, TimeProc, (DWORD)this, TIME_PERIODIC)) == NULL) {
#else
	if ((m_nIDTimer = SDL_AddTimer(m_nPeriod, TimeProc, (void*)this)) == NULL) {
#endif
	  bRtn = false;
	}

	return (bRtn);
}


// Timer proc for multimedia timer callback set with timeSetTime().
//
// Calls procedure specified when Timer object was created. The 
// dwUser parameter contains "this" pointer for associated Timer object.
// 
#ifndef SCP_UNIX
void CALLBACK Timer::TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
#else
uint Timer::TimeProc(uint interval, void *dwUser)
#endif
{
    // dwUser contains ptr to Timer object
	Timer * ptimer = (Timer *) dwUser;

    // Call user-specified callback and pass back user specified data
    (ptimer->m_pfnCallback) (ptimer->m_dwUser);

#ifdef SCP_UNIX
    if (ptimer->m_nPeriod) {
		return interval;
    } else {
		SDL_RemoveTimer(ptimer->m_nIDTimer);
		ptimer->m_nIDTimer = NULL;
		return 0;
    }
#endif
}


// WaveFile class implementation
//
////////////////////////////////////////////////////////////

// Constructor
void WaveFile::Init(void)
{
	// Init data members
	m_data_offset = 0;
	m_snd_info.cfp = NULL;
	m_snd_info.true_offset = 0;
	m_snd_info.size = 0;
	m_pwfmt_original = NULL;
	m_nBlockAlign= 0;
	m_nUncompressedAvgDataRate = 0;
	m_nDataSize = 0;
	m_nBytesPlayed = 0;
	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;
	m_al_format = AL_FORMAT_MONO8;

	memset(&m_wFilename, 0, MAX_FILENAME_LEN);

	m_hStream_open = 0;
	m_abort_next_read = false;
}

// Destructor
void WaveFile::Close(void)
{
	// Free memory
	if (m_pwfmt_original) {
		vm_free(m_pwfmt_original);
		m_pwfmt_original = NULL;
	}

	if ( m_hStream_open ) {
		ACM_stream_close((void*)m_hStream);
		m_hStream_open = 0;
	}

	// Close file
	if (m_snd_info.cfp) {
		if (m_wave_format == OGG_FORMAT_VORBIS)
			ov_clear(&m_snd_info.vorbis_file);

		mmioClose( m_snd_info.cfp, 0 );
		m_snd_info.cfp = NULL;
		m_snd_info.true_offset = 0;
		m_snd_info.size = 0;
	}
}

// -- from parselo.cpp --
extern char *stristr(const char *str, const char *substr);

// Open
bool WaveFile::Open(char *pszFilename, bool keep_ext)
{
	int rc = -1;
	WORD cbExtra = 0;
	bool fRtn = true;    // assume success
	PCMWAVEFORMAT pcmwf;
	int FileSize, FileOffset;
	char fullpath[MAX_PATH];
	char filename[MAX_FILENAME_LEN];
	const int NUM_EXT = 2;
	const char *audio_ext[NUM_EXT] = { ".ogg", ".wav" };

	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;

	// NOTE: we assume that the extension has already been stripped off if it was supposed to be!!
	strcpy_s( filename, pszFilename );


	// if we are supposed to load the file as passed...
	if (keep_ext) {
		for (int i = 0; i < NUM_EXT; i++) {
			if ( stristr(pszFilename, audio_ext[i]) ) {
				rc = i;
				break;
			}
		}

		// not a supported extension format ... somebody screwed up their tbls :)
		if (rc < 0)
			goto OPEN_ERROR;

		cf_find_file_location(pszFilename, CF_TYPE_ANY, sizeof(fullpath) - 1, fullpath, &FileSize, &FileOffset);
	}
	// ... otherwise we just find the best match
	else {
		rc = cf_find_file_location_ext(filename, NUM_EXT, audio_ext, CF_TYPE_ANY, sizeof(fullpath) - 1, fullpath, &FileSize, &FileOffset);
	}

	if (rc < 0) {
		goto OPEN_ERROR;
	} else {
		// set proper filename for later use (assumes that it doesn't already have an extension)
		strcat_s( filename, audio_ext[rc] );
	}

	m_snd_info.cfp = mmioOpen( fullpath, NULL, MMIO_ALLOCBUF | MMIO_READ );

	if (m_snd_info.cfp == NULL)
		goto OPEN_ERROR;

	m_snd_info.true_offset = FileOffset;
	m_snd_info.size = FileSize;

	// if in a VP then position the stream at the start of the file
	if (FileOffset > 0)
		mmioSeek( m_snd_info.cfp, FileOffset, SEEK_SET );

	// if Ogg Vorbis...
	if (rc == 0) {
		if ( ov_open_callbacks(&m_snd_info, &m_snd_info.vorbis_file, NULL, 0, mmio_callbacks) == 0 ) {
			// got an Ogg Vorbis, so lets read the info in
			ov_info(&m_snd_info.vorbis_file, -1);

			// we only support one logical bitstream
			if ( ov_streams(&m_snd_info.vorbis_file) != 1 ) {
				mprintf(("AUDIOSTR => OGG reading error:  We don't handle bitstream changes!\n"));
				goto OPEN_ERROR;
			}

			m_wave_format = OGG_FORMAT_VORBIS;
			m_wfmt.wFormatTag = WAVE_FORMAT_PCM;
			m_wfmt.nChannels = (WORD) m_snd_info.vorbis_file.vi->channels;
			m_wfmt.nSamplesPerSec = m_snd_info.vorbis_file.vi->rate;

			switch (Ds_sound_quality) {
				case DS_SQ_HIGH:
					m_wfmt.wBitsPerSample = Ds_float_supported ? 32 : 16;
					break;

				case DS_SQ_MEDIUM:
					m_wfmt.wBitsPerSample = 16;
					break;

				default:
					m_wfmt.wBitsPerSample = 8;
					break;
			}

			m_wfmt.cbSize = 0;

			m_wfmt.nBlockAlign = (ushort)(( m_wfmt.nChannels * m_wfmt.wBitsPerSample ) / 8);
			m_wfmt.nAvgBytesPerSec = m_wfmt.nSamplesPerSec * m_wfmt.nBlockAlign;

			m_nBlockAlign = m_wfmt.nBlockAlign;
			m_nUncompressedAvgDataRate = m_wfmt.nAvgBytesPerSec;

			// location of start of file in VP
			m_data_offset = 0;
			m_nDataSize = m_data_bytes_left = ((int)ov_pcm_total(&m_snd_info.vorbis_file, -1) * m_wfmt.nBlockAlign);
		} else {
			mprintf(("AUDIOSTR => OGG reading error: Not a valid Vorbis file!\n"));
		}
	}
	// if Wave...
	else if (rc == 1) {
		bool done = false;

		// Skip the "RIFF" tag and file size (8 bytes)
		// Skip the "WAVE" tag (4 bytes)
		mmioSeek( m_snd_info.cfp, 12+FileOffset, SEEK_SET );

		// Now read RIFF tags until the end of file
		uint tag, size, next_chunk;

		while ( !done ) {
			if ( !audiostr_read_uint(m_snd_info.cfp, &tag) )
				break;

			if ( !audiostr_read_uint(m_snd_info.cfp, &size) )
				break;

			next_chunk = mmioSeek(m_snd_info.cfp, 0, SEEK_CUR );
			next_chunk += size;

			switch (tag)
			{
				case 0x20746d66:		// The 'fmt ' tag
				{
					audiostr_read_word(m_snd_info.cfp, &pcmwf.wf.wFormatTag);
					audiostr_read_word(m_snd_info.cfp, &pcmwf.wf.nChannels);
					audiostr_read_dword(m_snd_info.cfp, &pcmwf.wf.nSamplesPerSec);
					audiostr_read_dword(m_snd_info.cfp, &pcmwf.wf.nAvgBytesPerSec);
					audiostr_read_word(m_snd_info.cfp, &pcmwf.wf.nBlockAlign);
					audiostr_read_word(m_snd_info.cfp, &pcmwf.wBitsPerSample);
			
					if (pcmwf.wf.wFormatTag == WAVE_FORMAT_ADPCM)
						audiostr_read_word(m_snd_info.cfp, &cbExtra);

					// Allocate memory for WAVEFORMATEX structure + extra bytes
					if ( (m_pwfmt_original = (WAVEFORMATEX *) vm_malloc(sizeof(WAVEFORMATEX)+cbExtra)) != NULL ) {
						Assert(m_pwfmt_original != NULL);
						// Copy bytes from temporary format structure
						memcpy (m_pwfmt_original, &pcmwf, sizeof(pcmwf));
						m_pwfmt_original->cbSize = cbExtra;

						// Read those extra bytes, append to WAVEFORMATEX structure
						if (cbExtra != 0)
							mmioRead( m_snd_info.cfp, ((char *)(m_pwfmt_original) + sizeof(WAVEFORMATEX)), cbExtra );
					} else {
						Int3();		// malloc failed
						goto OPEN_ERROR;
					}

					break;
				}

				case 0x61746164:		// the 'data' tag
				{
					m_nDataSize = size;	// This is size of data chunk.  Compressed if ADPCM.
					m_data_bytes_left = size;
					m_data_offset = mmioSeek( m_snd_info.cfp, 0, SEEK_CUR );
					done = true;

					break;
				}

				default:	// unknown, skip it
					break;
			}	// end switch

			mmioSeek( m_snd_info.cfp, next_chunk, SEEK_SET );
		}

		// make sure that we did good
		if ( !done || (m_pwfmt_original == NULL) )
			goto OPEN_ERROR;

  		// At this stage, examine source format, and set up WAVEFORATEX structure for DirectSound.
		// Since DirectSound only supports PCM, force this structure to be PCM compliant.  We will
		// need to convert data on the fly later if our souce is not PCM
		switch (m_pwfmt_original->wFormatTag) {
			case WAVE_FORMAT_PCM:
				m_wave_format = WAVE_FORMAT_PCM;
				m_wfmt.wBitsPerSample = m_pwfmt_original->wBitsPerSample;
				break;

			case WAVE_FORMAT_ADPCM:
				m_wave_format = WAVE_FORMAT_ADPCM;
				m_wfmt.wBitsPerSample = (Ds_sound_quality) ? 16: 8;
				m_bits_per_sample_uncompressed = m_wfmt.wBitsPerSample;
				break;

			case WAVE_FORMAT_IEEE_FLOAT: {
				m_wave_format = WAVE_FORMAT_IEEE_FLOAT;

				switch (Ds_sound_quality) {
					case DS_SQ_HIGH:
						m_wfmt.wBitsPerSample = (Ds_float_supported) ? 32 : 16;
						break;

					case DS_SQ_MEDIUM:
						m_wfmt.wBitsPerSample = 16;
						break;

					default:
						m_wfmt.wBitsPerSample = 8;
						break;
				}

				break;
			}

			default:
				nprintf(("SOUND", "SOUND => Not supporting %d format for playing wave files\n", m_pwfmt_original->wFormatTag));
				//Int3();
				goto OPEN_ERROR;
				break;

		} // end switch
            
		// Set up the WAVEFORMATEX structure to have the right PCM characteristics
		m_wfmt.wFormatTag = WAVE_FORMAT_PCM;
		m_wfmt.nChannels = m_pwfmt_original->nChannels;
		m_wfmt.nSamplesPerSec = m_pwfmt_original->nSamplesPerSec;
		m_wfmt.cbSize = 0;
		m_wfmt.nBlockAlign = (ushort)(( m_wfmt.nChannels * m_wfmt.wBitsPerSample ) / 8);
		m_wfmt.nAvgBytesPerSec = m_wfmt.nBlockAlign * m_wfmt.nSamplesPerSec;

		// Init some member data from format chunk
		m_nBlockAlign = m_pwfmt_original->nBlockAlign;
		m_nUncompressedAvgDataRate = m_wfmt.nAvgBytesPerSec;

		Assert( (m_wfmt.nChannels == 1) || (m_wfmt.nChannels == 2) );
	}
	// something unkown???
	else {
		Int3();
	}

	m_al_format = openal_get_format(m_wfmt.wBitsPerSample, m_wfmt.nChannels);

	if (m_al_format != AL_INVALID_VALUE) {	
		// Cue for streaming
		Cue();

		goto OPEN_DONE;
	}

OPEN_ERROR:
	// Handle all errors here
	nprintf(("SOUND","SOUND ==> Could not open wave file %s for streaming\n", filename));

	fRtn = false;

	if (m_snd_info.cfp != NULL) {
		// Close file
		mmioClose( m_snd_info.cfp, 0 );
		m_snd_info.cfp = NULL;
		m_snd_info.true_offset = 0;
		m_snd_info.size = 0;
	}

	if (m_pwfmt_original) {
		vm_free(m_pwfmt_original);
		m_pwfmt_original = NULL;
	}

OPEN_DONE:
	strncpy(m_wFilename, filename, MAX_FILENAME_LEN-1);

	if (fRtn)
		nprintf(("SOUND", "AUDIOSTR => Successfully opened: %s\n", filename));

	return (fRtn);
}


// Cue
//
// Set the file pointer to the start of wave data
//
bool WaveFile::Cue (void)
{
	bool fRtn = true;    // assume success
	int rval;

	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;

	if (m_wave_format == OGG_FORMAT_VORBIS) {
		rval = (int)ov_raw_seek(&m_snd_info.vorbis_file, m_data_offset);
	} else {
		rval = mmioSeek( m_snd_info.cfp, m_data_offset, SEEK_SET );
	}

	if ( rval == -1 ) {
		fRtn = false;
	}

	m_data_bytes_left = m_nDataSize;
	m_abort_next_read = false;

	return fRtn;
}


// Read
//
// Returns number of bytes actually read.
// 
//	Returns -1 if there is nothing more to be read.  This function can return 0, since
// sometimes the amount of bytes requested is too small for the ACM decompression to 
// locate a suitable block
int WaveFile::Read(ubyte *pbDest, uint cbSize, int service)
{
	void	*dest_buf=NULL, *uncompressed_wave_data;
	int				rc, uncompressed_bytes_written, section, last_section = -1, byte_order = 0;
	uint	src_bytes_used, convert_len, num_bytes_desired=0, num_bytes_read;

//	nprintf(("Alan","Reqeusted: %d\n", cbSize));

#if BYTE_ORDER == BIG_ENDIAN
	byte_order = 1;
#endif

	if ( service ) {
		uncompressed_wave_data = Wavedata_service_buffer;
	} else {
		uncompressed_wave_data = Wavedata_load_buffer;
	}

	switch ( m_wave_format ) {
		case WAVE_FORMAT_PCM:
			num_bytes_desired = cbSize;
			dest_buf = pbDest;
			break;

		case WAVE_FORMAT_ADPCM: {
			if ( !m_hStream_open ) {
				if ( !ACM_stream_open(m_pwfmt_original, &m_wfxDest, (void**)&m_hStream, m_bits_per_sample_uncompressed)  ) {
					m_hStream_open = 1;
				} else {
					Int3();
				}
			}

			num_bytes_desired = cbSize;
	
			if ( service ) {
				dest_buf = Compressed_service_buffer;
			} else {
				dest_buf = Compressed_buffer;
			}

			if ( num_bytes_desired <= 0 ) {
				num_bytes_desired = 0;
//				nprintf(("Alan","No bytes required for ADPCM time interval\n"));
			} else {
				num_bytes_desired = ACM_query_source_size((void*)m_hStream, cbSize);
//				nprintf(("Alan","Num bytes desired: %d\n", num_bytes_desired));
			}

			break;
		}

		case OGG_FORMAT_VORBIS:
			num_bytes_desired = cbSize;
			dest_buf = pbDest;
			break;

		case WAVE_FORMAT_IEEE_FLOAT: {
			num_bytes_desired = cbSize;

			if (m_wfmt.wBitsPerSample == 32) {
				dest_buf = pbDest;
			} else {
				if (service) {
					dest_buf = Compressed_service_buffer;
				} else {
					dest_buf = Compressed_buffer;
				}
			}

			break;
		}

		default:
			nprintf(("SOUND", "SOUND => Not supporting %d format for playing wave files\n", m_wave_format));
			Int3();
			break;

	} // end switch

	num_bytes_read = 0;
	convert_len = 0;
	src_bytes_used = 0;

	// read data from disk
	if ( m_data_bytes_left <= 0 ) {
		num_bytes_read = 0;
		uncompressed_bytes_written = 0;
		return -1;
	}

	if ( (m_data_bytes_left > 0) && (num_bytes_desired > 0) ) {
		int actual_read = 0;

		if ( num_bytes_desired <= (uint)m_data_bytes_left ) {
			num_bytes_read = num_bytes_desired;
		}
		else {
			num_bytes_read = m_data_bytes_left;
		}

		// OGG reading is special
		if ( m_wave_format == OGG_FORMAT_VORBIS ) {
			int sign = (m_wfmt.wBitsPerSample == 8) ? 0 : 1;
			int sample_size = sizeof(float) * m_wfmt.nChannels;

			while ( !m_abort_next_read && ((uint)actual_read < num_bytes_read)) {
				float **pcm = NULL;

				if (m_wfmt.wBitsPerSample == 32) {
					rc = ov_read_float(&m_snd_info.vorbis_file, &pcm, (num_bytes_read - actual_read) / sample_size, &section);
				} else {
					rc = ov_read(&m_snd_info.vorbis_file, (char *)dest_buf + actual_read, num_bytes_read - actual_read, byte_order, m_wfmt.wBitsPerSample / 8, sign, &section);
				}

				// fail if the bitstream changes, shouldn't get this far if that's the case though
				if ((last_section != -1) && (last_section != section)) {
					mprintf(("AUDIOSTR => OGG reading error:  We don't handle bitstream changes!\n"));
					goto READ_ERROR;
				}

				if ( rc > 0 ) {
					if (m_wfmt.wBitsPerSample == 32) {
						float *out_p = (float*)((ubyte*)dest_buf + actual_read);

						for (int i = 0; i < rc; i++) {
							for (int j = 0; j < m_wfmt.nChannels; j++) {
								*out_p++ = pcm[j][i];
							}
						}

						actual_read += (rc * m_wfmt.nBlockAlign);
					} else {
						actual_read += rc;
					}

					last_section = section;
				} else if ( rc == 0 ) {
					break;
				} else if ( rc < 0 ) {
					if ( dbg_print_ogg_error(m_wFilename, rc) ) {
						// must be a fatal error
						goto READ_ERROR;
					} else {
						// not fatal, just continue on
						break;
					}
				}
			}
		}
		// IEEE FLOAT is special too, downsampling can give short buffers
		else if (m_wave_format == WAVE_FORMAT_IEEE_FLOAT) {
			while ( !m_abort_next_read && ((uint)actual_read < num_bytes_read) ) {
				rc = mmioRead(m_snd_info.cfp, (char *)dest_buf, num_bytes_read);

				if (rc <= 0) {
					break;
				}

#if BYTE_ORDER == BIG_ENDIAN
				// need to byte-swap before any later conversions
				float *swap_tmp;

				for (int i = 0; i < rc; i += sizeof(float)) {
					swap_tmp = (float *)((ubyte*)dest_buf + i);
					*swap_tmp = INTEL_FLOAT(swap_tmp);
				}
#endif

				if (m_wfmt.wBitsPerSample == 32) {
					actual_read = rc;
				} else if (m_wfmt.wBitsPerSample == 16) {
					float *in_p = (float*)dest_buf;
					short *out_p = (short*)((ubyte*)uncompressed_wave_data + actual_read);

					int end = rc / sizeof(float);

					for (int i = 0; i < end; i++) {
						int i_val = (int)(in_p[i] * 32767.0f + 0.5f);
						CLAMP(i_val, -32768, 32767);

						*out_p++ = (short)i_val;
					}

					actual_read += (rc >> 1);
				} else {
					Assert( m_wfmt.wBitsPerSample == 8 );

					float *in_p = (float*)dest_buf;
					ubyte *out_p = (ubyte*)((ubyte*)uncompressed_wave_data + actual_read);

					int end = num_bytes_read / sizeof(float);

					for (int i = 0; i < end; i++) {
						int i_val = (int)(in_p[i] * 127.0f + 0.5f) + 128;
						CLAMP(i_val, 0, 255);

						*out_p++ = (ubyte)i_val;
					}

					actual_read += (rc >> 2);
				}
			}
		}
		// standard WAVE reading
		else {
			actual_read = mmioRead( m_snd_info.cfp, (char *)dest_buf, num_bytes_read );
		}

		if ( (actual_read <= 0) || (m_abort_next_read) ) {
			num_bytes_read = 0;
			uncompressed_bytes_written = 0;
			return -1;
		}

		if ( num_bytes_desired >= (uint)m_data_bytes_left ) {
			m_abort_next_read = 1;			
		}

		num_bytes_read = actual_read;
	}

	// convert data if necessary, to PCM
	if ( m_wave_format == WAVE_FORMAT_ADPCM ) {
		if ( num_bytes_read > 0 ) {
			rc = ACM_convert((void*)m_hStream, (ubyte*)dest_buf, num_bytes_read, (ubyte*)uncompressed_wave_data, BIGBUF_SIZE, &convert_len, &src_bytes_used);

			if ( rc == -1 ) {
				goto READ_ERROR;
			} else if ( convert_len == 0 ) {
				if (num_bytes_read < m_nBlockAlign) {
					mprintf(("AUDIOSTR => Warning: Short read detected in ACM decode of '%s'!!\n", m_wFilename));
				} else {
					Int3();
				}
			}
		}

		Assert(src_bytes_used <= num_bytes_read);
		if ( src_bytes_used < num_bytes_read ) {
			// seek back file pointer to reposition before unused source data
			mmioSeek( m_snd_info.cfp, src_bytes_used - num_bytes_read, SEEK_CUR );
		}

		// Adjust number of bytes left
		m_data_bytes_left -= src_bytes_used;
		m_nBytesPlayed += src_bytes_used;
		uncompressed_bytes_written = convert_len;

		// Successful read, keep running total of number of data bytes read
		goto READ_DONE;
	}
	else {
		// Successful read, keep running total of number of data bytes read
		// Adjust number of bytes left
		m_data_bytes_left -= num_bytes_read;
		m_nBytesPlayed += num_bytes_read;
		uncompressed_bytes_written = num_bytes_read;

#if BYTE_ORDER == BIG_ENDIAN
		if ( m_wave_format == WAVE_FORMAT_PCM ) {
			// swap 16-bit sound data
			if (m_wfmt.wBitsPerSample == 16) {
				ushort *swap_tmp;

				for (uint i=0; i<uncompressed_bytes_written; i=i+2) {
					swap_tmp = (ushort*)((ubyte*)dest_buf + i);
					*swap_tmp = INTEL_SHORT(*swap_tmp);
				}
			}
		}
#endif

		goto READ_DONE;
	}
    
READ_ERROR:
	num_bytes_read = 0;
	uncompressed_bytes_written = 0;

READ_DONE:
	m_total_uncompressed_bytes_read += uncompressed_bytes_written;
//	nprintf(("Alan","Read: %d\n", uncompressed_bytes_written));
	return (uncompressed_bytes_written);
}

//
// AudioStream class implementation
//
////////////////////////////////////////////////////////////

// The following constants are the defaults for our streaming buffer operation.
const ushort DefBufferServiceInterval = 250;  // default buffer service interval in msec

// Constructor
AudioStream::AudioStream (void)
{
	INITIALIZE_CRITICAL_SECTION( write_lock );
}

// Destructor
AudioStream::~AudioStream (void)
{
	DELETE_CRITICAL_SECTION( write_lock );
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

	m_pwavefile = NULL;
	m_fPlaying = m_fCued = false;
	m_cbBufOffset = 0;
	m_cbBufSize = 0;
	m_nBufService = DefBufferServiceInterval;
	m_nTimeStarted = 0;

	memset(m_buffer_ids, 0, sizeof(m_buffer_ids));
	m_source_id = 0;
}

// Create
bool AudioStream::Create (char *pszFilename)
{
	bool fRtn = true;    // assume success

	Assert(pszFilename);

	Init_Data();

	if (pszFilename) {
		// make 100% sure we got a good filename
		if ( !strlen(pszFilename) )
			return false;

		// Create a new WaveFile object
		m_pwavefile = (WaveFile *)vm_malloc(sizeof(WaveFile));
		Assert(m_pwavefile);

		if (m_pwavefile) {
			// Call constructor
			m_pwavefile->Init();
			// Open given file
			m_pwavefile->m_bits_per_sample_uncompressed = m_bits_per_sample_uncompressed;

			if ( m_pwavefile->Open(pszFilename, (type == ASF_EVENTMUSIC)) ) {
				m_cbBufSize = m_pwavefile->m_wfmt.nAvgBytesPerSec >> 2;
				// make sure that we are a multiple of the frame size
				m_cbBufSize -= (m_cbBufSize % m_pwavefile->m_wfmt.nBlockAlign);
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

				// Cue for playback
				Cue();
				Snd_sram += (m_cbBufSize * MAX_STREAM_BUFFERS);
			}
			else {
				// Error opening file
				nprintf(("SOUND", "SOUND => Failed to open wave file: %s\n\r", pszFilename));
				fRtn = false;
			}
		}
		else {
			// Error, unable to create WaveFile object
			nprintf(("Sound", "SOUND => Failed to create WaveFile object %s\n\r", pszFilename));
			fRtn = false;
		}
	}
	else {
		// Error, passed invalid parms
		fRtn = false;
	}

ErrorExit:
	if ( (fRtn == false) && (m_pwavefile) ) {
		mprintf(("AUDIOSTR => ErrorExit for ::Create() on wave file: %s\n", pszFilename));

		if (m_source_id)
			OpenAL_ErrorPrint( alDeleteSources(1, &m_source_id) );

		m_pwavefile->Close();
		vm_free(m_pwavefile);
		m_pwavefile = NULL;
	}

	return (fRtn);
}

// Destroy
bool AudioStream::Destroy (void)
{
	bool fRtn = true;
	ALint buffers_processed = 0;

	ENTER_CRITICAL_SECTION(write_lock);

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
	if (m_pwavefile) {
		m_pwavefile->Close();
		vm_free(m_pwavefile);
		m_pwavefile = NULL;
	}

	status = ASF_FREE;

	LEAVE_CRITICAL_SECTION(write_lock);

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
		ENTER_CRITICAL_SECTION(Global_service_lock);
	}
		    
	if ( service ) {
		uncompressed_wave_data = Wavedata_service_buffer;
	} else {
		uncompressed_wave_data = Wavedata_load_buffer;
	}

	int num_bytes_read = 0;

	if ( !service ) {
		for (int ib = 0; ib < MAX_STREAM_BUFFERS; ib++) {
			num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, m_cbBufSize, service);

			if (num_bytes_read < 0) {
				m_bReadingDone = 1;
			} else if (num_bytes_read > 0) {
				OpenAL_ErrorCheck( alBufferData(m_buffer_ids[ib], m_pwavefile->GetALFormat(), uncompressed_wave_data, num_bytes_read, m_pwavefile->m_wfmt.nSamplesPerSec), { fRtn = false; goto ErrorExit; } );
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

			num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, m_cbBufSize, service);

			if (num_bytes_read < 0) {
				m_bReadingDone = 1;
			} else if (num_bytes_read > 0) {
				OpenAL_ErrorPrint( alBufferData(buffer_id, m_pwavefile->GetALFormat(), uncompressed_wave_data, num_bytes_read, m_pwavefile->m_wfmt.nSamplesPerSec) );
				OpenAL_ErrorPrint( alSourceQueueBuffers(m_source_id, 1, &buffer_id) );

				*num_bytes_written += num_bytes_read;
			}

			buffers_processed--;
		}
	}

ErrorExit:

	if ( service ) {
		LEAVE_CRITICAL_SECTION(Global_service_lock);
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

	ENTER_CRITICAL_SECTION( write_lock );

	// status may have changed, so lets check once again
	if ( status != ASF_USED ){
		LEAVE_CRITICAL_SECTION( write_lock );

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
				LEAVE_CRITICAL_SECTION( write_lock );

				Destroy();	
				// Reset reentrancy semaphore

				return false;
			} else {
				Stop_and_Rewind();
				// Reset reentrancy semaphore
				LEAVE_CRITICAL_SECTION( write_lock );

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

			if ( m_pwavefile->m_total_uncompressed_bytes_read >= m_pwavefile->m_max_uncompressed_bytes_to_read ) {
				m_fade_timer_id = timer_get_milliseconds() + 1700;		// start fading 1.7 seconds from now
				m_finished_id = timer_get_milliseconds() + 2000;		// 2 seconds left to play out buffer
				m_pwavefile->m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;
			}

			if ( (m_fade_timer_id>0) && ((uint)timer_get_milliseconds() > m_fade_timer_id) ) {
				m_fade_timer_id = 0;
				Fade_and_Stop();
			}

			if ( (m_finished_id>0) && ((uint)timer_get_milliseconds() > m_finished_id) ) {
				m_finished_id = 0;
				m_bPastLimit = true;
			}


			// see if we're done
			ALint state = 0;
			OpenAL_ErrorPrint( alGetSourcei(m_source_id, AL_SOURCE_STATE, &state) );

			if ( m_bReadingDone && (state != AL_PLAYING) ) {
				if ( m_bDestroy_when_faded == true ) {
					LEAVE_CRITICAL_SECTION( write_lock );

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

	LEAVE_CRITICAL_SECTION( write_lock );

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

		// Cue for playback if necessary
		if ( !m_fCued )
			Cue ();

		if ( looping )
			m_bLooping = 1;
		else
			m_bLooping = 0;

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

	m_pwavefile->m_max_uncompressed_bytes_to_read = ((sample_cutoff * m_pwavefile->m_wfmt.wBitsPerSample) / 8);
}

uint AudioStream::Get_Samples_Committed(void)
{
	if ( m_pwavefile == NULL )
		return 0;

	return ((m_pwavefile->m_total_uncompressed_bytes_read * 8) / m_pwavefile->m_wfmt.wBitsPerSample);
}


// Fade_and_Destroy
void AudioStream::Fade_and_Destroy (void)
{
	m_bFade = true;
	m_bDestroy_when_faded = true;
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
	}

#ifdef SCP_UNIX
	SDL_InitSubSystem(SDL_INIT_TIMER);
#endif

	INITIALIZE_CRITICAL_SECTION( Global_service_lock );

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

	DELETE_CRITICAL_SECTION( Global_service_lock );

	Audiostream_inited = 0;

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
int audiostream_open( char *filename, int type )
{
	int i, rc;
	char fname[MAX_FILENAME_LEN];

	if ( !Audiostream_inited || !snd_is_inited() )
		return -1;

	for (i = 0; i < MAX_AUDIO_STREAMS; i++) {
		if (Audio_streams[i].status == ASF_FREE) {
			Audio_streams[i].status = ASF_USED;
			Audio_streams[i].type = type;
			break;
		}
	}

	if (i == MAX_AUDIO_STREAMS) {
		nprintf(("Sound", "SOUND => No more audio streams available!\n"));
		return -1;
	}

	// copy filename, since we might modify it
	strcpy_s(fname, filename);

	// we always uncompress to 16 bits
	Audio_streams[i].m_bits_per_sample_uncompressed = 16;

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
			Int3();
			return -1;
	}

	rc = Audio_streams[i].Create(fname);

	if ( rc == 0 ) {
		Audio_streams[i].status = ASF_FREE;
		return -1;
	} else {
		return i;
	}
}

void audiostream_close_file(int i, int fade)
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

void audiostream_close_all(int fade)
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

	int done_reading;
	done_reading = Audio_streams[i].Is_Past_Limit();

	return Audio_streams[i].Is_Past_Limit();
}

int audiostream_is_inited()
{
	return Audiostream_inited;
}

void audiostream_pause(int i)
{
	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return;

	if ( audiostream_is_playing(i) == (int)true )
		audiostream_stop(i, 0, 1);
}

void audiostream_pause_all()
{
	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE )
			continue;

		audiostream_pause(i);
	}
}

void audiostream_unpause(int i)
{
	int is_looping;

	if ( i == -1 )
		return;

	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	if ( Audio_streams[i].status == ASF_FREE )
		return;

	if ( audiostream_is_paused(i) == (int)true ) {
		is_looping = Audio_streams[i].Is_looping();
		audiostream_play(i, -1.0f, is_looping);
	}
}

void audiostream_unpause_all()
{
	int i;

	for ( i = 0; i < MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE )
			continue;

		audiostream_unpause(i);
	}
}
