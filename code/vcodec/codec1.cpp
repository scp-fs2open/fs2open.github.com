//                                  Codec1.cpp
//
// Contains C-style-C++ implementation of VoiCTech (Voice Communications
// Technology) voice encoder/decoder.
//
// Written by Matthew F. Storch, Ph.D., copyright (c) 1998 Volition Inc.

#include "pstypes.h"
#include <windows.h>

#include <math.h>
#include <assert.h>
#include "codec1.h"


//////////////////////////////////////////////////////////////////////////////
//
// Introduction to the VoiCTech encoder
// ------------------------------------
// The VoiCTech (short for Voice Communication Technology, pronounced
// "voice-tech") audio codec uses two separate algorithm suites: Codec1
// and LPC-10.  Codec1 does compression without using traditional signal 
// processing algorithms (expcept for a simple FIR low-pass filter).  As
// a result, it is extremely fast, and yields moderate-quality voice at a
// compression ratio of between 4-to-1 and 10-to-1, assuming 11KHz sampled
// data.  LPC-10 does significantly more analysis and is therefore slower
// (approximately 3 times slower than Codec1), but achieves substantially
// better compression (25-1) at a comparable quality.
//
// The externally-callable interface for both algorithm suites is through
// a simple, generic front-end that is prototyped in codec1.h.    
//
// This file contains the implementions of both the generic interface and
// Codec1. LPC-10 is implemented in a set of files in the LPC10 subdirectory.
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Low pass filter stuff.
//
// Number of points to convolve with.  Larger number means a better but
// slower low-pass filter.  Number should always be odd.  Useful range is
// 7 -> 31.
const int LPF_NUM_POINTS = 11; 
double Encoder_LPF_Coef[LPF_NUM_POINTS]; // convolution coefficents (weights)
// lookup table for coefficents * every possible sample value
char Encoder_LPF_CoefTimesSample[LPF_NUM_POINTS][256];
double Decoder_LPF_Coef[LPF_NUM_POINTS]; // convolution coefficents (weights)
// lookup table for coefficents * every possible sample value
char Decoder_LPF_CoefTimesSample[LPF_NUM_POINTS][256];
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Function prototypes for functions internal to Codec1

static void InitEncoder1(int QoS);
static void Smooth1(t_Sample* bufIn, t_Sample* bufOut, int size);
static double AutoGain1(t_Sample* bufIn, t_Sample* bufOut, int size);
static void UnAutoGain1(t_Sample* bufIn, t_Sample* bufOut, int size, 
						double gain);
static void Decode1(t_Sample* bufIn, t_Sample* bufOut, int size, int sizeOut);

#if defined(CODEC_DEMO)
static int Encode1(t_Sample* bufIn, t_Sample* bufOut, int sizeIn, int sizeOut,
                   t_Sample* levels, int* modes, int samples[9], int storage[9]);
#else
static int Encode1(t_Sample* bufIn, t_Sample* bufOut, int sizeIn, int sizeOut);
#endif

static void SkipEveryOther(t_Sample* bufIn, t_Sample* bufOut, int size);
static void InterpolateEveryOther(t_Sample* bufIn, t_Sample* bufOut, int size);

#if defined(CODEC_DEMO)
static int DoEncode(int mode, BOOL& packetPos, t_Sample*& in, t_Sample*& out, 
                    int& level, t_Sample*& levels, int*& modes, 
                    int samples[9], int storage[9]);
#else
static int DoEncode(int mode, BOOL& packetPos, t_Sample*& in, t_Sample*& out, 
                    int& level);
#endif

static void DecodeRL(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
                     t_Sample* bufEnd, t_Sample* bufOutEnd);
static void DecodeHF(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
					 t_Sample* bufOutEnd);
static void DecodeNom(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
                      unsigned int mode, t_Sample* bufOutEnd);
static void DecodeMF(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
					 t_Sample* bufOutEnd);
static void InitLowPassFilter(int QoS, double LPF_Coef[LPF_NUM_POINTS],
							  char LPF_CoefTimesSample[LPF_NUM_POINTS][256]);
static void LowPassFilter(t_Sample* bufIn, t_Sample* bufOut, int size,
						  char LPF_CoefTimesSample[LPF_NUM_POINTS][256]);

#if defined(USE_LPC10)
static void ConvertToLPC10(t_Sample* bufIn, t_Sample* bufOut, int size);
static void ConvertFromLPC10(t_Sample* bufIn, t_Sample* bufOut, int size);
extern "C"
{
    // the following three functions are defined in lpc10\lpc10.c
    void lpc10init();
    int lpc10encode(unsigned char *in, unsigned char *out, int inlen);
    int lpc10decode(unsigned char *in, unsigned char *out, int inlen);
}
#define AssertLPC10Available()
#else
#define ConvertToLPC10(bufIn, bufOut, size)
#define ConvertFromLPC10(bufIn, bufOut, size)
#define lpc10init()
#define lpc10encode(in, out, inlen) 0
#define lpc10decode(in, out, inlen) 0
#define AssertLPC10Available() assert(0)
#endif // defined(USE_LPC10)

//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Generic high-level codec interface.

t_Code EncodeMode = e_cLPC10;

t_Sample* TempDecoderBuf = NULL;
t_Sample* TempEncoderBuf1 = NULL;
t_Sample* TempEncoderBuf2 = NULL;

///////////////////////////////
// The following compile-time switches are applicable to Codec1 only.

// If SKIP_EVERY_OTHER is defined, every other sample will be thrown out
// after the low-pass filter has massaged the data but before the encoder
// goes at it.  The missing samples are then interpolated after decoding.
#define SKIP_EVERY_OTHER

// Don't define this ;)
//#define REMOVE_DC_BIAS

// Define this to use logarithmic gain (but linear gain works better).
//#define LOGARITHMIC_GAIN

// Define this to enable the low-pass filter.
#define USE_LOWPASS_FILTER

// End of compile-time switches
///////////////////////////////

#if defined(SKIP_EVERY_OTHER)
	const int SKIP_FACTOR = 2;
#else
	const int SKIP_FACTOR = 1;
#endif


void InitDecoder(int QoS, t_Sample* tempBuf) 
{ 
	QoS = QoS; // just to shut compiler up
    TempDecoderBuf = tempBuf;
    lpc10init(); // call unconditionally because we don't know what kind of
                 // coded packets we might receive
	InitLowPassFilter(8, Decoder_LPF_Coef, Decoder_LPF_CoefTimesSample);
}

void Decode(t_CodeInfo* ci, t_Sample* bufIn, t_Sample* bufOut, 
            int encodeSize, int decodeSize)
{
    if (ci->Code == e_cCodec1)
    {
        Decode1(bufIn, TempDecoderBuf, encodeSize, decodeSize/SKIP_FACTOR);
	  #if defined(SKIP_EVERY_OTHER)
		UnAutoGain1(TempDecoderBuf, TempDecoderBuf, decodeSize/SKIP_FACTOR, ci->Gain);
        InterpolateEveryOther(TempDecoderBuf, bufOut, decodeSize/SKIP_FACTOR);
	  #else
		UnAutoGain1(TempDecoderBuf, bufOut, decodeSize, ci->Gain);
	  #endif
		LowPassFilter(bufOut, bufOut, decodeSize,Decoder_LPF_CoefTimesSample);
    }
    else
    {
		AssertLPC10Available();
        lpc10decode(bufIn, bufOut, encodeSize);
        ConvertFromLPC10(bufOut, bufOut, decodeSize);
    }
}

void InitEncoder(t_Code code, int QoS, t_Sample* tempBuf1, t_Sample* tempBuf2)
{
    TempEncoderBuf1 = tempBuf1;
    TempEncoderBuf2 = tempBuf2;
    EncodeMode = code;
    if (code == e_cCodec1)
	{
        InitEncoder1(QoS);
		InitLowPassFilter(QoS, Encoder_LPF_Coef, Encoder_LPF_CoefTimesSample);
	}
}

#if defined(CODEC_DEMO)
#define EXTRA_CODEC_DEMO_ARGS4 , levels, modes, samples, storage
int Encode(t_Sample* bufIn, t_Sample* bufOut, int sizeIn, int sizeOut,
		   t_CodeInfo* codeInfo,
           t_Sample* levels, int* modes, int samples[9], int storage[9])
#else
#define EXTRA_CODEC_DEMO_ARGS4
int Encode(t_Sample* bufIn, t_Sample* bufOut, int sizeIn, int sizeOut, 
		   t_CodeInfo* codeInfo)
