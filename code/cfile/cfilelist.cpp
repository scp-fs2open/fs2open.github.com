/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>		/* needed for memory mapping of file functions */
#endif

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"


void cf_sort_filenames( SCP_vector<SCP_string> &list, int sort, SCP_vector<file_list_info> *info )
{
	// NOTE: This really needs to be updated to C++ style sorting at some point

	int i, j, incr;
	SCP_string t;
	file_list_info tt;

	int n = (int)list.size();

	if (sort == CF_SORT_NAME) {
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if (stricmp(list[j].c_str(), list[j + incr].c_str()) > 0) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						if (info) {
							tt = (*info)[j];
							(*info)[j] = (*info)[j + incr];
							(*info)[j + incr] = tt;
						}

						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_TIME) {
		Assert(info);
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if ( (*info)[j].write_time < (*info)[j + incr].write_time ) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						tt = (*info)[j];
						(*info)[j] = (*info)[j + incr];
						(*info)[j + incr] = tt;
						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_REVERSE) {
		std::reverse( list.begin(), list.end() );

		if (info) {
			std::reverse( info->begin(), info->end() );
		}

		return;
	}

	nprintf(("Error", "Unknown sorting method %d passed to cf_sort_filenames()\n", sort));
}

// Sorts a list of filenames using the specified sorting method (CF_SORT_*).
//   n = number of filenames in list to sort
//   list = list of filenames to be sorted
//   sort = sorting method to use (one of the CF_SORT_* defines)
//   info = extra info for each file.  Only required if sorting by time, however if you
//          have extra file info, you should pass it as well to get it sorted too (so an
//          index into list is the same index for info for that file
void cf_sort_filenames( int n, char **list, int sort, file_list_info *info )
{
	int i, j, incr;
	char *t;
	file_list_info tt;

	if (sort == CF_SORT_NAME) {
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if (stricmp(list[j], list[j + incr]) > 0) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						if (info) {
							tt = info[j];
							info[j] = info[j + incr];
							info[j + incr] = tt;
						}

						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_TIME) {
		Assert(info);
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if (info[j].write_time < info[j + incr].write_time) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						tt = info[j];
						info[j] = info[j + incr];
						info[j + incr] = tt;
						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_REVERSE) {
		incr = n / 2;
		char buffer[MAX_FILENAME_LEN];
		file_list_info tt_tmp;

		for (i = 0; i < incr; i++) {
			t = list[n - 1 - i];

			if (list[i] != t) {
				strcpy_s(buffer, list[i]);
				strcpy(list[i], t);
				strcpy(t, buffer);

				if (info) {
					tt = info[n - 1 - i];
					tt_tmp = info[i];
					info[i] = tt;
					tt = tt_tmp;
				}
			}
		}

		return;
	}

	nprintf(("Error", "Unknown sorting method %d passed to cf_sort_filenames()\n", sort));
}


#if (0) // these aren't used anywhere now

