/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/midiseq.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:29 $
 * $Author: penguin $
 *
 * Header file with MIDI parsing constants and data structures
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 1     4/28/97 4:45p John
 * Initial version of ripping sound & movie out of OsAPI.
 * 
 * 4     2/10/97 9:26a Lawrance
 * 
 * 3     1/31/97 2:40p Lawrance
 * MIDI playback working
 *
 * $NoKeywords: $
 */

#ifndef __MIDIBASE_H__
#define __MIDIBASE_H__


// MIDI file constants
//

#define MThd        0x6468544D      // Start of file
#define MTrk        0x6B72544D      // Start of track

#define MIDI_SYSEX		((BYTE)0xF0)        // SysEx begin
#define MIDI_SYSEXEND   ((BYTE)0xF7)        // SysEx begin
#define MIDI_META			((BYTE)0xFF)        // Meta event begin
#define MIDI_META_TEMPO ((BYTE)0x51)        // Tempo change
#define MIDI_META_EOT   ((BYTE)0x2F)        // End-of-track

#define MIDI_NOTEOFF    ((BYTE)0x80)        // + note + velocity
#define MIDI_NOTEON		((BYTE)0x90)        // + note + velocity
#define MIDI_POLYPRESS  ((BYTE)0xA0)        // + pressure (2 bytes)
#define MIDI_CTRLCHANGE ((BYTE)0xB0)        // + ctrlr + value
#define MIDI_PRGMCHANGE ((BYTE)0xC0)        // + new patch
#define MIDI_CHANPRESS  ((BYTE)0xD0)        // + pressure (1 byte)
#define MIDI_PITCHBEND  ((BYTE)0xE0)        // + pitch bend (2 bytes)

#define MIDI_META_MARKER		((BYTE)0x06)        // Marker META event
#define NUM_CHANNELS    16

#define MIDICTRL_VOLUME     ((BYTE)0x07)
#define MIDICTRL_VOLUME_LSB ((BYTE)0x27)
#define MIDICTRL_PAN        ((BYTE)0x0A)

#define MIDIEVENT_CHANNEL(dw)		(dw & 0x0000000F)
#define MIDIEVENT_TYPE(dw)			(dw & 0x000000F0)
#define MIDIEVENT_DATA1(dw)		((dw & 0x0000FF00) >> 8)
#define MIDIEVENT_VOLUME(dw)		((dw & 0x007F0000) >> 16)

// Macros for swapping hi/lo-endian data
//
#define WORDSWAP(w) (((w) >> 8) | \
            (((w) << 8) & 0xFF00))

#define DWORDSWAP(dw)   (((dw) >> 24) |         \
            (((dw) >> 8) & 0x0000FF00) |    \
            (((dw) << 8) & 0x00FF0000) |    \
            (((dw) << 24) & 0xFF000000))

// Make a little distinction here so the various structure members are a bit
// more clearly labelled -- we have offsets and byte counts to keep track of
// that deal with both in-memory buffers and the file on disk

#define FILEOFF DWORD
            
// These structures are stored in MIDI files; they need to be byte aligned.
//
#pragma pack(1)

// Chunk header. dwTag is either MTrk or MThd.
//
typedef struct
{
    DWORD   dwTag;			     // Type
    DWORD   dwChunkLength;      // Length (hi-lo)
} MIDICHUNK;

// Contents of MThd chunk.
typedef struct
{
    WORD    wFormat;					// Format (hi-lo)
    WORD    wTrackCount;			// # tracks (hi-lo)
    WORD    wTimeDivision;			// Time division (hi-lo)
} MIDIFILEHDR;

#pragma pack() // End of need for byte-aligned structures


// Temporary event structure which stores event data until we're ready to
// dump it into a stream buffer
//
typedef struct
{
    DWORD   tkEvent;				// Absolute time of event
    BYTE    byShortData[4];	// Event type and parameters if channel msg
    DWORD   dwEventLength;		// Length of data which follows if meta or sysex
    LPBYTE  pLongData;			// -> Event data if applicable
} TEMPEVENT, *PTEMPEVENT;

#define ITS_F_ENDOFTRK  0x00000001

// Description of a track open for read
//
typedef struct
{
    DWORD   fdwTrack;   // Track status
    DWORD   dwTrackLength;  // Total bytes in track
    DWORD   dwLeftInBuffer; // Bytes left unread in track buffer
    LPBYTE  pTrackStart;    // -> start of track data buffer
    LPBYTE  pTrackCurrent;  // -> next byte to read in buffer
    DWORD   tkNextEventDue; // Absolute time of next event in track
    BYTE    byRunningStatus;// Running status from last channel msg

    FILEOFF foTrackStart;   // Start of track -- used for walking the file
    FILEOFF foNextReadStart;// File offset of next read from disk
    DWORD   dwLeftOnDisk;   // Bytes left unread on disk

//    int	in_song[MAX_SONGS];
	 
} INTRACKSTATE, *PINTRACKSTATE;

// Description of the input MIDI file
//
typedef struct
{
    DWORD   cbFileLength;       // Total bytes in file
    DWORD   dwTimeDivision;     // Original time division
    DWORD   dwFormat;       // Original format
    DWORD   dwTrackCount;       // Track count (specifies pitsTracks size)
    INTRACKSTATE *pitsTracks;       // -> array of tracks in this file
} INFILESTATE, *PINFILESTATE;

#endif /* __MIDIBASE_H__ */