#endif
{
    int encodeSize;
	codeInfo->Code = EncodeMode;
    if (EncodeMode == e_cCodec1)
    {
	  #if defined(SKIP_EVERY_OTHER)
		LowPassFilter(bufIn, TempEncoderBuf1, sizeIn,

					  Encoder_LPF_CoefTimesSample);
        SkipEveryOther(TempEncoderBuf1, TempEncoderBuf1, sizeIn);
	  #else
		LowPassFilter(bufIn, TempEncoderBuf1, sizeIn,
					  Encoder_LPF_CoefTimesSample);

	  #endif
        Smooth1(TempEncoderBuf1, TempEncoderBuf2, sizeIn/SKIP_FACTOR);
		codeInfo->Gain = AutoGain1(TempEncoderBuf2, TempEncoderBuf2, 
								   sizeIn/SKIP_FACTOR);
      #if defined(CODEC_DEMO)
        encodeSize = Encode1(TempEncoderBuf2, bufOut, sizeIn/SKIP_FACTOR, 
                             sizeOut, levels, modes, samples, storage);
      #else
        encodeSize = Encode1(TempEncoderBuf2, bufOut, sizeIn/SKIP_FACTOR, 
                             sizeOut);
      #endif
    }
    else
    {
		AssertLPC10Available();
        ConvertToLPC10(bufIn, TempEncoderBuf2, sizeIn);
        encodeSize = lpc10encode(TempEncoderBuf2, bufOut, sizeIn);
    }
    return encodeSize;
}

//
//////////////////////////////////////////////////////////////////////////////



// ********* EVERYTHING FROM THIS POINT ON IS SPECIFIC TO CODEC1. ************


//////////////////////////////////////////////////////////////////////////////
//
// Introduction to Codec1
// ----------------------
// Codec1 uses multiple encoders, including a highly modal primary
// encoder, to achieve a substantial degree of compression while
// accurately preserving the low-to-mid frequency components of human
// voice.  Higher frequency components are preserved less well, but
// usually acceptably.  The encoder works best with samples taken at
// low-to-moderate microphone volumes, though an additonal logarithmic
// compression pass has been added to give better performance at high
// volumes.
//
// The interface is very simple.  The externally-visible functions are:
//    InitEncoder1   -- initialize the encoder
//    LowPassFilter  -- get rid of unwanted high frequency data
//    SkipEveryOther -- throw out every other sample (optional)
//    Smooth1        -- smooth little bumps so run-length encoding works better
//    AutoGain1      -- reduce problematic high-volume samples
//    Encode1        -- primary encoder
//    ~~~~~~ transmission to remote machine ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    Decode1        -- primary decoder
//    UnAutoGain1    -- restore true volume
//    InterpolateEveryOther -- bring back missing samples
//
// All encoder and decoder algorithms have O(N) running time with
// respect to the number of samples (otherwise it would be hopelessly
// slow).  Decoding is extremely fast and should not have a
// significant impact on a modern computer.  The C implementation is
// most likely fine for production code.  The encoding is quite fast
// compared with other voice encoders, but it is of course substantially
// slower than the decoding.  Because of the "bit twiddling" nature of
// the algorithms, I would expect that a good assembly language 
// implementation could easily provide 2X-3X the execution time performance
// of this C implementation.  As of 12/97, I have implemented one key
// routine in assembly that increased performance substantially. 
// 
// The encoded format is novel and somewhat complex because of the
// desire for very high degrees of compression (4:1 to 10:1, nominally
// 8:1) with respectable sound quality.  In order to achieve such
// extraordinarily high compression rates, the exact format is
// (unfortunately) specific to 8-bit samples.  Ideas very similar to
// those used for the 8-bit codec could be used to develop 16 or
// 24-bit codecs, but the details would differ enough to require a
// mostly new implementation.
//
// A sequence of samples in the original data is encoded as a
// "packet".  A single packet may describe anywhere from a few samples
// to several thousand samples depending on the type of packet.  There
// are currently 4 major categories of packets: nominal packets,
// run-length packets, high-frequency packets, and medium-frequency
// packets.  An additional category, low-frequency packets, may be
// added later.
//
// The centerpiece of the encoding strategy is the category of nominal
// packets.  Nominal packets encode 9 contiguous 8-bit samples in 12
// bits, for a compression ratio of 6:1 and very high fidelity.  The
// encoded value of each sample is relative to the value of the
// previous sample, so this technique can only be effectively applied
// when contiguous samples have values that are not too different
// (i.e. for samples that contain only low-to-medium frequency
// components).  Detailed examination of actual voice samples revealed
// that monotonic upward (or downward) runs of several samples were
// common, so I decided that the encoding strategy would be as
// follows.  Each bit in a nominal packet corresponds to one sample,
// and the value of the bit determines what offset is added to the
// previous sample value to define the current sample value.  The
// actual offset values depend on the mode of the packet.  For
// example, in a mode 3 packet, a data bit of 0 means the offset is
// +1, while a data bit of 1 means the offset is +3.  On the other
// hand, in a mode 5 packet, a data bit of 0 means the offset is -1
// and a data bit of 1 means the offset is +1.
//
// The 12-bit packet size presents some challenges given that the
// natural data sizes of 80x86 processors are 8, 16, and 32 bits.
// However, the challenge is justified because none of the natural
// data sizes provided encoding opportunites as good.  The
// alternatives to the scheme I chose were to either encode 6 samples
// in 8 bits, or 12 samples in 16 bits.  6 samples in 8 bits leaves
// only 2 bits to assign the mode, which is inadequate.  12 bits in 16
// samples leaves plenty of mode bits (4) but then we are committed to
// the same mode for 12 bits, which I felt is too long for the voice
// data I examined.  So I chose the 9-in-12 "sweet spot" granularity
// as the basis for the encoding strategy.
//
// The 12-bit packets are encoded in pairs; each pair occupies 3
// bytes.  If all packets were nominal packets, a single 3-byte
// "packet pair" data structure could be used, and such a structure is
// in fact used when nominal packets are back-to-back.  However, all
// the samples in a typical voice sample buffer cannot be encoded
// using nominal packets, so other encoding schemes are used.  The
// 3 mode bits available in the 9-in-12 scheme allow us 8 modes.
// Modes 1 through 6 are used for nominal packets, while mode 0 is an
// escape to either run-length or high-frequency mode, and mode 7 is an
// escape to medium-frequency mode. 
//
// The data structures used for run-length (RL), high-frequency (HF),
// and medium-frequency (MF) modes are different depending on whether
// the previous packet is the first packet in a nominal packet pair
// (case 0), or is any of the following (case 1):
//   - the second packet in a nominal packet pair 
//   - a run-length, high-frequency, or medium-frequency packet
//   - the first packet in the buffer
// In case 0, the RL, HF, or MF packet is 12 bits, while in case 1 it
// is 16 bits (that is, we don't use 12 bit encoding for these modes
// unless we are forced to, because 16 bit packets are better all the
// way around for RL, HF, and MF modes.  Both the encoder and decoder
// maintain knowlege of case 0 vs. case 1 as a state variable called
// "packetPos".
//
// The only identical-value runs that occur frequently enough to be
// worth worrying about have a value of 0 (silence), so run length
// encoding uses 0 as the implicit data value.
//
// In high-frequency mode each sample is *not* relative to the
// previous sample (as it is in the nominal modes).  Instead it is an
// absolute value that is multiplied by a multiplier that is part of
// the HF packet.
//
// In medium-frequency mode, a sequence of 4 samples in the original
// data is approximated by a straight line.  This mode can also encode
// low-frequency data reasonably well, but not as well as the nominal
// modes, so the latter are used where possible.  However, medium
// frequency mode can be used when the total rise (or fall) of 4
// samples is up to 32, whereas the best total rise (or fall) the
// nominal modes can achieve over 4 samples is 3*4 = 12.
//
// Finally, there is a special "literal" mode that is used only for
// the very first sample in the buffer (which is implicitly literal
// mode) and the last few samples (signaled by a special "run length"
// packet that has a length of 0).  It adds a very slight inefficiency
// (since it does no compression) but it greatly simplifies dealing
// with the edge cases, which would otherwise be problematic.
//
//////////////////////////////////////////////////////////////////////////////
//
// Some details of the encoding scheme are summarized below:
//
// Encoding Modes
// --------------
// Mode number   Description
//        0      run-length or high-frequency mode
//        1       0, +1
//        2       0, -1
//        3      +1, +3
//        4      -1, -3
//        5      +1, -1
//        6      +2, -2
//        7      medium-frequency mode
//
// Packet Layout
// -------------
// Run-length/high-frequency in first packet of packet pair (case 1)
// 0000 nnnn nnnn nnnn -- run length, n = length of run (1st nibble low-order)
// 0001 mmmm xxxx yyyy -- high frequency, m = multiplier
//                        x, y and succeeding nibbles until 0 byte
//                            = absolute sample values
//
// Run-length/high-frequency in second packet of packet pair (case 0)
// ---- ---- ---- 0000 nnnn nnnn -- run length, n = length of run
// ---- ---- ---- 0001 mmmm xxxx -- high frequency, m = multiplier
//                                  x, y and succeeding nibbles until 0 byte
//                                      = absolute sample values
//
// mmmm = 0 is not a useful value so we can use it as an escape indicator for
// other modes
//
//////////////////////////////////////////////////////////////////////////////
//
// Misc notes
// 
// CODEC_DEMO
// ----
// CODEC_DEMO is a compile-time flag that adds/alters code for use with the demo/
// experiment program.  If CODEC_DEMO is not defined, the codec routines will be
// compiled for general-purpose use outside of the demo/experiment program.
// (Like in a GAME maybe...what a NOVEL concept!)
//
// Packing Problems
// ----------------
// Even with #pragma pack(1), sizeof returns 4 for 3 byte structures.  In
// some places in the code, structure sizes had to be hardwired to get around
// this problem.


