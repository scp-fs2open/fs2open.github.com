//                              Codec1.h
//
// Contains interface definition of VoiCTech (Voice Communications Technology)
// voice encoder/decoder.
//
// Written by Matthew F. Storch, Ph.D., copyright (c) 1998 Volition Inc.


//////////////////////////////////////////////////////////////////////////////
//
// Introduction to the VoiCTech encoder
// ------------------------------------
// The VoiCTech (short for Voice Communication Technology, pronounced
// "voice-tech") audio codec uses two separate algorithm suites: Codec1
// and LPC-10.  Codec1 does compression without using traditional signal 
// processing algorithms (expcept for a simple FIR low-pass filter).  As
// a result, it is extremely fast, and yields moderate-quality voice at a
// compression ratio of between 3-to-1 and 10-to-1, assuming 11KHz sampled
// data.  LPC-10 does significantly more analysis and is therefore slower
// (approximately 3 times slower than Codec1), but achieves substantially
// better compression (25-1) at a comparable quality.  (However, the
// *character* of the loss in quality is quite different.)
//
// The externally-callable interface for both algorithm suites is through
// a simple, generic front-end that is prototyped in this file.    
//
// codec1.cpp contains the implementions of both the generic interface and
// Codec1. LPC-10 is implemented in a set of files in the LPC10 subdirectory.
//
//////////////////////////////////////////////////////////////////////////////


// VoiCTech is only intended to work with 8-bit samples recorded at 11KHz.
// 8KHz should also work.
typedef unsigned char t_Sample;

// Algorithm suites.
enum t_Code { e_cCodec1, e_cLPC10 };

// One of these is passed with every buffer that is transmitted; it contains
// data on how to decode the buffer.
struct t_CodeInfo { t_Code Code; double Gain; };


// WARNING ***** DANGER ***** WARNING ***** DANGER ***** WARNING ***** DANGER
// If LPC-10 is used, all buffers must be a multiple of LFRAME (Frame length
// for LPC-10 codec) in length
#define LFRAME 180
// WARNING ***** DANGER ***** WARNING ***** DANGER ***** WARNING ***** DANGER


//////////////////////////////////////////////////////////////////////////////
// InitDecoder must be called once to initialize the decoder.
// Input: 
//     QoS      - Quality of Service: 1..10, 1 = highest compression/lowest
//                quality, 10 = worst compression/best quality (currently
//                *not* used by decoder)
//     tempBuf  - Temporary buffer that must be available until Encode() will
//                no longer be called.  Must be at least as large as the 
//                largest sizeIn that will be passed to Encode().
// Output:
//     initializes global variables
//
void InitDecoder(int QoS, t_Sample* tempBuf); 
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Decode is the main decoder entry point.
// Input:
//     codeInfo   - Information about how the buffer was encoded
//     bufIn      - Encoded data to decode
//     bufOut     - Empty buffer in which to place decoded data (should be
//                  at least decodeSize in length)
//     encodeSize - Size in bytes of encoded data in bufIn
//     decodeSize - Size data should be after decoding
// Output:
//     bufOut     - Decoded data written here
//     returns    - Nothing
//
void Decode(t_CodeInfo* codeInfo, t_Sample* bufIn, t_Sample* bufOut, 
            int encodeSize, int decodeSize);
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// InitEncoder must be called once to initialize the encoder.  May safely be
// called again to change tuning parameters.
// Input: 
//     code     - The algorithm that will be used to encode the data in
//                subsequent calls to Encode() 
//     QoS      - Quality of Service: 1..10, 1 = highest compression/lowest
//                quality, 10 = worst compression/best quality
//     tempBuf1 - Temporary buffer that must be available until Encode() will
//                no longer be called.  Must be at least as large as the 
//                largest sizeIn that will be passed to Encode().
//     tempBuf2 - Another temporary buffer (cannot be the same buffer as
//                tempBuf1) with same lifetime and size as tempBuf1.
// Output:
//     initializes global variables
//
void InitEncoder(t_Code code, int QoS, t_Sample* tempBuf1, t_Sample* tempBuf2);
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Encode is the main Encoder entry point.
// Input: 
//     bufIn    - encoded data to decode
//     bufOut   - empty buffer in which to place decoded data
//     sizeIn   - size of bufIn in bytes
//     sizeOut  - size of bufOut in bytes
// Output:
//     bufOut   - encoded data written here
//     returns  - number of bytes written to bufOut
//
#if defined(CODEC_DEMO)
// Test program version
int Encode(t_Sample* bufIn, t_Sample* bufOut, int sizeIn, int sizeOut,
		   t_CodeInfo* codeInfo,
           t_Sample* levels, int* modes, int samples[9], int storage[9]);
#else
// Release version
int Encode(t_Sample* bufIn, t_Sample* bufOut, int sizeIn, int sizeOut,
		   t_CodeInfo* codeInfo);
#endif
//
//////////////////////////////////////////////////////////////////////////////