//	cf_compress - Do Run Length Compression on a block of data. Targa format.
//
//	Usage:
//				out			Buffer to write it out to
//				in				Buffer to compress
//				bytecount	Number of bytes input
int cf_compress(char *out, char *in, int bytecount )
{
	int pixcount;		// number of pixels in the current packet
	char *inputpixel=NULL;	// current input pixel position
	char *matchpixel=NULL;	// pixel value to match for a run
	char *flagbyte=NULL;		// location of last flag byte to set
	int rlcount;		// current count in r.l. string 
	int rlthresh;		// minimum valid run length
	char *copyloc;		// location to begin copying at

	// set the threshold -- the minimum valid run length
	rlthresh = 2;					// Require a 2 pixel span before rle'ing
	
	// set the first pixel up

	flagbyte = out;	// place to put next flag if run
	inputpixel = in;
	pixcount = 1;
	rlcount = 0;
	copyloc = (char *)0;

	// loop till data processing complete
	do	{

		// if we have accumulated a 128-byte packet, process it
		if ( pixcount == 129 )	{
			*flagbyte = 127;

			// set the run flag if this is a run

			if ( rlcount >= rlthresh )	{
					*flagbyte |= 0x80;
					pixcount = 2;
			}

			// copy the data into place
			++flagbyte;
			memmove( flagbyte, copyloc, pixcount-1 );
			flagbyte += pixcount-1;
			pixcount = 1;

			// set up for next packet
			continue;
		}

		// if zeroth byte, handle as special case
		if ( pixcount == 1 )	{
			rlcount = 0;
			copyloc = inputpixel;		/* point to 1st guy in packet */
			matchpixel = inputpixel;	/* set pointer to pix to match */
			pixcount = 2;
			inputpixel += 1;
			continue;
		}

		// assembling a packet -- look at next pixel

		// current pixel == match pixel?
		if ( *inputpixel == *matchpixel )	{

			//	establishing a run of enough length to
			//	save space by doing it
			//		-- write the non-run length packet
			//		-- start run-length packet

			if ( ++rlcount == rlthresh )	{
				
				//	close a non-run packet
				
				if ( pixcount > (rlcount+1) )	{
					// write out length and do not set run flag

					*flagbyte++ = (char)(pixcount - 2 - rlthresh);

					memmove(flagbyte, copyloc, (pixcount-1-rlcount) );
					flagbyte += (pixcount-1-rlcount);

					copyloc = inputpixel;
					pixcount = rlcount + 1;
				}
			}
		} else {

			// no match -- either break a run or continue without one
			//	if a run exists break it:
			//		write the bytes in the string (1+1)
			//		start the next string

			if ( rlcount >= rlthresh )	{

				*flagbyte++ = (char)(0x80 | rlcount);
				memmove(flagbyte, copyloc, 1 );
				flagbyte += 1;
				pixcount = 1;
				continue;
			} else {

				//	not a match and currently not a run
				//		- save the current pixel
				//		- reset the run-length flag
				rlcount = 0;
				matchpixel = inputpixel;
			}
		}
		pixcount++;
		inputpixel += 1;
	} while ( inputpixel < (in + bytecount));

	// quit this buffer without loosing any data
	if ( --pixcount >= 1 )	{
		*flagbyte = (char)(pixcount - 1);
		if ( rlcount >= rlthresh )	{
			*flagbyte |= 0x80;
			pixcount = 1;
		}

		// copy the data into place
		++flagbyte;
		memmove(flagbyte, copyloc, pixcount );
		flagbyte += pixcount;
	}
	return(flagbyte-out);
}


//	cf_decompress - Do Decompression on a run-length encoded block of data. Targa format.
//
//	Usage:
//				out			Buffer to write it out to
//				in				Buffer to compress
//				bytecount	Number of bytes input
int cf_decompress(char *out, char *in )
{
	int count;

	char *param_out = out;

	while(1)	{
	
		count = int(*in++);
		int run_span = count & 0x80;

		count &= (~0x80);

		if ( count > 0 )	{
			if ( run_span )	{
				// RLE'd data
				ubyte c = *in++;

				memset( out, c, count );
				out += count;
			} else {
				memmove( out, in, count );
				in += count;
				out += count;
			}
		}
	}

	return out - param_out;
			
}

#endif // 0


// cfread() reads from a file and decompresses it
//
// returns:   returns the number of full elements read
//            
//
int cfread_compressed(void *buf, int elsize, int nelem, CFILE *cfile)
{
	char *out = (char *)buf;
	
	while(1)	{

		ubyte count;

		if ( cfread( &count, 1, 1, cfile ) != 1 )	{
			break;
		}

		int run_span = count & 0x80;
		count &= (~0x80);
		count++;

		if ( count > 0 )	{
			if ( run_span )	{
				// RLE'd data
				ubyte c;
				if ( cfread( &c, 1, 1, cfile ) != 1 )	{
					break;
				}
				memset( out, c, count );
			} else {
				if ( cfread( out, 1, count, cfile ) != count )	{
					break;
				}
			}
			out += count;
			if ( out >= (char *)buf + (elsize*nelem))	{
				break;
			}
		} else {
			break;
		}
	}

	return (out - (char *)buf)/elsize;
}