///////////////////////////////////////////////////////////////////////////////
// Data structures

// Not sure if there is a point in using unions somehow to make one big struct;
// for now just forget it and cast to totally different structs which is
// theoretically evil but is not really all that dangerous in practice...

// disable compiler padding of structures
#pragma pack(push, packet_declarations)
#pragma pack(1)

// most general notion of a packet pair
struct t_PacketPair
{
    unsigned long Mode1   : 3;
    unsigned long Mode1Ex : 1;
    unsigned long Data1   : 8;
    unsigned long Mode0   : 3;
    unsigned long Mode0Ex : 1;
    unsigned long Data0   : 8;
};

// nominal packet pair
struct t_PacketPairNom
{
    unsigned long Mode1 : 3;
    unsigned long Data1 : 9;
    unsigned long Mode0 : 3;
    unsigned long Data0 : 9;
};

// run-length packet, case 1 
struct t_PacketRL1
{
    unsigned short Mode   : 3;
    unsigned short ModeEx : 1;  // extra mode bit to distinguish RL & HF
    unsigned short Length : 12;
};

// high-frequency packet, case 1
struct t_PacketHF1
{
    unsigned short Mode   : 3;
    unsigned short ModeEx : 1; // extra mode bit to distinguish RL & HF
	unsigned short Table  : 3; // lookup table number
	unsigned short Data2  : 3; // absolute sample data
	unsigned short Data1  : 3; // absolute sample data
	unsigned short Data0  : 3; // absolute sample data
};

// high-frequency data packet, only used immediately after an HF packet, or
// after another HF data packet
struct t_PacketHFData
{
	unsigned short Table : 1; // 1 ==> DataT is lookup table number
	unsigned short Data3 : 3; // absolute sample data
	unsigned short Data2 : 3; // absolute sample data
	unsigned short Data1 : 3; // absolute sample data
	unsigned short Data0 : 3; // absolute sample data
	unsigned short DataT : 3; // absolute sample data or lookup table number
};

// run-length packet, case 0
struct t_PacketRL0
{
    unsigned long Mode1   : 3; // mode of previous packet in pair
    unsigned long Data1   : 9; // data of previous packet in pair
    unsigned long Mode0   : 3; // mode of this packet (always 0)
    unsigned long Mode0Ex : 1; // extra bit to distinguish RL & HF (always 0)
    unsigned long Length  : 8; // length of run
};

// high-frequency packet, case 0
struct t_PacketHF0
{
    unsigned long Mode1   : 3; // mode of previous packet in pair
    unsigned long Data1   : 9; // data of previous packet in pair
    unsigned long Mode0   : 3; // mode of this packet (always 0)
    unsigned long Mode0Ex : 1; // extra bit to distinguish RL & HF (always 1)
	unsigned long Table   : 1; // 1 ==> DataT is lookup table number
	unsigned long Data0   : 3; // absolute sample data
	unsigned long DataT   : 3; // absolute sample data or lookup table number
	unsigned long Unused  : 1;
};

// medium-frequency packet, case 1
struct t_PacketMF1
{
    unsigned short Mode  : 3; // mode of this packet (always 7)
    unsigned short Mult  : 1; // 0 ==> mult data by 1, 1 ==> mult data by 2
    short          Data1 : 6; // total rise or fall over current 4 samples
    short          Data0 : 6; // total rise or fall over next 4 samples
};

// medium-frequency packet, case 0
struct t_PacketMF0
{
    unsigned long  Mode1 : 3; // mode of previous packet in pair
    unsigned long  Data1 : 9; // data of previous packet in pair
    unsigned long  Mode0 : 3; // mode of this packet (always 7)
    unsigned long  Mult  : 1; // 0 ==> mult data by 1, 1 ==> mult data by 2
    long           DataX : 2; // not currently used
    long           Data0 : 6; // total rise or fall over next 4 samples
};

// restore state of compiler padding of structures
#pragma pack(pop, packet_declarations)




////////////////////////////////////////////////////////////////////////////////
// Constants, enums, and data tables
//
// These are not all delclared "const", but all are constant after the code is
// initialized.

// For the second packet of a packet pair, the breakeven point for run length 
// vs. normal encoding is 9 bits, because in that case we are using exactly
// the packet in both cases (we resolve the tie in favor of RL because it's
// encoding and decoding is faster).  For the first packet, RL uses a minimum
// of 16 bits whereas normal encoding commits us to 12 bits, so the breakeven
// point is with somewhat longer runs.  Using the compression ratio as the 
// figure of merit, breakeven is at x/16 = 9/12, or x = 12 bits. 
const int MIN_RUN_LEN[2] = { 9, 12 };

const int ZERO = 128; // the value of silence ;)

// some quality-of-service tuning parameters
int BIG_MOVE2;  // useful values 4..32
int FUDGE;      // useful values 1..5

// some tuning parameters to help decide which mode to use when
const int BIG_MOVE = 8;
const int SMALL_MOVE1 = 2;
const int SMALL_MOVE2 = 2;
const int SMALL_MOVE3 = 2;

// enum DecodeState { e_dsFirst, e_dsSecond };
enum EncodeMode 
{ 
    e_emRL_HF = 0, // run-length/high-frequency
    e_emZ_P1  = 1, // nominal:  0 or +1
    e_em0_N1  = 2, // nominal:  0 or -1
    e_emP1_P3 = 3, // nominal: +1 or +3
    e_emN1_N3 = 4, // nominal: -1 or -3
    e_emP1_N1 = 5, // nominal: +1 or -1
    e_emP2_N2 = 6, // nominal: +2 or -2
    e_emMF    = 7  // medium-frequency
};

// offsets for modes 1 through 6 (bogus first entry)
const int Deltas[7][2] = 
{
    {  0,  0 },
    {  0, +1 },
    { -1,  0 },
    { +1, +3 },
    { -3, -1 },
    { -1, +1 },
    { -2, +2 }
};

double Log2[256]; // lookup table for log-base-2 (i.e. log(x) / log(2))


// HF encoding table
#if 0
unsigned int EncTable[8][256] =
{
	{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
		2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  

		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  
        5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  
		6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
	},
	{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  

		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  
        5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  
		6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  
		7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
	},
};
#else
unsigned int EncTable[8][256];
#endif

// HF decoding table
int DecTable[8][8] =
{
	{ -12,  -6,  -3,  -1,  1,  3,  6, 12 },
	{ -24, -12,  -6,  -3,  3,  6, 12, 24 },
	{ -36, -18,  -9,  -5,  5,  9, 18, 36 },
	{ -48, -24, -12,  -6,  6, 12, 24, 48 },
	{ -60, -30, -15,  -7,  7, 15, 30, 60 },
	{ -72, -36, -18,  -9,  9, 18, 36, 72 },
	{ -84, -42, -21, -10, 10, 21, 42, 84 },
	{ -96, -48, -24, -12, 12, 24, 48, 96 }
};


///////////////////////////////////////////////////////////////////////////////
// Decoder

// Decode1 is the main decoder entry point.
// Input: 
//     bufIn    - encoded data to decode
//     bufOut   - empty buffer in which to place decoded data
//     size     - size of bufIn in bytes
//     sizeOut  - size of bufOut in bytes
// Output:
//     bufOut   - decoded data written here
//     returns  - nothing
//
static void Decode1(t_Sample* bufIn, t_Sample* bufOut, int size, int sizeOut)
{
    unsigned int mode, modeEx;
    BOOL packetPos = 1; // 1 = first packet of packet pair, 0 = second
    t_Sample* bufEnd = bufIn + size - 1;
    t_Sample* bufOutEnd = bufOut + sizeOut - 1;
    t_Sample* p = bufIn;  // current position in input buffer
    t_Sample* q = bufOut; // current position in output buffer

    *q++ = *p++; // first sample is in literal mode

    // Main decoding loop.  Look at the mode of the packet and call the
    // appropriate decoder.
    while (p <= bufEnd && q <= bufOutEnd)
    {
        if (packetPos)
        {
            mode   = ((t_PacketPair*)p)->Mode1;
            modeEx = ((t_PacketPair*)p)->Mode1Ex;
        }
        else
        {
            mode   = ((t_PacketPair*)p)->Mode0;
            modeEx = ((t_PacketPair*)p)->Mode0Ex;
        }
        switch (mode)
        {
            case e_emRL_HF:
                if (modeEx) // modeEx differentiates HF vs RL modes
                    DecodeHF(packetPos, p, q, bufOutEnd);
                else
                    DecodeRL(packetPos, p, q, bufEnd, bufOutEnd);
                packetPos = 1;
                break;

            case e_emZ_P1: case e_em0_N1: case e_emP1_P3: case e_emN1_N3: 
            case e_emP1_N1: case e_emP2_N2: 
                DecodeNom(packetPos, p, q, mode, bufOutEnd);
                packetPos = !packetPos;
                break;

            case e_emMF:
                DecodeMF(packetPos, p, q, bufOutEnd);
                packetPos = 1;
                break;
        }
    }

    //assert(p == bufEnd && q == bufOutEnd);

  #if defined(CODEC_DEMO)
    // Do some extra error checking in CODEC_DEMO mode.
    if (abs(bufEnd - p) > 1 || abs(bufOutEnd - q) > 1)
    {
        int leftIn = bufEnd - p,
            leftOut = bufOutEnd - q;
        char str[80];
        sprintf(str, "%d bytes left in source, %d bytes left in dest", 
                leftIn, leftOut);
        AfxMessageBox(str);
        
    }
  #endif
}

