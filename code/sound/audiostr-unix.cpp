/*
 * $Logfile: $
 * $Revision: 1.2 $
 * $Date: 2004-07-26 17:39:19 $
 * $Author: Goober5000 $
 *
 * OpenAL based audio streaming
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/05/24 07:00:46  taylor
 * The Great Linux Merge
 *
 *
 * $NoKeywords: $
 */

#ifndef WIN32	// Goober5000

#include "globalincs/pstypes.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/acm.h"
#include "cfile/cfile.h"
#include "sound/sound.h"
#include "io/timer.h"
#include "osapi/osapi.h"


// status
#define ASF_FREE	0
#define ASF_USED	1

#define MAX_AUDIO_STREAMS	30

// Constants
#ifndef SUCCESS
#define SUCCESS TRUE        // Error returns for all member functions
#define FAILURE FALSE
#endif // SUCCESS

#define BIGBUF_SIZE					180000			// This can be reduced to 88200 once we don't use any stereo
//#define BIGBUF_SIZE					88300			// This can be reduced to 88200 once we don't use any stereo
unsigned char *Wavedata_load_buffer = NULL;		// buffer used for cueing audiostreams
unsigned char *Wavedata_service_buffer = NULL;	// buffer used for servicing audiostreams

CRITICAL_SECTION Global_service_lock;

#define COMPRESSED_BUFFER_SIZE	88300
unsigned char *Compressed_buffer = NULL;				// Used to load in compressed data during a cueing interval
unsigned char *Compressed_service_buffer = NULL;	// Used to read in compressed data during a service interval

#define AS_HIGHEST_MAX	999999999	// max uncompressed filesize supported is 999 meg


int Audiostream_inited = 0;


static int audiostr_read_uint(SDL_RWops *rw, uint *i)
{
	int rc = SDL_RWread(rw, i, sizeof(uint), 1);
	if (rc != 1)
		return 0;
	*i = INTEL_INT(*i);
	return 1;
}

static int audiostr_read_word(SDL_RWops *rw, WORD *i)
{
	int rc = SDL_RWread(rw, i, sizeof(WORD), 1);
	if (rc != 1)
		return 0;
	*i = INTEL_SHORT(*i);
	return 1;
}

static int audiostr_read_dword(SDL_RWops *rw, DWORD *i)
{
	int rc = SDL_RWread(rw, i, sizeof(DWORD), 1);
	if (rc != 1)
		return 0;
	*i = INTEL_INT(*i);
	return 1;
}

class WaveFile
{
public:
	void Init(void);
	void Close(void);
	BOOL Open (char *pszFilename);
	BOOL Cue (void);
	int	Read (ubyte *pbDest, ushort cbSize, int service=1);
	ushort GetNumBytesRemaining (void) { return (m_nDataSize - m_nBytesPlayed); }
	ushort GetUncompressedAvgDataRate (void) { return (m_nUncompressedAvgDataRate); }
	ushort GetDataSize (void) { return (m_nDataSize); }
	ushort GetNumBytesPlayed (void) { return (m_nBytesPlayed); }
	ubyte GetSilenceData (void);
	WAVEFORMATEX m_wfmt;					// format of wave file used by Direct Sound
	WAVEFORMATEX * m_pwfmt_original;	// foramt of wave file from actual wave source
	ushort m_total_uncompressed_bytes_read;
	ushort m_max_uncompressed_bytes_to_read;
	ushort	m_bits_per_sample_uncompressed;

protected:
	ushort m_data_offset;						// number of bytes to actual wave data
	int  m_data_bytes_left;
	SDL_RWops *cfp;

	ushort m_wave_format;						// format of wave source (ie WAVE_FORMAT_PCM, WAVE_FORMAT_ADPCM)
	ushort m_nBlockAlign;						// wave data block alignment spec
	ushort m_nUncompressedAvgDataRate;		// average wave data rate
	ushort m_nDataSize;							// size of data chunk
	ushort m_nBytesPlayed;						// offset into data chunk
	BOOL m_abort_next_read;

	void			*m_hStream;
	int				m_hStream_open;
	WAVEFORMATEX	m_wfxDest;
};