int cfwrite_compressed(void *param_buf, int param_elsize, int param_nelem, CFILE *cfile)
{
	char *in = (char *)param_buf;
	int bytecount = (param_elsize * param_nelem );

	int pixcount;		// number of pixels in the current packet
	char *inputpixel=NULL;	// current input pixel position
	char *matchpixel=NULL;	// pixel value to match for a run
	int rlcount;		// current count in r.l. string 
	int rlthresh;		// minimum valid run length
	char *copyloc;		// location to begin copying at

	// set the threshold -- the minimum valid run length
	rlthresh = 2;					// Require a 2 pixel span before rle'ing
	
	// set the first pixel up

	inputpixel = in;
	pixcount = 1;
	rlcount = 0;
	copyloc = (char *)0;

	// loop till data processing complete
	do	{

		// if we have accumulated a 128-byte packet, process it
		if ( pixcount == 129 )	{
			ubyte code = 127;

			// set the run flag if this is a run

			if ( rlcount >= rlthresh )	{
					code |= 0x80;
					pixcount = 2;
			}

			cfwrite( &code, 1, 1, cfile );

			// copy the data into place
			cfwrite( copyloc, 1, pixcount-1, cfile );
			pixcount = 1;

			// set up for next packet
			continue;
		}

		// if zeroth byte, handle as special case
		if ( pixcount == 1 )	{
			rlcount = 0;
			copyloc = inputpixel;		/* point to 1st guy in packet */
			matchpixel = inputpixel;	/* set pointer to pix to match */
			pixcount = 2;
			inputpixel += 1;
			continue;
		}

		// assembling a packet -- look at next pixel

		// current pixel == match pixel?
		if ( *inputpixel == *matchpixel )	{

			//	establishing a run of enough length to
			//	save space by doing it
			//		-- write the non-run length packet
			//		-- start run-length packet

			if ( ++rlcount == rlthresh )	{
				
				//	close a non-run packet
				
				if ( pixcount > (rlcount+1) )	{
					// write out length and do not set run flag

					ubyte code = (ubyte)(pixcount - 2 - rlthresh);
					cfwrite( &code, 1, 1, cfile );

					cfwrite( copyloc, 1, (pixcount-1-rlcount), cfile );
					copyloc = inputpixel;
					pixcount = rlcount + 1;
				}
			}
		} else {

			// no match -- either break a run or continue without one
			//	if a run exists break it:
			//		write the bytes in the string (1+1)
			//		start the next string

			if ( rlcount >= rlthresh )	{

				ubyte code = (ubyte)(0x80 | rlcount);
				cfwrite( &code, 1, 1, cfile );
				cfwrite( copyloc, 1, 1, cfile );
				pixcount = 1;
				continue;
			} else {

				//	not a match and currently not a run
				//		- save the current pixel
				//		- reset the run-length flag
				rlcount = 0;
				matchpixel = inputpixel;
			}
		}
		pixcount++;
		inputpixel += 1;
	} while ( inputpixel < (in + bytecount));

	// quit this buffer without loosing any data
	if ( --pixcount >= 1 )	{
		ubyte code = ubyte(pixcount - 1);

		// set the run flag if this is a run

		if ( rlcount >= rlthresh )	{
				code |= 0x80;
				pixcount = 1;
		}

		cfwrite( &code, 1, 1, cfile );

		// copy the data into place
		cfwrite( copyloc, 1, pixcount, cfile );
	}

	return param_nelem;
}