// Run-length decoder.  Very straightforward.
static void DecodeRL(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
                     t_Sample* bufEnd, t_Sample* bufOutEnd)
{
    int len;
    if (packetPos)
    {
        len = ((t_PacketRL1*)p)->Length;
        p += sizeof(t_PacketRL1);
    }
    else
    {
        len = ((t_PacketRL0*)p)->Length;
        p += 3; //sizeof(t_PacketRL0); // see "Packing Problems" comment above
    }

	// q + len-1 is where the last sample in this run will be written
	if (q + len-1 > bufOutEnd)
	{
		q = bufOutEnd + 1; // cause decoder to immediately abort
		return;
	}

    for (int i = 0; i < len; i++)
        *q++ = ZERO;

    if (len == 0) // check for literal-mode marker near end of data
    {
        // remaining data is in literal mode
        for (int remaining = *p++; remaining > 0 && q<=bufOutEnd; remaining--)
            *q++ = *p++;
//        assert(p == bufEnd+1);
    }
}

// High-frequency decoder.  Each sample is the data value for that sample
// multiplied by the current multiplier.
static void DecodeHF(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
					 t_Sample* bufOutEnd)
{
    static unsigned int table = 1;
    int data;
    // t_Sample* pstart = p;

	if (q > bufOutEnd - 3)
	{
		q = bufOutEnd + 1; // cause decoder to immediately abort
		return;
	}

    if (packetPos)
    {
        table = ((t_PacketHF1*)p)->Table;
        data = ((t_PacketHF1*)p)->Data2; 
        *q++ = t_Sample(DecTable[table][data] + ZERO);
        data = ((t_PacketHF1*)p)->Data1;
        *q++ = t_Sample(DecTable[table][data] + ZERO);
        data = ((t_PacketHF1*)p)->Data0;
        *q++ = t_Sample(DecTable[table][data] + ZERO);
        p += sizeof(t_PacketHF1);
    }
    else
    {
		if (((t_PacketHF0*)p)->Table == 1)
			table = ((t_PacketHF0*)p)->DataT;
        data = ((t_PacketHF0*)p)->Data0;
        *q++ = t_Sample(DecTable[table][data] + ZERO);
		if (((t_PacketHF0*)p)->Table == 0)
		{
			data = ((t_PacketHF0*)p)->DataT;
			*q++ = t_Sample(DecTable[table][data] + ZERO);
		}
        p += 3; //sizeof(t_PacketHF0); // see "Packing Problems" comment above
    }

    while (*(char*)p != 0)
    {
		if (q > bufOutEnd - 5)
		{
			q = bufOutEnd + 1; // cause decoder to immediately abort
			return;
		}
		if (((t_PacketHFData*)p)->Table == 1)
			table = ((t_PacketHFData*)p)->DataT;
        data = ((t_PacketHFData*)p)->Data3;
		*q++ = t_Sample(DecTable[table][data] + ZERO);
        data = ((t_PacketHFData*)p)->Data2;
		*q++ = t_Sample(DecTable[table][data] + ZERO);
        data = ((t_PacketHFData*)p)->Data1;
		*q++ = t_Sample(DecTable[table][data] + ZERO);
        data = ((t_PacketHFData*)p)->Data0;
		*q++ = t_Sample(DecTable[table][data] + ZERO);
		if (((t_PacketHFData*)p)->Table == 0)
		{
			data = ((t_PacketHFData*)p)->DataT;
			*q++ = t_Sample(DecTable[table][data] + ZERO);
		}
        p += sizeof(t_PacketHFData);
    }

    p += sizeof(char); //sizeof(t_PacketHFData); // to cover final "0" packet
}

// Nominal packet decoder.  Each sample is equal to the previous sample
// +/- an offset.
static void DecodeNom(BOOL packetPos, t_Sample*& p, t_Sample*& q,
                      unsigned int mode, t_Sample* bufOutEnd)
{
    unsigned int data;
    t_Sample delta0 = t_Sample(Deltas[mode][0]), // offset for "0" bits
             delta1 = t_Sample(Deltas[mode][1]), // offset for "1" bits
             last = *(q-1);

	if (q > bufOutEnd - 9)
	{
		q = bufOutEnd + 1;
		return;
	}

    if (packetPos)
    {
        data = ((t_PacketPairNom*)p)->Data1;
    }
    else
    {
        data = ((t_PacketPairNom*)p)->Data0;
        p += 3; // sizeof(t_PacketPairNom);
    }
    
    for (int i = 0; i < 9; i++)
    {
        if (data & 0x100)
		{
			last = t_Sample(last + delta1); // using += causes level 4 warning
            *q++ = last;
		}
        else
		{
			last = t_Sample(last + delta0); // using += causes level 4 warning
            *q++ = last;
		}
        data <<= 1;
    }
}

// Medium-frequency decoder.  Uses a straight line to approximate 4 
// consecutive samples.
static void DecodeMF(BOOL packetPos, t_Sample*& p, t_Sample*& q, 
					 t_Sample* bufOutEnd)
{
    unsigned int mult, data;
    t_Sample level = *(q-1);

	if (q > bufOutEnd - 4)
	{
		q = bufOutEnd + 1;
		return;
	}

    if (packetPos)
    {
        mult = ((t_PacketMF1*)p)->Mult; // currently unused
        data = ((t_PacketMF1*)p)->Data1; 
        // Make each of the 4 points is computed in a way equivalent to that
        // used in the encoder.
		                   level = t_Sample(level + data / 4);  *q++ = level;
        data -= data / 4;  level = t_Sample(level + data / 3);  *q++ = level;
        data -= data / 3;  level = t_Sample(level + data / 2);  *q++ = level;
        data -= data / 2;  level = t_Sample(level + data    );  *q++ = level;

        data = ((t_PacketMF1*)p)->Data0; 

        p += sizeof(t_PacketMF1);
    }
    else
    {
        mult = ((t_PacketMF0*)p)->Mult; // currently unused
        data = ((t_PacketMF0*)p)->Data0;

        p += sizeof(t_PacketMF0);
    }
                       level = t_Sample(level + data / 4);  *q++ = level;
    data -= data / 4;  level = t_Sample(level + data / 3);  *q++ = level;
    data -= data / 3;  level = t_Sample(level + data / 2);  *q++ = level;
    data -= data / 2;  level = t_Sample(level + data    );  *q++ = level;
}



///////////////////////////////////////////////////////////////////////////////
// Encoder

int QualityOfService;

// InitEncoder1 must be called once to initialize the encoder.  May safely be
// called again to change tuning parameters.
// Input: 
//     QoS      - Quality of Service: 1..10, 1 = highest compression/lowest
//                quality, 10 = worst compression/best quality
// Output:
//     initializes global variables
//
static void InitEncoder1(int QoS)
{
	int i, j, table, in, ip, vn, vp;
	QualityOfService = QoS;
    // There is nothing magic about the folowing 2 formulas, they're just one
    // way to translate from a semantic notion of "quality of service" to
    // reasonable values of the tuning parameters.  Each parameter is
    // orthogonal to the others, so the only reason to tie them to a single
    // input parameter is simplicity.
    BIG_MOVE2 = 8 + (10 - QoS) * 2;
    FUDGE = (12 - QoS) / 2;
    
    // initilize log-base-2 lookup table
    for (i = 1; i < 256; i++)
        Log2[i] = log((double)i) / log(2.0);

	for (table = 7; table >= 0; table--)
	{
		vn = 3, vp = 4;
		in = ZERO-1, ip = ZERO;
		for (i = 0; i < 16 * (table+1)/8.0; i++)
			EncTable[table][in--] = vn,  EncTable[table][ip++] = vp;

		for (j = 1; j < 4; j++)
		{
			vn--, vp++;
			for (i = 0; i < pow(2.0, 3.0+j) * (table+1)/8.0; i++)
				EncTable[table][in--] = vn,  EncTable[table][ip++] = vp;
		}
		while (in >= 0)
			EncTable[table][in--] = vn,  EncTable[table][ip++] = vp;
	}
	table = 0;
#if 0
	for (table = 0; table < 8; table++)
        for (i = 0; i < 256; i++)
		{
			s = abs(i - ZERO);
			if (s < 128*(table+1)/8.0)
				EncTable[table][i] = (unsigned int)log(s*8.0/(table+1)) 
					                 / log(2.0);
			else
				EncTable[table][i] = (unsigned int)log(128*8.0/(table+1));
    }
    
	for (table = 0; table < 8; table++)
		for (i = 0; i < 8; i++)
			DecTable[table][i] = pow(2.0, i);
#endif

}