class AudioStream
{
public:
	AudioStream (void);
	~AudioStream (void);
	BOOL Create (char *pszFilename);
	BOOL Destroy (void);
	void Play (long volume, int looping);
	int Is_Playing(){ return(m_fPlaying); }
	int Is_Paused(){ return(m_bIsPaused); }
	int Is_Past_Limit() { return m_bPastLimit; }
	void Stop (int paused=0);
	void Stop_and_Rewind (void);
	void Fade_and_Destroy (void);
	void Fade_and_Stop(void);
	void	Set_Volume(long vol);
	long	Get_Volume();
	void	Init_Data();
	void	Set_Byte_Cutoff(unsigned int num_bytes_cutoff);
	void  Set_Default_Volume(long converted_volume) { m_lDefaultVolume = converted_volume; }
	long	Get_Default_Volume() { return m_lDefaultVolume; }
	uint Get_Bytes_Committed(void);
	int	Is_looping() { return m_bLooping; }
	int	status;
	int	type;
	ushort m_bits_per_sample_uncompressed;

protected:
	void Cue (void);
	BOOL WriteWaveData (uint cbSize, uint* num_bytes_written,int service=1);
	BOOL WriteSilence (uint cbSize);
	DWORD GetMaxWriteSize (void);
	BOOL ServiceBuffer (void);
	static BOOL TimerCallback (DWORD dwUser);

//	AudioStreamServices * m_pass;  // ptr to AudioStreamServices object
//	LPDIRECTSOUNDBUFFER m_pdsb;    // ptr to Direct Sound buffer
	WaveFile * m_pwavefile;        // ptr to WaveFile object
//	Timer m_timer;              // ptr to Timer object
	BOOL m_fCued;                  // semaphore (stream cued)
	BOOL m_fPlaying;               // semaphore (stream playing)
//	DSBUFFERDESC m_dsbd;           // Direct Sound buffer description
	long m_lInService;             // reentrancy semaphore
	ushort m_cbBufOffset;            // last write position
	ushort m_nBufLength;             // length of sound buffer in msec
	ushort m_cbBufSize;              // size of sound buffer in bytes
	ushort m_nBufService;            // service interval in msec
	ushort m_nTimeStarted;           // time (in system time) playback started

	BOOL	m_bLooping;						// whether or not to loop playback
	BOOL	m_bFade;							// fade out music 
	BOOL	m_bDestroy_when_faded;
	long	m_lVolume;						// volume of stream ( 0 -> -10 000 )
	long	m_lCutoffVolume;
	BOOL	m_bIsPaused;					// stream is stopped, but not rewinded
	ushort	m_silence_written;			// number of bytes of silence written to buffer
	ushort	m_bReadingDone;				// no more bytes to be read from disk, still have remaining buffer to play
	DWORD	m_fade_timer_id;				// timestamp so we know when to start fade
	DWORD	m_finished_id;					// timestamp so we know when we've played #bytes required
	BOOL	m_bPastLimit;					// flag to show we've played past the number of bytes requred
	long	m_lDefaultVolume;
	int		h_result;

	CRITICAL_SECTION write_lock;
};

// WaveFile class implementation
//
////////////////////////////////////////////////////////////

// Constructor
void WaveFile::Init(void)
{
	// Init data members
	m_data_offset = 0;
	cfp = NULL;
	m_pwfmt_original = NULL;
	m_nBlockAlign= 0;
	m_nUncompressedAvgDataRate = 0;
	m_nDataSize = 0;
	m_nBytesPlayed = 0;
	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;

	m_hStream_open = 0;
	m_abort_next_read = FALSE;
}

// Destructor
void WaveFile::Close(void)
{
	// Free memory
	if (m_pwfmt_original) {
		free(m_pwfmt_original);
		m_pwfmt_original = NULL;
	}

	if ( m_hStream_open ) {
		ACM_stream_close((void*)m_hStream);
		m_hStream_open = 0;
	}

	// Close file
	if (cfp) {
		SDL_RWclose(cfp);
		cfp = NULL;
	}
}