// Encode1 is the main Encoder entry point.
// Input: 
//     bufIn    - encoded data to decode
//     bufOut   - empty buffer in which to place decoded data
//     size     - size of bufIn in bytes
//     sizeOut  - size of bufOut in bytes
// Output:
//     bufOut   - encoded data written here
//     returns  - number of bytes written to bufOut
//
#if defined(CODEC_DEMO)
#define EXTRA_CODEC_DEMO_ARGS4 , levels, modes, samples, storage
static int Encode1(t_Sample* bufIn, t_Sample* bufOut, int size, int sizeOut,
                   t_Sample* levels, int* modes, int samples[9], int storage[9])
#else
#define EXTRA_CODEC_DEMO_ARGS4
static int Encode1(t_Sample* bufIn, t_Sample* bufOut, int size, int sizeOut)
#endif
{
    const int CUTOFF = 4;
    t_Sample *in = bufIn,   // current position in input buffer
             *out = bufOut, // current position in output buffer
             *p,            // temporary pointer 
             *end = bufIn + size,        // end of input buffer
             *endOut = bufOut + sizeOut; // end of output buffer
    int i,     // loop counter
        level, // current "true" sample value
        s0,    // first of two samples
        s1,    // second of two samples
        smin, smax,         // min and max of current set of samples
        hist[CUTOFF*2 + 1], // histogram of sample-to-sample deltas
        upMove, downMove;   // total up/down deltas
    BOOL packetPos = 1; // 1 = first packet of packet pair, 0 = second
	int tableNum = 0;


    assert(sizeof(t_PacketRL1) == 2);
    assert(sizeof(t_PacketHF1) == 2);
    assert(sizeof(t_PacketHFData) == 2);
    assert(sizeof(t_PacketHF0) == 4);

    // First byte in encoded data is unencoded (literal mode) initial level.
    level = *out++ = *in++; 

  #if defined(CODEC_DEMO)
    // Remove *levels++ to make level 1 pixel offset.
    *levels++ = t_Sample(level);
    // For statistics, pretend that literal mode is high-frequency mode. 
    *modes++ = 8;
    samples[8] += 1;  storage[8] += 2*sizeof(t_Sample);
  #endif

    // Main encoding loop.  General strategy is to compute a histogram and
    // some other stats about the next 9 samples, then decide which is the
    // best encoding mode (that's the hard part), and finally encode the
    // data.  Note that if we decide on run-length or high-frequency mode,
    // we may encode more than 9 samples before going around this loop
    // again.
    while (in < end && out < endOut)
    {
		// Use current level rather than last sample for histogram etc. 
		// because level is where we are really starting from.  In the case
		// where we are starting with significant error (e.g. just coming out
		// of HF mode), we may very well pick a different mode starting from
		// level than we would starting from the last sample.
        //smin = smax = s0 = *(in-1);
        smin = smax = s0 = level;

        //int sstart = (s0 + *in) / 2,
        //  send = (in[7] + in[8]) / 2;

        // Compute histogram.
        // Do 9 bits at a time, ensuring we don't go off end of bufIn.
        for (i = 0; i < CUTOFF*2 + 1; i++)
            hist[i] = 0;
        upMove = downMove = 0;
        for (i = 0; i < 9 && in+i < end; i++)
        {
            s1 = in[i];
            if (s1 < smin) smin = s1;
            if (s1 > smax) smax = s1;
            int diff = s1 - s0;
            if (diff < 0) downMove -= diff;
            if (diff > 0) upMove += diff;
            if (diff > CUTOFF) diff = CUTOFF;
            if (diff < -CUTOFF) diff = -CUTOFF;
            hist[diff+CUTOFF]++;
            s0 = s1;
        }
        
        // it's faster to do this in the loop above...
        //for (i = 0, downMove = 0; i < CUTOFF; i++)
        //  downMove += hist[i] * (CUTOFF-i);
        //for (i = CUTOFF+1, upMove = 0; i < CUTOFF*2 + 1; i++)
        //  upMove += hist[i] * (i-CUTOFF);


        short delta = short(in[3] - level); // for medium-frequency mode

        // First check if we need to switch to literal mode.
        if (in+9 >= end)
        {
            // We are near the end of the data.  Put marker indicating
            // the final mode switch to literal mode.  The marker is a
            // run-length mode packet of length 0.  Literal mode
            // encoding is a single byte indicating the number of
            // literal mode samples followed by the original samples.
            if (packetPos)
            {
                t_PacketRL1 packet;
                packet.Mode   = e_emRL_HF;
                packet.ModeEx = 0;
                packet.Length = 0;
                *(t_PacketRL1*)out = packet;
                out += sizeof(t_PacketRL1);
            }
            else
            {
                t_PacketRL0 packet = *(t_PacketRL0*)out;
                packet.Mode0   = e_emRL_HF;
                packet.Mode0Ex = 0;
                packet.Length = 0;
                *(t_PacketRL0*)out = packet;
                out += 3; //sizeof(t_PacketRL0); // see "Packing Problems"
            }

            i = *out++ = t_Sample(end - in);

          #if defined(CODEC_DEMO)
            int remaining = i;
            while (i-- > 0)
                *levels++ = *out++ = *in++;

            // For statistical purposes, pretend that literal mode is high-
            // frequency mode since high frequency mode is the most similar
            // in terms of bits per sample.  In any case, literal mode will
            // have very little impact on any of the statistics.
            for (i = remaining; i > 0; i--) *modes++ = 8;
            samples[8] += remaining;  storage[8] += 2*remaining;
          #else
            while (i-- > 0)
                *out++ = *in++;
          #endif
        }
        // Check if low frequency mode applies; if so, use it because it
        // is the most efficient.
        else if (upMove <= SMALL_MOVE3 && downMove <= SMALL_MOVE3)
        { // low frequency mode
            BOOL runLengthMode = FALSE;
            if (hist[CUTOFF] == 9)
            { // possible 0-run starting; check ahead

                // packet1 can handle runs of 2^12-1 = 4095 samples
                // packet0 can handle runs of 2^8-1 = 255 samples
                int len, maxLen = packetPos ? 4095 : 255;
                for (p = &in[9], len = 9; p < end && len < maxLen; p++, len++)
                    if (*p != ZERO)
                        break;

                if (len > MIN_RUN_LEN[packetPos])
                { // encode region in-to-p as run-length
                    runLengthMode = TRUE;
                    if (packetPos)
                    {
                        t_PacketRL1 packet;
                        packet.Mode = e_emRL_HF;
                        packet.ModeEx = 0;
                        packet.Length = unsigned short(len);
                        *(t_PacketRL1*)out = packet;
                        out += sizeof packet;
                        // packetPos remains at 1
                      #if defined(CODEC_DEMO)
                        samples[0] += len;  storage[0] += 2*sizeof packet;
                      #endif
                    }
                    else
                    {
                        t_PacketRL0 packet = *(t_PacketRL0*)out;
                        packet.Mode0 = e_emRL_HF;
                        packet.Mode0Ex = 0;
                        packet.Length = len;
                        *(t_PacketRL0*)out = packet;
                        out += 3; // sizeof packet;
                        packetPos = 1;
                      #if defined(CODEC_DEMO)
                        samples[0] += len;  storage[0] += 3; // sizeof packet;
                      #endif
                    }
                    in += len;

                  #if defined(CODEC_DEMO)
                    for (int j = 0; j < len; j++)
                        *levels++ = ZERO, *modes++ = 0;
                  #endif
                }
            }
            if (!runLengthMode)
            { // normal low-frequency mode
                //!!! Need to implement a low-frequency mode for real.  In the
                // meantime, use the appropriate nominal mode in its place.
                if (upMove > 0 && downMove > 0)
                { 
                    out += DoEncode(e_emP1_N1, packetPos, in, out, level 
                                    EXTRA_CODEC_DEMO_ARGS4);
                }
                else if (upMove > 0)
                { 
                    out += DoEncode(e_emZ_P1, packetPos, in, out, level 
                                    EXTRA_CODEC_DEMO_ARGS4);
                }
                else
                { 
                    out += DoEncode(e_em0_N1, packetPos, in, out, level 
                                    EXTRA_CODEC_DEMO_ARGS4);
                }
            }
        }
        // Check if nominal modes apply; if so, use one of them.
        // Given 9 bits with a maximum per-bit delta of 3, we can keep up
        // with an end-to-end delta of 3 * 9 in nominal mode, plus we add
        // in a little fudge factor because we would really rather avoid
        // medium or high-frequency modes if possible.
        //else if (hist[CUTOFF*2] <= 2 && hist[0] <= 2 && 
        //       upMove < 3 * 9 + 5 && downMove < 3 * 9 + 5)
        else if (hist[CUTOFF*2] + hist[0] <= 2 && 
                 upMove < 3 * 9 + FUDGE && downMove < 3 * 9 + FUDGE)
        {
            if (upMove >= BIG_MOVE)
            { // lot of up movement, use mode 3 or 6
                //  mode 3 unless nontrivial down move
                if (downMove <= SMALL_MOVE1) 
                    out += DoEncode(e_emP1_P3, packetPos, in, out, level 
                                    EXTRA_CODEC_DEMO_ARGS4);
                else
                    out += DoEncode(e_emP2_N2, packetPos, in, out, 
                                    level EXTRA_CODEC_DEMO_ARGS4);
            }
            else if (downMove >= BIG_MOVE)
            { // lot of down movement, use mode 4 or 6
                if (upMove <= SMALL_MOVE1) //  mode 4 unless nontrivial up move
                    out += DoEncode(e_emN1_N3, packetPos, in, out, level
                                    EXTRA_CODEC_DEMO_ARGS4);
                else
                    out += DoEncode(e_emP2_N2, packetPos, in, out, level 
                                    EXTRA_CODEC_DEMO_ARGS4);
            }
            else if (upMove >= SMALL_MOVE2 && downMove >= SMALL_MOVE2)
            { 
                out += DoEncode(e_emP1_N1, packetPos, in, out, level 
                                EXTRA_CODEC_DEMO_ARGS4);
            }
            else if (upMove >= SMALL_MOVE2)
            { 
                out += DoEncode(e_emZ_P1, packetPos, in, out, level 
                                EXTRA_CODEC_DEMO_ARGS4);
            }
            else
            { 
                out += DoEncode(e_em0_N1, packetPos, in, out, level 
                                EXTRA_CODEC_DEMO_ARGS4);
            }
        }
        // Last chance to avoid dreaded high-frequency mode.  In medium-
        // frequency mode, a sequence of 4 samples is encoded as a straight-
        // line approximation.  There are 6 bits available to encode the
        // total rise/fall of the line, so we can track a total rise of 
        // +(2^5-1) = +31 and a total fall of -(2^5) = -32.
        //else if (delta <= 31 && delta >= -32)
        else if ((upMove < BIG_MOVE2 || downMove < BIG_MOVE2) 
                 && delta <= 31 && delta >= -32)
        { // medium frequency mode
          #if defined(CODEC_DEMO)
            int temp1;
          #endif
            int temp2;
            in += 4;
            if (packetPos)
            {
                t_PacketMF1 packet;
                packet.Mode = e_emMF;
                packet.Mult = 0; //!!! should implement .Mult
                packet.Data1 = delta;
                temp2 = level;
                level += packet.Data1;
                delta = short(in[3] - level);
                if (delta > 31)       packet.Data0 = 31;
                else if (delta < -32) packet.Data0 = -32;
                else                  packet.Data0 = delta;
                *(t_PacketMF1*)out = packet;
                out += sizeof packet;
                in += 4;

              #if defined(CODEC_DEMO)
                temp1 = packet.Data1; *levels++ = t_Sample(temp2 += temp1 / 4);
                temp1 -= temp1 / 4;   *levels++ = t_Sample(temp2 += temp1 / 3);
                temp1 -= temp1 / 3;   *levels++ = t_Sample(temp2 += temp1 / 2);
                temp1 -= temp1 / 2;   *levels++ = t_Sample(temp2 += temp1);
                assert(temp2 == level);
                temp1 = packet.Data0; *levels++ = t_Sample(temp2 += temp1 / 4);
                temp1 -= temp1 / 4;   *levels++ = t_Sample(temp2 += temp1 / 3);
                temp1 -= temp1 / 3;   *levels++ = t_Sample(temp2 += temp1 / 2);
                temp1 -= temp1 / 2;   *levels++ = t_Sample(temp2 += temp1);
                assert(temp2 == level + packet.Data0);

                for (i = 0; i < 8; i++) *modes++ = 7;
                samples[7] += 8;  storage[7] += 2*sizeof packet;
              #endif

                level += packet.Data0;
            }
            else
            {
                t_PacketMF0 packet = *(t_PacketMF0*)out;
                packet.Mode0 = e_emMF;
                packet.DataX = 0;
                packet.Data0 = delta;
                *(t_PacketMF0*)out = packet;
                out += sizeof packet;

              #if defined(CODEC_DEMO)
                temp2 = level;
                temp1 = packet.Data0; *levels++ = t_Sample(temp2 += temp1 / 4);
                temp1 -= temp1 / 4;   *levels++ = t_Sample(temp2 += temp1 / 3);
                temp1 -= temp1 / 3;   *levels++ = t_Sample(temp2 += temp1 / 2);
                temp1 -= temp1 / 2;   *levels++ = t_Sample(temp2 += temp1);
                assert(temp2 == level + packet.Data0);

                for (i = 0; i < 4; i++) *modes++ = 7;
                samples[7] += 4;  storage[7] += 3; // sizeof packet;
              #endif

                level += packet.Data0; 
            }
            packetPos = 1;
        }
        else // No choice but to use bit-gobbling high frequency mode.
        { // high frequency mode
            // need abs because smax is not guaranteed to be > ZERO and smin
            // is not guaranteed to be < ZERO
			int temp1 = abs((smax - ZERO) / 16), 
				temp2 = abs((ZERO - smin - 1) / 16);
			// "table" is our new preferred table
			int table = temp1 > temp2 ? temp1 : temp2;
			int data4, data3, data2, data1, data0, datat, val3, val2, val1, val0;
			int max;
			int finalPacketData;

            if (packetPos)
            {
                t_PacketHF1 packet;
                packet.Mode = e_emRL_HF;
                packet.ModeEx = 1;
				// HF1 packets include table number unconditionally
				packet.Table = unsigned short(tableNum = table);
				packet.Data2 = unsigned short(EncTable[table][*in++]);
				packet.Data1 = unsigned short(EncTable[table][*in++]);
				finalPacketData = 
				packet.Data0 = unsigned short(EncTable[table][*in]);
				data0 = *in++;
                *(t_PacketHF1*)out = packet;
                out += sizeof packet;

              #if defined(CODEC_DEMO)
				*levels++ = t_Sample(DecTable[table][packet.Data2] + ZERO);
				*levels++ = t_Sample(DecTable[table][packet.Data1] + ZERO);
				*levels++ = t_Sample(DecTable[table][packet.Data0] + ZERO);
                *modes++ = 8; 
				*modes++ = 8;
				*modes++ = 8;
				samples[8] += 3;  storage[8] += 2*sizeof packet;
              #endif
            }
            else
            {
                t_PacketHF0 packet = *(t_PacketHF0*)out;
                packet.Mode0 = e_emRL_HF;
                packet.Mode0Ex = 1;

				finalPacketData = packet.Data0 = EncTable[table][*in++];
				// "tableNum-1" in order to introduce a little hysteresis in 
				// going to a smaller table.  No such trick in the other 
				// direction since the consequences of using a table that is
				// too small is worse than using one that is to large.
				if (table > tableNum || table < tableNum-1)
				{
					packet.Table = 1;
					packet.DataT = tableNum = table;
				}
				else // use previous table
				{
					packet.Table = 0;
					finalPacketData = packet.DataT = EncTable[table = tableNum][*in++];
				}
                *(t_PacketHF0*)out = packet;
                out += 3; // sizeof packet;

              #if defined(CODEC_DEMO)
				*levels++ = t_Sample(DecTable[table][packet.Data0] + ZERO);
                *modes++ = 8;
                samples[8] += 1;  storage[8] += 3; // sizeof packet;
				if (packet.Table == 0)
				{
					*levels++ = t_Sample(DecTable[table][packet.DataT] + ZERO);
					*modes++ = 8;
					samples[8] += 1;
				}
              #endif
            }
            
            t_PacketHFData packet;
			for (;;)
			{
				if (in+5 >= end)
				{
                    *(char*)out = 0;
                    out += sizeof(char);
                  #if defined(CODEC_DEMO)
                    storage[8] += 2*sizeof(char);
                  #endif
				}
                data4 = *(in-1) - ZERO;
				data3 = *in++ - ZERO;
				data2 = *in++ - ZERO;
				data1 = *in++ - ZERO;
				data0 = *in++ - ZERO;
				datat = *in - ZERO;
				max = abs(data3) > abs(data2) ? abs(data3) : abs(data2);
				max = abs(data1) > max ? abs(data1) : max;
				max = abs(data0) > max ? abs(data0) : max;
				max = abs(datat) > max ? abs(datat) : max;
				table = (max-1) / 16;

				if (table > tableNum || table < tableNum-1)
				{
					packet.Table = 1;
					packet.DataT = unsigned short(table);
				}
				else // use previous table
				{
					packet.Table = 0;
					table = tableNum;
					packet.DataT = unsigned short(EncTable[table][datat+ZERO]);
					in++;
				}

				packet.Data3 = unsigned short(val3 = EncTable[table][data3+ZERO]);
				packet.Data2 = unsigned short(val2 = EncTable[table][data2+ZERO]);
				packet.Data1 = unsigned short(val1 = EncTable[table][data1+ZERO]);
				packet.Data0 = unsigned short(val0 = EncTable[table][data0+ZERO]);

				// break if the data is relatively smooth or if we encounter
				// data (two 0's) that would cause us to encode a packet that
				// looks like a HF termination byte
				if ((abs(data2 - data3) + abs(data1 - data2) + 
                     abs(data0 - data1) + abs(data3 - data4) <= 12) ||
                    (val3 == 0 && val2 == 0))
                {
                    *(char*)out = 0;
                    out += sizeof(char);
                  #if defined(CODEC_DEMO)
                    storage[8] += 2*sizeof(char);
                  #endif
					if (packet.Table == 1)
						in -= 4;
					else
						in -= 5;
                    break;
                }

				// now that we know for certain that this packet, will be 
				// used, set persistent state to indicate the last used
				// table & last encoded data value
				finalPacketData = packet.Table ? packet.Data0 : packet.DataT;
				tableNum = table;

                *(t_PacketHFData*)out = packet;
                out += sizeof packet;

              #if defined(CODEC_DEMO)
				*levels++ = t_Sample(DecTable[table][packet.Data3] + ZERO);
				*levels++ = t_Sample(DecTable[table][packet.Data2] + ZERO);
				*levels++ = t_Sample(DecTable[table][packet.Data1] + ZERO);
				*levels++ = t_Sample(DecTable[table][packet.Data0] + ZERO);
                *modes++ = 8; *modes++ = 8; *modes++ = 8; *modes++ = 8;
                samples[8] += 4;  storage[8] += 2*sizeof packet;
				if (packet.Table == 0)
				{
					*levels++ = t_Sample(DecTable[table][packet.DataT] + ZERO);
					*modes++ = 8;
					samples[8] += 1;
				}
              #endif
            }
            // be sure to set level here so it stays in sync
			level = DecTable[tableNum][finalPacketData] + ZERO;
            packetPos = 1;
        }
    }

    return out - bufOut; 
}

/*
#if defined(CODEC_DEMO)
static int ComputeNomData(t_Sample*& in, const int deltas[], int& level,
                          t_Sample*& levels)
#else
static int ComputeNomData(t_Sample*& in, const int deltas[], int& level)
#endif
{
    int data = 0;

    for (int i = 0; i < 9; i++, in++)
    {
        if (level+deltas[0] < *in)
        {
            data = (data << 1) | 1;
            level += deltas[1];
        }
        else
        {
            data <<= 1;
            level += deltas[0];
        }
        #if defined(CODEC_DEMO)
            *levels++ = t_Sample(level);
        #endif
    }
    return data;
}
*/

// ecx i
// eax temp
// edx level
// ebx deltas[0]
// edi deltas[1]
// esi in
// ebp data

#if defined(CODEC_DEMO)
static int ComputeNomDataF(t_Sample*& inp, const int deltas[], int& level,
                          t_Sample*& levels)
#else
static int ComputeNomDataF(t_Sample*& inp, const int deltas[], int& level)
#endif
{
    int data;
    __asm
    {
        push ebp
        mov esi, [inp]          // esi = &inp
        mov esi, [esi]          // esi = inp
        mov ebx, deltas[0]      // ebx = &deltas[0]
        mov edi, [ebx+4]        // edi = deltas[1]
        mov ebx, [ebx]          // ebx = deltas[0]
        mov edx, [level]        // edx = &level
        mov edx, [edx]          // edx = level
        mov ebp, 0

        mov eax, edx            // temp = level
        mov ecx, 9              // loop count = 9

      DO_9_SAMPLES:
        add eax, ebx            // temp += deltas[0]
        cmp al, byte ptr [esi]  // level+deltas[0] < *in ?
        rcl ebp, 1              // shift right bit into data
        jge SUM_GE_IN           // if (level+deltas[0] >= *in) goto SUM_GE_IN
        add edx, edi            // level+deltas[0] < *in -> level += deltas[1]
        mov eax, edx            // temp = level (for next iteration)
        jmp SUM_LT_IN
      SUM_GE_IN:
        mov edx, eax            // level = temp
      SUM_LT_IN:

      #if defined(CODEC_DEMO)
        push eax                // ran out of registers
        push ebp                // we will need the real ebp to get at levels
        mov ebp,[esp+8]         // get real (frame ptr) value of ebp
        mov eax, levels         // eax = &levels (implicitly uses ebp)
        //mov al, byte ptr ss:[levels]      
        // eax = &levels
        mov ebp, eax            // ebp = &levels
        mov eax, [eax]          // eax = levels
        mov [eax], dl           // *levels = level
        inc eax                 // eax = levels + 1
        mov [ebp], eax          // levels = eax
        pop ebp
        pop eax
      #endif
        inc esi
        loop DO_9_SAMPLES

        mov ebx, ebp
        pop ebp
        mov eax, [level]        // eax = &level
        mov [eax], edx          // level = edx
        mov eax, [inp]          // eax = &inp
        mov [eax], esi          // inp = esi
        mov [data], ebx
    }
    return data;
}