// Open
BOOL WaveFile::Open (char *pszFilename)
{
	int done = FALSE;
	WORD cbExtra = 0;
	BOOL fRtn = SUCCESS;    // assume success
	PCMWAVEFORMAT pcmwf;
	char fullpath[_MAX_PATH];

	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;

	int FileSize, FileOffset;

	if ( !cf_find_file_location(pszFilename, CF_TYPE_ANY, fullpath, &FileSize, &FileOffset ))	{
		goto OPEN_ERROR;
	}

	cfp = SDL_RWFromFile(fullpath, "rb");
	if ( cfp == NULL ) {
		goto OPEN_ERROR;
	}

	// Skip the "RIFF" tag and file size (8 bytes)
	// Skip the "WAVE" tag (4 bytes)
	SDL_RWseek( cfp, 12+FileOffset, SEEK_SET );

	// Now read RIFF tags until the end of file
	uint tag, size, next_chunk;

	while(done == FALSE)	{
		if ( !audiostr_read_uint(cfp, &tag) )
			break;

		if ( !audiostr_read_uint(cfp, &size) )
			break;

		next_chunk = SDL_RWseek( cfp, 0, SEEK_CUR );
		next_chunk += size;

		switch( tag )	{
		case 0x20746d66:		// The 'fmt ' tag
			audiostr_read_word(cfp, &pcmwf.wf.wFormatTag);
			audiostr_read_word(cfp, &pcmwf.wf.nChannels);
			audiostr_read_dword(cfp, &pcmwf.wf.nSamplesPerSec);
			audiostr_read_dword(cfp, &pcmwf.wf.nAvgBytesPerSec);
			audiostr_read_word(cfp, &pcmwf.wf.nBlockAlign);
			audiostr_read_word(cfp, &pcmwf.wBitsPerSample);
			
			if ( pcmwf.wf.wFormatTag != WAVE_FORMAT_PCM ) {
				audiostr_read_word(cfp, &cbExtra);
			}

			// Allocate memory for WAVEFORMATEX structure + extra bytes
			if ( (m_pwfmt_original = (WAVEFORMATEX *) malloc ( sizeof(WAVEFORMATEX)+cbExtra )) != NULL ){
				Assert(m_pwfmt_original != NULL);
				// Copy bytes from temporary format structure
				memcpy (m_pwfmt_original, &pcmwf, sizeof(pcmwf));
				m_pwfmt_original->cbSize = cbExtra;

				// Read those extra bytes, append to WAVEFORMATEX structure
				if (cbExtra != 0) {
					SDL_RWread(cfp, ((ubyte *)(m_pwfmt_original) + sizeof(WAVEFORMATEX)), cbExtra, 1);
				}
			}
			else {
				Int3();		// malloc failed
				goto OPEN_ERROR;
			}	
			break;

		case 0x61746164:		// the 'data' tag
			m_nDataSize = size;	// This is size of data chunk.  Compressed if ADPCM.
			m_data_bytes_left = size;
			m_data_offset = SDL_RWseek(cfp, 0, SEEK_CUR);
			done = TRUE;
			break;

		default:	// unknown, skip it
			break;
		}	// end switch

		SDL_RWseek(cfp, next_chunk, SEEK_SET);
	}

  	// At this stage, examine source format, and set up WAVEFORATEX structure for DirectSound.
	// Since DirectSound only supports PCM, force this structure to be PCM compliant.  We will
	// need to convert data on the fly later if our souce is not PCM
	switch ( m_pwfmt_original->wFormatTag ) {
		case WAVE_FORMAT_PCM:
			m_wave_format = WAVE_FORMAT_PCM;
			m_wfmt.wBitsPerSample = m_pwfmt_original->wBitsPerSample;
			break;

		case WAVE_FORMAT_ADPCM:
			m_wave_format = WAVE_FORMAT_ADPCM;
			m_wfmt.wBitsPerSample = 16;
			break;

		default:
			nprintf(("SOUND", "SOUND => Not supporting %d format for playing wave files\n"));
			//Int3();
			goto OPEN_ERROR;
			break;

	} // end switch
            
	// Set up the WAVEFORMATEX structure to have the right PCM characteristics
	m_wfmt.wFormatTag = WAVE_FORMAT_PCM;
	m_wfmt.nChannels = m_pwfmt_original->nChannels;
	m_wfmt.nSamplesPerSec = m_pwfmt_original->nSamplesPerSec;
	m_wfmt.cbSize = 0;
	m_wfmt.nBlockAlign = (unsigned short)(( m_wfmt.nChannels * m_wfmt.wBitsPerSample ) / 8);
	m_wfmt.nAvgBytesPerSec = m_wfmt.nBlockAlign * m_wfmt.nSamplesPerSec;

	// Init some member data from format chunk
	m_nBlockAlign = m_pwfmt_original->nBlockAlign;
	m_nUncompressedAvgDataRate = m_wfmt.nAvgBytesPerSec;

	// Cue for streaming
	Cue ();
 
	// Successful open
	goto OPEN_DONE;
    
OPEN_ERROR:
	// Handle all errors here
	nprintf(("SOUND","SOUND ==> Could not open wave file %s for streaming\n",pszFilename));

	fRtn = FAILURE;
	if (cfp != NULL) {
		// Close file
		SDL_RWclose(cfp);
		cfp = NULL;
	}
	if (m_pwfmt_original)
	{
		free(m_pwfmt_original);
		m_pwfmt_original = NULL;
	}

OPEN_DONE:
	return (fRtn);
}