#define VERIFY_ASM

// Nominal packet encoder.  Each encoded sample is equal to the previous sample
// +/- an offset.  The mode has been decided by the routine that calls DoEncode
// so the mode is passed in.
#if defined(CODEC_DEMO)
#define EXTRA_CODEC_DEMO_ARGS1 , levels
static int DoEncode(int mode, BOOL& packetPos, t_Sample*& in, t_Sample*& out, 
                    int& level, t_Sample*& levels, int*& modes, 
                    int samples[9], int storage[9])
#else
#define EXTRA_CODEC_DEMO_ARGS1
static int DoEncode(int mode, BOOL& packetPos, t_Sample*& in, t_Sample*& out, 
                    int& level)
#endif
{
    t_PacketPairNom packet;
    int advanceOutput = 0;

    if (packetPos)
    {
        #if defined(VERIFY_ASM) && defined(CODEC_DEMO)
            t_Sample *inT1 = in, *inT2 = in, *levelsT = levels;
            int levelT1 = level, levelT2 = level;
        #endif
        packet.Mode1 = mode;
        packet.Data1 = ComputeNomDataF(in,Deltas[mode], level EXTRA_CODEC_DEMO_ARGS1);
        #if defined(VERIFY_ASM) && defined(CODEC_DEMO)
            levels = levelsT;
            unsigned int dataT = ComputeNomData(inT1, Deltas[mode], levelT1 EXTRA_CODEC_DEMO_ARGS1);
            if (dataT != packet.Data1)
            {
                inT1 = inT2, levelT1 = levelT2;
                levels = levelsT;
                dataT = ComputeNomData(inT1, Deltas[mode], levelT1 EXTRA_CODEC_DEMO_ARGS1);
                levels = levelsT;
                dataT = ComputeNomDataF(inT2, Deltas[mode], levelT2 EXTRA_CODEC_DEMO_ARGS1);
            }
        #endif
    }
    else
    {
        packet = *(t_PacketPairNom*)out;
        packet.Mode0 = mode;
        packet.Data0 = ComputeNomDataF(in,Deltas[mode], level EXTRA_CODEC_DEMO_ARGS1);
        advanceOutput = 3; // sizeof packet;
    }
    *(t_PacketPairNom*)out = packet;
    packetPos = !packetPos;

  #if defined(CODEC_DEMO)
    for (int i = 0; i < 9; i++) *modes++ = mode;
    samples[mode] += 9;  storage[mode] += 3; // sizeof packet;
  #endif

    return advanceOutput;
}


static void Smooth1(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    int i;
    bufOut[0] = bufIn[0];
    bufOut[size-1] = bufIn[size-1];
    for (i = 2; i < size; i++)
    {
        int s0 = bufIn[i-2], s1 = bufIn[i-1], s2 = bufIn[i];
        if (s0 == s2 && abs(s1 - s0) < 3)
            bufOut[i-1] = bufIn[i];
        else
            bufOut[i-1] = bufIn[i-1];
        if (abs(s0 - ZERO) < 2 && abs(s1 - ZERO) < 3)
            bufOut[i-2] = ZERO;
    }
}