// Cue
//
// Set the file pointer to the start of wave data
//
BOOL WaveFile::Cue (void)
{
	BOOL fRtn = SUCCESS;    // assume success
	int rval;

	m_total_uncompressed_bytes_read = 0;
	m_max_uncompressed_bytes_to_read = AS_HIGHEST_MAX;

	rval = SDL_RWseek(cfp, m_data_offset, SEEK_SET);
	if ( rval == -1 ) {
		fRtn = FAILURE;
	}

	m_data_bytes_left = m_nDataSize;
	m_abort_next_read = FALSE;

	return fRtn;
}


// Read
//
// Returns number of bytes actually read.
// 
//	Returns -1 if there is nothing more to be read.  This function can return 0, since
// sometimes the amount of bytes requested is too small for the ACM decompression to 
// locate a suitable block
int WaveFile::Read(ubyte *pbDest, ushort cbSize, int service)
{
	void	*dest_buf=NULL, *uncompressed_wave_data;
	int				rc, uncompressed_bytes_written;
	unsigned int	src_bytes_used, convert_len, num_bytes_desired=0, num_bytes_read;

//	nprintf(("Alan","Reqeusted: %d\n", cbSize));


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

		case WAVE_FORMAT_ADPCM:
			if ( !m_hStream_open ) {
				if ( !ACM_stream_open(m_pwfmt_original, &m_wfxDest, (void**)&m_hStream), m_bits_per_sample_uncompressed  ) {
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

		default:
			nprintf(("SOUND", "SOUND => Not supporting %d format for playing wave files\n"));
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

	if ( m_data_bytes_left > 0 && num_bytes_desired > 0 ) {
		int actual_read;

		if ( num_bytes_desired <= (unsigned int)m_data_bytes_left ) {
			num_bytes_read = num_bytes_desired;
		}
		else {
			num_bytes_read = m_data_bytes_left;
		}

		actual_read = SDL_RWread( cfp, dest_buf, num_bytes_read, 1);
		if ( (actual_read <= 0) || (m_abort_next_read) ) {
			num_bytes_read = 0;
			uncompressed_bytes_written = 0;
			return -1;
		}

		if ( num_bytes_desired >= (unsigned int)m_data_bytes_left ) {
			m_abort_next_read = 1;			
		}

		num_bytes_read = actual_read;
	}

	// convert data if necessary, to PCM
	if ( m_wave_format == WAVE_FORMAT_ADPCM ) {
		if ( num_bytes_read > 0 ) {
			//	rc = ACM_convert((void*)m_hStream, (ubyte*)dest_buf, num_bytes_read, uncompressed_wave_data, BIGBUF_SIZE, &convert_len, &src_bytes_used);
				if ( rc == -1 ) {
					goto READ_ERROR;
				}
				if ( convert_len == 0 ) {
					Int3();
				}
		}

		Assert(src_bytes_used <= num_bytes_read);
		if ( src_bytes_used < num_bytes_read ) {
			// seek back file pointer to reposition before unused source data
			SDL_RWseek(cfp, src_bytes_used - num_bytes_read, SEEK_CUR);
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


// GetSilenceData
//
// Returns 8 bits of data representing silence for the Wave file format.
//
// Since we are dealing only with PCM format, we can fudge a bit and take
// advantage of the fact that for all PCM formats, silence can be represented
// by a single byte, repeated to make up the proper word size. The actual size
// of a word of wave data depends on the format:
//
// PCM Format       Word Size       Silence Data
// 8-bit mono       1 byte          0x80
// 8-bit stereo     2 bytes         0x8080
// 16-bit mono      2 bytes         0x0000
// 16-bit stereo    4 bytes         0x00000000
//
ubyte WaveFile::GetSilenceData (void)
{
	ubyte bSilenceData = 0;

	// Silence data depends on format of Wave file
	if (m_pwfmt_original) {
		if (m_wfmt.wBitsPerSample == 8) {
			// For 8-bit formats (unsigned, 0 to 255)
			// Packed DWORD = 0x80808080;
			bSilenceData = 0x80;
		} else if (m_wfmt.wBitsPerSample == 16) {
			// For 16-bit formats (signed, -32768 to 32767)
			// Packed DWORD = 0x00000000;
			bSilenceData = 0x00;
		} else {
			Int3();
		}
	} else {
		Int3();
	}

	return (bSilenceData);
}

//
// AudioStream class implementation
//
////////////////////////////////////////////////////////////

// The following constants are the defaults for our streaming buffer operation.
const ushort DefBufferLength          = 2000; // default buffer length in msec
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
	m_bFade = FALSE;
	m_fade_timer_id = 0;
	m_finished_id = 0;
	m_bPastLimit = FALSE;
	
	m_bDestroy_when_faded = FALSE;
	m_lVolume = 0;
	m_lCutoffVolume = -10000;
	m_bIsPaused = FALSE;
	m_silence_written = 0;
	m_bReadingDone = FALSE;

	m_pwavefile = NULL;
//	m_pdsb = NULL;
	m_fPlaying = m_fCued = FALSE;
	m_lInService = FALSE;
	m_cbBufOffset = 0;
	m_nBufLength = DefBufferLength;
	m_cbBufSize = 0;
	m_nBufService = DefBufferServiceInterval;
	m_nTimeStarted = 0;
}

// Create
BOOL AudioStream::Create (char *pszFilename)
{

	BOOL fRtn = SUCCESS;    // assume success

	Assert(pszFilename);

	Init_Data();

	if (pszFilename) {
		// Create a new WaveFile object
		m_pwavefile = (WaveFile *)malloc(sizeof(WaveFile));
		Assert(m_pwavefile);

		if (m_pwavefile) {
			// Call constructor
			m_pwavefile->Init();
			// Open given file
			m_pwavefile->m_bits_per_sample_uncompressed = m_bits_per_sample_uncompressed;
			if (m_pwavefile->Open (pszFilename)) {
				// Calculate sound buffer size in bytes
				// Buffer size is average data rate times length of buffer
				// No need for buffer to be larger than wave data though
				m_cbBufSize = (m_pwavefile->GetUncompressedAvgDataRate () * m_nBufLength) / 1000;
				nprintf(("SOUND", "SOUND => Stream buffer created using %d bytes\n", m_cbBufSize));
				// m_cbBufSize = (m_cbBufSize > m_pwavefile->GetDataSize ()) ? m_pwavefile->GetDataSize () : m_cbBufSize;

				//nprintf(("Sound", "SOUND => average data rate = %d\n\r", m_pwavefile->GetUncompressedAvgDataRate ()));
				//nprintf(("Sound", "SOUND => m_cbBufSize = %d\n\r", m_cbBufSize));
/*
				// Create sound buffer
				int hr;
				memset (&m_dsbd, 0, sizeof (DSBUFFERDESC));
				m_dsbd.dwSize = sizeof (DSBUFFERDESC);
				m_dsbd.dwBufferBytes = m_cbBufSize;
				m_dsbd.lpwfxFormat = &m_pwavefile->m_wfmt;
				m_dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;

				hr = (m_pass->GetPDS ())->CreateSoundBuffer (&m_dsbd, &m_pdsb, NULL);
				if (hr == DS_OK) {
					// Cue for playback
					Cue ();
					Snd_sram += m_cbBufSize;
				}
				else {
					// Error, unable to create DirectSound buffer
					nprintf(("Sound", "SOUND => Error, unable to create DirectSound buffer\n\r"));
					if (hr == DSERR_BADFORMAT) {
						nprintf(("Sound", "SOUND => Bad format (probably ADPCM)\n\r"));
					}

					fRtn = FAILURE;
				}*/
			}
			else {
				// Error opening file
				nprintf(("SOUND", "SOUND => Failed to open wave file: %s\n\r", pszFilename));
				m_pwavefile->Close();
				free(m_pwavefile);
				m_pwavefile = NULL;
				fRtn = FAILURE;
			}   
		}
		else {
			// Error, unable to create WaveFile object
			nprintf(("Sound", "SOUND => Failed to create WaveFile object %s\n\r", pszFilename));
			fRtn = FAILURE;
		}
	}
	else {
		// Error, passed invalid parms
		fRtn = FAILURE;
	}

	return (fRtn);
}

// Destroy
BOOL AudioStream::Destroy (void)
{
	BOOL fRtn = SUCCESS;

	ENTER_CRITICAL_SECTION(write_lock);

	// Stop playback
	Stop ();

	// Release DirectSound buffer
/*	if (m_pdsb) {
		m_pdsb->Release ();
		m_pdsb = NULL;
		Snd_sram -= m_cbBufSize;
	}
*/
	// Delete WaveFile object
	if (m_pwavefile) {
		m_pwavefile->Close();
		free(m_pwavefile);
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
BOOL AudioStream::WriteWaveData (uint size, uint *num_bytes_written, int service)
{
	HRESULT hr = 0;
	LPBYTE lpbuf1 = NULL;
	LPBYTE lpbuf2 = NULL;
	DWORD dwsize1 = 0;
	DWORD dwsize2 = 0;
	DWORD dwbyteswritten1 = 0;
	DWORD dwbyteswritten2 = 0;
	BOOL fRtn = SUCCESS;
	unsigned char	*uncompressed_wave_data;

	*num_bytes_written = 0;

	if ( size == 0 || m_bReadingDone ) {
		return fRtn;
	}

	if ( /*!m_pdsb ||*/ !m_pwavefile ) {
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

    // Lock the sound buffer
//	hr = m_pdsb->Lock (m_cbBufOffset, size, (void**)(&lpbuf1), &dwsize1, (void**)(&lpbuf2), &dwsize2, 0);
	if (hr == 0) {
		// Write data to sound buffer. Because the sound buffer is circular, we may have to
		// do two write operations if locked portion of buffer wraps around to start of buffer.
		Assert(lpbuf1);
		
		num_bytes_read = m_pwavefile->Read(uncompressed_wave_data, dwsize1+dwsize2,service);
		if ( num_bytes_read == -1 ) {
			// means nothing left to read!
			num_bytes_read = 0;
			m_bReadingDone = 1;
		}

		if ( num_bytes_read > 0 ) {
			if ( (unsigned int)num_bytes_read > dwsize1 ) {
				dwbyteswritten1 = dwsize1;
				dwbyteswritten2 = num_bytes_read - dwsize1;

				memcpy(lpbuf1, uncompressed_wave_data, dwsize1);
				Assert(lpbuf2);
				memcpy(lpbuf2, uncompressed_wave_data+dwsize1, num_bytes_read-dwsize1);
			} else {
				dwbyteswritten1 = num_bytes_read;
				dwbyteswritten2 = 0;
				memcpy(lpbuf1, uncompressed_wave_data, num_bytes_read);
			}
		}
			
		// Update our buffer offset and unlock sound buffer
		m_cbBufOffset = (m_cbBufOffset + dwbyteswritten1 + dwbyteswritten2) % m_cbBufSize;
		*num_bytes_written = dwbyteswritten1 + dwbyteswritten2;
	//	m_pdsb->Unlock (lpbuf1, dwsize1, lpbuf2, dwsize2);
    }
	else {
		// Error locking sound buffer
		nprintf(("SOUND", "SOUND ==> Error, unable to lock sound buffer in AudioStr\n"));
		fRtn = FAILURE;
	}

	if ( service ) {
		LEAVE_CRITICAL_SECTION(Global_service_lock);
	}
    
	return (fRtn);
}


// WriteSilence
//
// Writes silence to sound buffer. This is a helper method used by
// ServiceBuffer; it's not exposed to users of the AudioStream class.
BOOL AudioStream::WriteSilence (uint size)
{
	int hr = 0;
	LPBYTE lpbuf1 = NULL;
	LPBYTE lpbuf2 = NULL;
	DWORD dwsize1 = 0;
	DWORD dwsize2 = 0;
	DWORD dwbyteswritten1 = 0;
	DWORD dwbyteswritten2 = 0;
	BOOL fRtn = SUCCESS;

	// Lock the sound buffer
//	hr = m_pdsb->Lock (m_cbBufOffset, size, (void**)(&lpbuf1), &dwsize1, (void**)(&lpbuf2), &dwsize2, 0);
	if (hr == 0) {

		// Get silence data for this file format. Although word sizes vary for different
		// wave file formats, ::Lock will always return pointers on word boundaries.
		// Because silence data for 16-bit PCM formats is 0x0000 or 0x00000000, we can
		// get away with writing bytes and ignoring word size here.
		ubyte bSilence = m_pwavefile->GetSilenceData ();
        
		// Write silence to sound buffer. Because the sound buffer is circular, we may have to
		// do two write operations if locked portion of buffer wraps around to start of buffer.
		memset (lpbuf1, bSilence, dwsize1);
		dwbyteswritten1 = dwsize1;
            
     // Second write required?
		if (lpbuf2) {
			memset (lpbuf2, bSilence, dwsize2);
			dwbyteswritten2 = dwsize2;
		}
            
		// Update our buffer offset and unlock sound buffer
		m_cbBufOffset = (m_cbBufOffset + dwbyteswritten1 + dwbyteswritten2) % m_cbBufSize;
//		m_pdsb->Unlock (lpbuf1, dwbyteswritten1, lpbuf2, dwbyteswritten2);
//		m_pdsb->Unlock (lpbuf1, dwsize1, lpbuf2, dwsize2);
	}
	else {
		// Error locking sound buffer
		nprintf(("SOUND", "SOUND ==> Error, unable to lock sound buffer in AudioStr\n"));
		fRtn = FAILURE;
	}

	return (fRtn);
}


// GetMaxWriteSize
//
// Helper function to calculate max size of sound buffer write operation, i.e. how much
// free space there is in buffer.
DWORD AudioStream::GetMaxWriteSize (void)
{/*
	DWORD dwWriteCursor, dwPlayCursor, dwMaxSize;

	// Get current play position
	if (m_pdsb->GetCurrentPosition (&dwPlayCursor, &dwWriteCursor) == DS_OK) {
		if (m_cbBufOffset <= dwPlayCursor) {
			// Our write position trails play cursor
			dwMaxSize = dwPlayCursor - m_cbBufOffset;
		}

		else  {// (m_cbBufOffset > dw7Cursor)
			// Play cursor has wrapped
			dwMaxSize = m_cbBufSize - m_cbBufOffset + dwPlayCursor;
		}
	}
	else {
		// GetCurrentPosition call failed
		Int3();
		dwMaxSize = 0;
	}

//	nprintf(("Alan","Max write size: %d\n", dwMaxSize));
	return (dwMaxSize);
*/return 0;}

// Cue
void AudioStream::Cue (void)
{
	uint num_bytes_written;

	if (!m_fCued) {
		m_bFade = FALSE;
		m_fade_timer_id = 0;
		m_finished_id = 0;
		m_bPastLimit = FALSE;
		m_lVolume = 0;
		m_lCutoffVolume = -10000;

		m_bDestroy_when_faded = FALSE;

		// Reset buffer ptr
		m_cbBufOffset = 0;

		// Reset file ptr, etc
		m_pwavefile->Cue ();

		// Reset DirectSound buffer
//		m_pdsb->SetCurrentPosition (0);

		// Fill buffer with wave data
		WriteWaveData (m_cbBufSize, &num_bytes_written,0);

		m_fCued = TRUE;
	}
}

// Play
void AudioStream::Play (long volume, int looping)
{/*
	if (m_pdsb) {
		// If playing, stop
		if (m_fPlaying) {
			if ( m_bIsPaused == FALSE)
			Stop_and_Rewind();
		}

		// Cue for playback if necessary
		if (!m_fCued) {
			Cue ();
		}

		if ( looping )
			m_bLooping = 1;
		else
			m_bLooping = 0;

		// Begin DirectSound playback
		int hr = m_pdsb->Play (0, 0, DSBPLAY_LOOPING);
		if (hr == 0) {
			m_nTimeStarted = timer_get_milliseconds();
			Set_Volume(volume);
			// Kick off timer to service buffer
			m_timer.constructor();

			m_timer.Create (m_nBufService, m_nBufService, DWORD (this), TimerCallback);

			// Playback begun, no longer cued
			m_fPlaying = TRUE;
			m_bIsPaused = FALSE;
		}
		else {
			// If the buffer was lost, try to restore it
			if ( hr == -1 ) {
				hr = m_pdsb->Restore();
				if ( hr == 0 ) {
					hr = m_pdsb->Play (0, 0, DSBPLAY_LOOPING);
				}
				else {
					nprintf(("Sound", "Sound => Lost a buffer, tried restoring but got %s\n", get_DSERR_text(hr) ));
					Int3();	// get Alan, he wants to see this
				}
			}

			if ( hr != 0 ) {
				nprintf(("Sound", "Sound => Play failed with return value %s\n", get_DSERR_text(hr) ));
			}
		}
	}
*/}

void AudioStream::Set_Byte_Cutoff(unsigned int byte_cutoff)
{
	if ( m_pwavefile == NULL )
		return;

	m_pwavefile->m_max_uncompressed_bytes_to_read = byte_cutoff;
}

unsigned int AudioStream::Get_Bytes_Committed(void)
{
	if ( m_pwavefile == NULL )
		return 0;

	return m_pwavefile->m_total_uncompressed_bytes_read;
}


// Fade_and_Destroy
void AudioStream::Fade_and_Destroy (void)
{
	m_bFade = TRUE;
	m_bDestroy_when_faded = TRUE;
}

// Fade_and_Destroy
void AudioStream::Fade_and_Stop (void)
{
	m_bFade = TRUE;
	m_bDestroy_when_faded = FALSE;
}


// Stop
void AudioStream::Stop(int paused)
{
	if (m_fPlaying) {
		// Stop DirectSound playback
//		m_pdsb->Stop ();
		m_fPlaying = FALSE;
		m_bIsPaused = paused;

		// Delete Timer object
//		m_timer.destructor();
	}
}

// Stop_and_Rewind
void AudioStream::Stop_and_Rewind (void)
{
	if (m_fPlaying) {
		// Stop DirectSound playback
//		m_pdsb->Stop ();

		// Delete Timer object
//		m_timer.destructor();

		m_fPlaying = FALSE;
	}

	m_fCued = FALSE;	// this will cause wave file to start from beginning
	m_bReadingDone = FALSE;
}

// Set_Volume
void AudioStream::Set_Volume(long vol)
{
	if ( vol < -10000 )
		vol = -10000;
	
	if ( vol > 0 )
		vol = 0;

	Assert( vol >= -10000 && vol <= 0 );
//	h_result = m_pdsb->SetVolume(vol);
	m_lVolume = vol;
	if ( h_result != 0 )
		nprintf(("Sound","SOUND => SetVolume() failed with code '%s'\n", get_DSERR_text(h_result) ));
}


// Set_Volume
long AudioStream::Get_Volume()
{
	return m_lVolume;
}



AudioStream Audio_streams[MAX_AUDIO_STREAMS];


void audiostream_init()
{
	STUB_FUNCTION;
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
		free(Wavedata_load_buffer);
		Wavedata_load_buffer = NULL;
	}

	if ( Wavedata_service_buffer ) {
		free(Wavedata_service_buffer);
		Wavedata_service_buffer = NULL;
	}

	if ( Compressed_buffer ) {
		free(Compressed_buffer);
		Compressed_buffer = NULL;
	}

	if ( Compressed_service_buffer ) {
		free(Compressed_service_buffer);
		Compressed_service_buffer = NULL;
	}

//	DeleteCriticalSection( &Global_service_lock );

	Audiostream_inited = 0;

}

// Open a digital sound file for streaming
//
// input:	filename	=>	disk filename of sound file
//				type		=> what type of audio stream do we want to open:
//									ASF_SOUNDFX
//									ASF_EVENTMUSIC
//									ASF_VOICE
//	
// returns:	success => handle to identify streaming sound
//				failure => -1
int audiostream_open( char *filename, int type )
{
	int i, rc;

	if (!Audiostream_inited || !snd_is_inited())
		return -1;

	for ( i=0; i<MAX_AUDIO_STREAMS; i++ ) {
		if ( Audio_streams[i].status == ASF_FREE ) {
			Audio_streams[i].status = ASF_USED;
			Audio_streams[i].type = type;
			break;
		}
	}

	if ( i == MAX_AUDIO_STREAMS ) {
		nprintf(("Sound", "SOUND => No more audio streams available!\n"));
		return -1;
	}

	switch(type) {
		case ASF_VOICE:
		case ASF_SOUNDFX:
			Audio_streams[i].m_bits_per_sample_uncompressed = 8;
		case ASF_EVENTMUSIC:
			Audio_streams[i].m_bits_per_sample_uncompressed = 16;
			break;
		default:
			Int3();
			return -1;
	}

	rc = Audio_streams[i].Create(filename);

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
		if ( fade == TRUE ) {
			Audio_streams[i].Fade_and_Destroy();
		} else {
			Audio_streams[i].Destroy();
		}
	}
}

void audiostream_close_all(int fade)
{
	STUB_FUNCTION;
}

void audiostream_play(int i, float volume, int looping)
{
	if (!Audiostream_inited)
		return;

	if ( i == -1 )
		return;

	Assert(looping >= 0);
	Assert( i >= 0 && i < MAX_AUDIO_STREAMS );

	// convert from 0->1 to -10000->0 for volume
	int converted_volume;
	if ( volume == -1 ) {
		converted_volume = Audio_streams[i].Get_Default_Volume();
	} else {
		Assert(volume >= 0.0f && volume <= 1.0f );
		converted_volume = ds_convert_volume(volume);
	}

	Assert( Audio_streams[i].status == ASF_USED );
	Audio_streams[i].Set_Default_Volume(converted_volume);
	Audio_streams[i].Play(converted_volume, looping);
}

int audiostream_is_playing(int i)
{
	STUB_FUNCTION;
	
	return -1;
}

void audiostream_stop(int i, int rewind, int paused)
{
	STUB_FUNCTION;
}

void audiostream_set_volume_all(float volume, int type)
{
	STUB_FUNCTION;
}

void audiostream_set_volume(int i, float volume)
{
	STUB_FUNCTION;
}

int audiostream_is_paused(int i)
{
	STUB_FUNCTION;
	
	return -1;
}

void audiostream_set_byte_cutoff(int i, unsigned int cutoff)
{
	STUB_FUNCTION;
}

uint audiostream_get_bytes_committed(int i)
{
	STUB_FUNCTION;
	
	return 0;
}

int audiostream_done_reading(int i)
{
	STUB_FUNCTION;
	
	return -1;
}

int audiostream_is_inited()
{
	STUB_FUNCTION;
	
	return -1;
}

void audiostream_pause(int i)
{
	STUB_FUNCTION;
}

void audiostream_pause_all()
{
	STUB_FUNCTION;
}

void audiostream_unpause(int i)
{
	STUB_FUNCTION;
}

void audiostream_unpause_all()
{
	STUB_FUNCTION;
}

#endif		// Goober5000 - #ifndef WIN32