// can operate in place
static double AutoGain1(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    register int i, s;
	int hist[17], sum, silenceSize = 0;
	const int TARGET_MAX = 32, BUCKET_SIZE = 3;
	double x;

	for (i = 0; i < 17; i++)
		hist[i] = 0;
	for (i = 0; i < size; i++)
	{
        s = abs(bufIn[i] - ZERO);
		hist[s >> BUCKET_SIZE]++;
		if (s == 0 || s == 1 || s == -1)
			silenceSize++;
	}
	// ignore silence data when computing significant_data_above_this
	for (i = 15, sum = hist[16]; i >= 0 && sum < ((size - silenceSize) >> 4);
		 i--)
		sum += hist[i];

	int significant_data_above_this = i << BUCKET_SIZE;

  #if LOGARITHMIC_GAIN
	if (significant_data_above_this > TARGET_MAX)
	{
		x = ((double)Log2[significant_data_above_this]) / TARGET_MAX;

		for (i = 0; i < size; i++)
		{
		    s = abs(bufIn[i] - ZERO);
		    s = int(Log2[s] / x);
		    bufOut[i] = bufIn[i] < ZERO ? t_Sample(ZERO - s) : t_Sample(s + ZERO);
		}
	}
	else
		x = 0.0;
	return x;
  #else
	// linearly scale data such that a value of significant_data_above_this
	// will become a value of TARGET_MAX
	if (significant_data_above_this > TARGET_MAX)
	{
		x = ((double)TARGET_MAX) / significant_data_above_this;
		for (i = 0; i < size; i++)
		{
		    s = abs(bufIn[i] - ZERO);
		    s = int(((double)s) * x + .5);
		    bufOut[i] = bufIn[i] < ZERO ? t_Sample(ZERO - s) : t_Sample(s + ZERO);
		}
		x  = 1 / x; // inverse for UnAutoGain() (no danger of x being 0)
	}
	else if (significant_data_above_this > 0)
		x = ((double)TARGET_MAX) / significant_data_above_this;
	else
		x = 4.0;

	// Do a little extra volume boost for low QoS situations.  There is 
	// nothing magic about the constants in the following, they're just
	// reasonable heuristic values.
	if (QualityOfService <= 3)
		x = (x * 6) / (QualityOfService+2);
	if (QualityOfService == 1)
		x *= 1.5;
	return x;
  #endif
}

// can operate in place
static void UnAutoGain1(t_Sample* bufIn, t_Sample* bufOut, int size, 
						double gain)
{
    register int i;
	if (gain == 0.0)
	{
		if (bufIn != bufOut)
			for (i = 0; i < size; i++)
				bufOut[i] = bufIn[i];
		return;
	}

    for (i = 0; i < size; i++)
    {
        int s = abs(bufIn[i] - ZERO);
      #if defined(LOGARITHMIC_GAIN)
        s = int(pow(2.0, s * gain)); //!!! could use some optimization
      #else
		s =  int(((double)s) * gain + .5);
      #endif
		if (s > 127) s = 127;
        bufOut[i] = bufIn[i] < ZERO ? t_Sample(ZERO - s) : t_Sample(s + ZERO);
    }
}

static void InitLowPassFilter(int QoS, double LPF_Coef[], 
							  char LPF_CoefTimesSample[LPF_NUM_POINTS][256])
{
	// QoS==10 ==> cutoff frequency is 5500Hz (filter is a noop)
	// QoS==1  ==> cutoff frequency is  550Hz
	int N = 19, K = 2 * QoS - 1, kshift = LPF_NUM_POINTS/2, k;
    for (k = 0; k < LPF_NUM_POINTS; k++)
    {
        if (k != kshift)
            LPF_Coef[k] = sin(PI * (k-kshift) * K / (double)N) / 
                        ( sin(PI * (k-kshift) / (double)N) * N );
        else
            LPF_Coef[k] = ((double)K) / N;
        for (int i = 0; i < 256; i++)
            LPF_CoefTimesSample[k][i] = (char)(LPF_Coef[k] * i + .5);
    }
}

// can now operate in place
static void LowPassFilter(t_Sample* bufIn, t_Sample* bufOut, int size,
						  char LPF_CoefTimesSample[LPF_NUM_POINTS][256])
{
    register int j;

  #if !defined(USE_LOWPASS_FILTER)
	if (bufIn != bufOut)
		for (j = 0; j < size; j++)
			bufOut[j] = bufIn[j];
	return;
  #endif

    for (j = LPF_NUM_POINTS-1; j < size; j++)
    {
        register int i, sum = 0;
        for (i = 0; i < LPF_NUM_POINTS; i++)
		{
			int temp1 = bufIn[ i + (j-(LPF_NUM_POINTS-1)) ]; //    0 -> 255
			temp1 -= ZERO;                                   // -128 -> 127
			// lookup based on abs value
			int temp2 = LPF_CoefTimesSample[i][abs(temp1)];   
            if (temp1 < 0)
				sum -= temp2;
			else
				sum += temp2;
		}

		// depending on the exact values of the coefficients, there is a
		// chance that sum may become slightly larger (or smaller) than the
		// maximum (mimimum) allowable values, so truncate it before it does
		// any damage
		if (sum >  127) sum =  127;
		if (sum < -128) sum = -128;

        bufOut[j-(LPF_NUM_POINTS-1)] = t_Sample(sum + ZERO);
    }

  #if 0
    //for (j = 0; j < LPF_NUM_POINTS-1; j++)
    //  bufOut[j] = 0;	// clear unused buffer entries

	// fade in the samples to avoid pops & clicks
    int level = bufOut[LPF_NUM_POINTS-1];
    for (j = LPF_NUM_POINTS-2; j >= 0; j--)
        bufOut[j] = level = ((level-ZERO) >> 1) + ZERO;
  #else
	// fade out the samples to avoid pops & clicks
    int level = bufOut[size - LPF_NUM_POINTS];
    for (j = size-(LPF_NUM_POINTS+1); j < size; j++)
		bufOut[j] = t_Sample(level = ((level-ZERO) >> 1) + ZERO);
  #endif
  #if defined(REMOVE_DC_BIAS)
	int sum, bias;
	// remove DC bias
	for (j = 0, sum = 0; j < size; j++)
		sum += bufOut[j] - ZERO;
	bias = sum / size;
	for (j = 0; j < size; j++)
		bufOut[j] -= bias;
  #endif
}

// can operate in place (i.e. bufIn == bufOut is ok)
static void SkipEveryOther(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    for (int i = 0; i < size; i += 2)
        bufOut[i/2] = bufIn[i];
}

static void InterpolateEveryOther(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    for (int i = 0; i < size; i++)
    {
        *bufOut++ = *bufIn;
        *bufOut++ = t_Sample((*bufIn + *(bufIn+1)) / 2);
        bufIn++;
    }
}


#if defined(USE_LPC10)
extern "C"
{
    #include "lpc10\ulaw.h" // it's the LAW
};
// can operate in place
static void ConvertToLPC10(t_Sample* bufIn, t_Sample* bufOut, int size)
{
#if 0
    unsigned short s;
    for (int i = 0; i < size; i++)
    {
        s = bufIn[i]-ZERO;
        s <<= 8;
        s >>= 3;
        bufOut[i] = s2u[s];
    }
#else

    for (int i = 0; i < size; i++)
        bufOut[i] = audio_c2u(bufIn[i]);
#endif
}

// can operate in place
static void ConvertFromLPC10(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    for (int i = 0; i < size; i++)
        bufOut[i] = t_Sample(audio_u2c(bufIn[i]));
}
#endif // defined(USE_LPC10)


//////////////////////////////////////////////////////////////////////////////
// Only unused junk below.  Being kept around for reference only.

#if 0
    for (i = 0; i < 256; i++)
    {
        s = abs(i - ZERO);
        if (s > 22)
            s = (log((double)s) / log(2.0)) * 5 + .5;
        s = i < ZERO ? ZERO - s : s + ZERO;
        unsigned char junk = s;
        if (junk > ZERO+35 || junk < ZERO-36)
            junk = 127;
    }

static void Log1(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    register int i, s;
    for (i = 0; i < size; i++)
    {
        s = abs(bufIn[i] - ZERO);
        if (s > 22)
            s = Log2[s] * 5.0 + .5;
        bufOut[i] = bufIn[i] < ZERO ? ZERO - s : s + ZERO;
        if (bufOut[i] > ZERO+35 || bufOut[i] < ZERO-36)
            bufOut[i] = 127;
    }
}

static void UnLog1(t_Sample* bufIn, t_Sample* bufOut, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        int s = abs(bufIn[i] - ZERO);
        
        if (s > 22)
            s = pow(2.0, s / 5.0); //!!! could use some optimization
        bufOut[i] = bufIn[i] < ZERO ? ZERO - s : s + ZERO;
    }
}

// from InitLowPassFilter():
    //int K = LPF_NUM_POINTS;
    //double cutoffFrequency = 1100.0*QoS,
    //       bandwidthPerBucket = cutoffFrequency / (K/2);
    //int N = (int)(11000.0 / bandwidthPerBucket), k,
    //    kshift = K/2;

    // cutoff fraction = cutoff freq / sample freq
    // ==> cutoff fraction = LPF_NUM_POINTS / N
    // ==> N = LPF_NUM_POINTS / cutoff fraction
    //int i, k, N = (int)(LPF_NUM_POINTS / ((1100.0*QoS) / 11000.0)), 
    //    kshift = LPF_NUM_POINTS/2;


#endif

