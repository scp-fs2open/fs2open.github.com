/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Anim/PackUnpack.cpp $
 * $Revision: 2.4 $
 * $Date: 2004-02-14 00:18:29 $
 * $Author: randomtiger $
 *
 * Code for handling packing and unpacking in Hoffoss's RLE format, used for
 * Anim files.  Also handles Anim loading, creating Anim instances (for
 * utilizing an Anim), and getting getting frames of the Anim.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2003/10/24 17:35:04  randomtiger
 * Implemented support for 32bit TGA and JPG for D3D
 * Also 32 bit PCX, but it still has some bugs to be worked out
 * Moved convert_24_to_16 out of the bitmap pfunction structures and into packunpack.cpp because thats the only place that uses it.
 *
 * Revision 2.2  2003/03/18 10:07:00  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.1.2.1  2002/11/04 03:02:27  randomtiger
 *
 * I have made some fairly drastic changes to the bumpman system. Now functionality can be engine dependant.
 * This is so D3D8 can call its own loading code that will allow good efficient loading and use of textures that it desparately needs without
 * turning bumpman.cpp into a total hook infested nightmare. Note the new bumpman code is still relying on a few of the of the old functions and all of the old bumpman arrays.
 *
 * I have done this by adding to the gr_screen list of function pointers that are set up by the engines init functions.
 * I have named the define calls the same name as the original 'bm_' functions so that I havent had to change names all through the code.
 *
 * Rolled back to an old version of bumpman and made a few changes.
 * Added new files: grd3dbumpman.cpp and .h
 * Moved the bitmap init function to after the 3D engine is initialised
 * Added includes where needed
 * Disabled (for now) the D3D8 TGA loading - RT
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    7/30/99 10:10p Dave
 * Fixed loading bar in 32 bit mode.
 * 
 * 11    7/18/99 1:59p Johnson
 * Fixed potential anim locking problem.
 * 
 * 10    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 9     7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 8     4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 7     1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 6     12/01/98 5:53p Dave
 * Simplified the way pixel data is swizzled. Fixed tga bitmaps to work
 * properly in D3D and Glide.
 * 
 * 5     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     10/22/98 6:14p Dave
 * Optimized some #includes in Anim folder. Put in the beginnings of
 * parse/localization support for externalized strings and tstrings.tbl
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 30    5/14/98 3:38p John
 * Added in more non-darkening colors for Adam.  Had to fix some bugs in
 * BmpMan and Ani stuff to get this to work.
 * 
 * 29    5/07/98 3:11a Lawrance
 * Implement custom streaming code
 * 
 * 28    11/19/97 8:28p Dave
 * Hooked in Main Hall screen. Put in Anim support for ping ponging
 * animations as well as general reversal of anim direction.
 * 
 * 27    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 26    8/22/97 8:21a Lawrance
 * short circuit key frame check if keyframe matches frame we are
 * searching for
 * 
 * 25    8/21/97 5:11p Lawrance
 * frame numbering for ANI's now is from 0 -> total_frames-1.
 * 
 * 24    8/19/97 10:59a Lawrance
 * fix problem with accessing key frames
 * 
 * 23    7/28/97 10:42p Lawrance
 * re-did interface to unpack_frame() to make more general
 * 
 * 22    7/28/97 10:52a Lawrance
 * correctly set bitmap flags in anim_get_frame()
 * 
 * 21    7/21/97 5:10p Lawrance
 * fix problem that was causing infinite recursion
 * 
 * 20    7/20/97 6:57p Lawrance
 * supporting new RLE format
 * 
 * 19    6/27/97 4:36p Lawrance
 * update pal translation table when gr_screen.signature changes
 * 
 * 18    6/26/97 12:12a Lawrance
 * supporting anti-aliased bitmap animations
 * 
 * 17    6/25/97 3:03p Lawrance
 * fix palette translation problem with anti-alised bitmaps
 * 
 * 16    5/19/97 2:28p Lawrance
 * changes some variables to flags
 * 
 * 15    5/15/97 4:42p Lawrance
 * supporting animations in-game
 * 
 * 14    2/25/97 11:06a Lawrance
 * moved some higher level functions to from PackUnpack to AnimPlay
 * 
 * 13    2/19/97 9:51p Lawrance
 * made keyframe decompression more effecient, moved 
 * default anim FPS to header file
 * 
 * 12    2/19/97 4:00p Lawrance
 * don't assert when cannot find anim filename, return a NULL instead
 * 
 * 11    2/17/97 4:17p Hoffoss
 * modified packing internal format and added random access function to an
 * Anim frame.
 * 
 * 10    2/17/97 2:59p Lawrance
 * integrating into game
 * 
 * 9     2/14/97 11:27p Lawrance
 * optimized unpacking some more (Jason)
 * 
 * 8     2/14/97 11:09p Hoffoss
 * Made optimizations.
 * 
 * 7     2/14/97 10:48p Hoffoss
 * fixed bug.
 * 
 * 6     2/14/97 10:38p Lawrance
 * fixing bugs
 * 
 * 5     2/14/97 5:38p Hoffoss
 * Changes to get AnimCoverter project to compile and link.
 * 
 * 4     2/14/97 3:29p Hoffoss
 * Added header for MSDEV to fill in.
 *
 * $NoKeywords: $
 */

#include "graphics/grinternal.h"
#include "bmpman/bmpman.h"
#include "palman/palman.h"
#include "graphics/2d.h"
#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "graphics/2d.h"

int packer_code = PACKER_CODE;
int transparent_code = 254;

void anim_check_for_palette_change(anim_instance *instance) {
	if ( instance->parent->screen_sig != gr_screen.signature ) {
		instance->parent->screen_sig = gr_screen.signature;
		anim_set_palette(instance->parent);
	}
}

anim_instance *init_anim_instance(anim *ptr, int bpp)
{
	anim_instance *inst;

	if (!ptr) {
		Int3();
		return NULL;
	}

	if ( ptr->flags & ANF_STREAMED ) {
		if ( ptr->file_offset < 0 ) {
			Int3();
			return NULL;
		}
	} else {
		if ( !ptr->data ) {
			Int3();
			return NULL;
		}
	}

	ptr->instance_count++;
	inst = (anim_instance *) malloc(sizeof(anim_instance));
	Assert(inst);
	inst->frame_num = -1;
	inst->last_frame_num = -1;
	inst->parent = ptr;
	inst->data = ptr->data;
	inst->file_offset = ptr->file_offset;
	inst->stop_now = FALSE;
	inst->aa_color = NULL;

	if(bpp == 16){
		inst->frame = (ubyte *) malloc(inst->parent->width * inst->parent->height * 2);
	} else {
		inst->frame = (ubyte *) malloc(inst->parent->width * inst->parent->height);
	}
	return inst;
}

void free_anim_instance(anim_instance *inst)
{
	Assert(inst->frame);
	free(inst->frame);
	inst->frame = NULL;
	inst->parent->instance_count--;	
	inst->parent = NULL;
	inst->data = NULL;
	inst->file_offset = -1;

	free(inst);	
}

int anim_get_next_frame(anim_instance *inst)
{
	int bm, bitmap_flags;
	int aabitmap = 0;
	int bpp = 16;

	if ( anim_instance_is_streamed(inst) ) {
		if ( inst->file_offset <= 0 ) {
			return -1;
		}
	} else {
		if (!inst->data)
			return -1;
	}

	inst->frame_num++;
	if (inst->frame_num >= inst->parent->total_frames) {
		inst->data = NULL;
		inst->file_offset = inst->parent->file_offset;
		return -1;
	}

	if (inst->parent->flags & ANF_XPARENT) {
		// bitmap_flags = BMP_XPARENT;
		bitmap_flags = 0;
	} else {
		bitmap_flags = 0;
	}

	bpp = 16;
	if(inst->aa_color != NULL){
		bitmap_flags |= BMP_AABITMAP;
		aabitmap = 1;
		bpp = 8;
	}

	anim_check_for_palette_change(inst);

	BM_SELECT_TEX_FORMAT();

	if ( anim_instance_is_streamed(inst) ) {
		inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation, aabitmap, bpp);
	} else {
		inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation, aabitmap, bpp);
	}

	bm = bm_create(bpp, inst->parent->width, inst->parent->height, inst->frame, bitmap_flags);
	bm_unload(bm);
	return bm;
}

ubyte *anim_get_next_raw_buffer(anim_instance *inst, int xlate_pal, int aabitmap, int bpp)
{	
	if ( anim_instance_is_streamed(inst) ) {
		if ( inst->file_offset < 0 ) {
			return NULL;
		}
	} else {
		if (!inst->data){
			return NULL;
		}
	}

	inst->frame_num++;
	if (inst->frame_num >= inst->parent->total_frames) {
		inst->data = NULL;
		inst->file_offset = inst->parent->file_offset;
		return NULL;
	}

	anim_check_for_palette_change(inst);

	if ( anim_instance_is_streamed(inst) ) {
		if ( xlate_pal ){
			inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation, aabitmap, bpp);
		} else {
			inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, NULL, aabitmap, bpp);
		}
	} else {
		if ( xlate_pal ){
			inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation, aabitmap, bpp);
		} else {
			inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, NULL, aabitmap, bpp);
		}
	}

	return inst->frame;
}

// --------------------------------------------------------------------
// anim_get_frame()
//
// Get a bitmap id from the anim_instance for the specified frame_num
//
//	input:	*inst			=>		pointer to anim instance
//				frame_num	=>		frame number to get (first frame is 0)
//				xlate_pal	=>		DEFAULT PARM (value 1): whether to translate the palette
//										to the current game palette
//
int anim_get_frame(anim_instance *inst, int frame_num, int xlate_pal)
{
	/*
	int			bm, bitmap_flags, key = 0, offset = 0;
	int idx;

	if ((frame_num < 0) || (frame_num >= inst->parent->total_frames))  // illegal frame number
		return -1;

	int need_reset = 0;
	if ( anim_instance_is_streamed(inst) ) {
		if ( inst->file_offset < 0 ) {
			need_reset = 1;
		}
	} else {
		if ( !inst->data ) {
			need_reset = 1;
		}
	}

	if (need_reset || (inst->frame_num >= inst->parent->total_frames)) {  // reset to valid info
		inst->data = inst->parent->data;
		inst->file_offset = inst->parent->file_offset;
		inst->frame_num = 0;
	}

	bitmap_flags = 0;
	if (inst->parent->flags & ANF_XPARENT) {
		// bitmap_flags = BMP_XPARENT;
		bitmap_flags = 0;
	}

	if ( inst->frame_num == frame_num ) {
		bm = bm_create(16, inst->parent->width, inst->parent->height, inst->frame, bitmap_flags);
		bm_unload(bm);
		return bm;
	}

	if (inst->parent->flags & ANF_XPARENT){
		// bitmap_flags = BMP_XPARENT;
		bitmap_flags = 0;
	} else {
		bitmap_flags = 0;
	}

	idx = 0;
	key = 0;
	while(idx < inst->parent->num_keys){			
		if (( (inst->parent->keys[idx].frame_num-1) <= frame_num) && ( (inst->parent->keys[idx].frame_num-1) > key)) {  // find closest key
			key = inst->parent->keys[idx].frame_num - 1;
			offset = inst->parent->keys[idx].offset;
				
			if ( key == frame_num )
				break;
		}
		idx++;
	}
			
	if ( key == frame_num ) {
		inst->frame_num = key;

		if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = inst->parent->file_offset + offset;
		} else {
			inst->data = inst->parent->data + offset;
		}

		anim_check_for_palette_change(inst);

		if ( anim_instance_is_streamed(inst) ) {
			if ( xlate_pal ){
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation);
			} else {
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, NULL);
			}
		} else {
			if ( xlate_pal ){
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation);
			} else {
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, NULL);
			}
		}

		goto create_bitmap;
	}

	if (key > inst->frame_num)  // best key is closer than current position
	{
		inst->frame_num = key;

		if ( anim_instance_is_streamed(inst) ) {
			inst->file_offset = inst->parent->file_offset + offset;
		} else {
			inst->data = inst->parent->data + offset;
		}

		anim_check_for_palette_change(inst);

		if ( anim_instance_is_streamed(inst) ) {
			if ( xlate_pal )
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation);
			else 
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, NULL);
		} else {
			if ( xlate_pal )
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation);
			else 
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, NULL);
		}
	}

	while (inst->frame_num != frame_num) {
		anim_check_for_palette_change(inst);

		if ( anim_instance_is_streamed(inst) ) {
			if ( xlate_pal )
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation);
			else 
				inst->file_offset = unpack_frame_from_file(inst, inst->frame, inst->parent->width*inst->parent->height, NULL);
		} else {
			if ( xlate_pal )
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, inst->parent->palette_translation);
			else 
				inst->data = unpack_frame(inst, inst->data, inst->frame, inst->parent->width*inst->parent->height, NULL);
		}
		inst->frame_num++;
	}

	create_bitmap:

	bm = bm_create(16, inst->parent->width, inst->parent->height, inst->frame, bitmap_flags);
	bm_unload(bm);
	return bm;
  */
	Int3();
	return -1;
}

// frame = frame pixel data to pack
// save = memory to store packed data to
// size = number of bytes to pack
// max = maximum number of packed bytes (size of buffer)
// returns: actual number of bytes data packed to or -1 if error
int pack_key_frame(ubyte *frame, ubyte *save, long size, long max, int compress_type)
{
	int last = -32768, count = 0;
	long packed_size = 1;

	switch ( compress_type ) {
		case PACKING_METHOD_RLE_KEY:
			*save++ = PACKING_METHOD_RLE_KEY;
			while (size--) {
				if (*frame != last || count > 255) {
					if (packed_size + 3 >= max)
						return -1;

					if (count < 3) {
						if (last == packer_code) {
							*save++ = (ubyte)packer_code;
							*save++ = (ubyte)(count - 1);
							packed_size += 2;

						} else
							while (count--) {
								*save++ = (ubyte)last;
								packed_size++;
							}

					} else {
						*save++ = (ubyte)packer_code;
						*save++ = (ubyte)(count - 1);
						*save++ = (ubyte)last;
						packed_size += 3;
					}

					count = 0;
					last = *frame;
				}

				count++;
				frame++;
			}

			if (packed_size + 3 >= max)
				return -1;

			if (count < 3) {
				if (last == packer_code) {
					*save++ = (ubyte)packer_code;
					*save++ = (ubyte)(count - 1);
					packed_size += 2;

				} else
					while (count--) {
						*save++ = (ubyte)last;
						packed_size++;
					}

			} else {
				*save++ = (ubyte)packer_code;
				*save++ = (ubyte)(count - 1);
				*save++ = (ubyte)last;
				packed_size += 3;
			}
			break;

		case PACKING_METHOD_STD_RLE_KEY: {
			ubyte *dest_start;
			int i;

			dest_start = save;
			count = 1;

			last = *frame++;
			*save++ = PACKING_METHOD_STD_RLE_KEY;
			for (i=1; i < size; i++ )	{

				if ( *frame != last ) {
					if ( count ) {

						if (packed_size + 2 >= max)
							return -1;

						if ( (count == 1) && !(last & STD_RLE_CODE) ) {
							*save++ = (ubyte)last;
							packed_size++;
							Assert( last != STD_RLE_CODE );
//							printf("Just packed %d 1 times, since pixel change, no count included\n",last);
						}
						else {
							count |= STD_RLE_CODE;
							*save++ = (ubyte)count;
							*save++ = (ubyte)last;
							packed_size += 2;
//							printf("Just packed %d %d times, since pixel change\n",last,count);
						}
					}
		
					last = *frame;
					count = 0;
				}

				count++;
				frame++;

				if ( count == 127 ) {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
					count = 0;
//					printf("Just packed %d %d times, since count overflow\n",last,count);

				}
			}	// end for

			if (count)	{

				if (packed_size + 2 >= max)
					return -1;

				if ( (count == 1) && !(last & STD_RLE_CODE) ) {
					*save++ = (ubyte)last;
					packed_size++;
//					printf("Just packed %d 1 times, at end since single pixel, no count\n",last);
					Assert( last != STD_RLE_CODE );
				}
				else {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
//					printf("Just packed %d %d times, at end since pixel change\n",last,count);
				}
			}

			Assert(packed_size == (save-dest_start) );
			return packed_size;
			break;
			}

		default:
			Assert(0);
			return -1;
			break;
	} // end switch

	return packed_size;
}

// frame = frame pixel data to pack
// frame2 = previous frame's pixel data
// save = memory to store packed data to
// size = number of bytes to pack
// max = maximum number of packed bytes (size of buffer)
// returns: actual number of bytes data packed to or -1 if error
int pack_frame(ubyte *frame, ubyte *frame2, ubyte *save, long size, long max, int compress_type)
{
	int pixel, last = -32768, count = 0, i;
	long packed_size = 1;

	switch ( compress_type ) {
		case PACKING_METHOD_RLE:					// Hoffoss RLE regular frame
			*save++ = PACKING_METHOD_RLE;
			while (size--) {
				if (*frame != *frame2++)
					pixel = *frame;
				else
					pixel = transparent_code;

				if (pixel != last || count > 255) {
					if (packed_size + 3 >= max)
						return -1;

					if (count < 3) {
						if (last == packer_code) {
							*save++ = (ubyte)packer_code;
							*save++ = (ubyte)(count - 1);
							packed_size += 2;

						} else
							while (count--) {
								*save++ = (ubyte)last;
								packed_size++;
							}

					} else {
						*save++ = (ubyte)packer_code;
						*save++ = (ubyte)(count - 1);
						*save++ = (ubyte)last;
						packed_size += 3;
					}

					count = 0;
					last = pixel;
				}

				frame++;
				count++;
			}

			if (packed_size + 3 >= max)
				return -1;

			if (count < 3) {
				if (last == packer_code) {
					*save++ = (ubyte)packer_code;
					*save++ = (ubyte)(count - 1);
					packed_size += 2;

				} else
					while (count--) {
						*save++ = (ubyte)last;
						packed_size++;
					}

			} else {
				*save++ = (ubyte)(packer_code);
				*save++ = (ubyte)(count - 1);
				*save++ = (ubyte)(last);
				packed_size += 3;
			}
			break;

		case PACKING_METHOD_STD_RLE: {		// high bit count regular RLE frame

			ubyte *dest_start;

			dest_start = save;
			count = 1;

			if (*frame++ != *frame2++)
				last = *frame;
			else
				last = transparent_code;

			*save++ = PACKING_METHOD_STD_RLE;
			for (i=1; i < size; i++ )	{

				if (*frame != *frame2++)
					pixel = *frame;
				else
					pixel = transparent_code;

				if ( pixel != last ) {
					if ( count ) {

						if (packed_size + 2 >= max)
							return -1;

						if ( (count == 1) && !(last & STD_RLE_CODE) ) {
							*save++ = (ubyte)last;
							packed_size++;
							Assert( last != STD_RLE_CODE );
						}
						else {
							count |= STD_RLE_CODE;
							*save++ = (ubyte)count;
							*save++ = (ubyte)last;
							packed_size += 2;
						}
					}
		
					last = pixel;
					count = 0;
				}

				count++;
				frame++;

				if ( count == 127 ) {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
					count = 0;
				}
			}	// end for

			if (count)	{

				if (packed_size + 2 >= max)
					return -1;

				if ( (count == 1) && !(last & STD_RLE_CODE) ) {
					*save++ = (ubyte)last;
					packed_size++;
					Assert( last != STD_RLE_CODE );
				}
				else {
					count |= STD_RLE_CODE;
					*save++ = (ubyte)count;
					*save++ = (ubyte)last;
					packed_size += 2;
				}
			}

			Assert(packed_size == (save-dest_start) );
			return packed_size;
			break;
			}

		default:
			Assert(0);
			return -1;
			break;
	} // end switch

	return packed_size;
}

// convert a 24 bit value to a 16 bit value
void convert_24_to_16(int bit_24, ushort *bit_16)
{
	ubyte *pixel = (ubyte*)&bit_24;
	ubyte alpha = 1;

	bm_set_components((ubyte*)bit_16, (ubyte*)&pixel[0], (ubyte*)&pixel[1], (ubyte*)&pixel[2], &alpha);	
}

// unpack a pixel given the passed index and the anim_instance's palette, return bytes stuffed
int unpack_pixel(anim_instance *ai, ubyte *data, ubyte pix, int aabitmap, int bpp)
{
	int bit_24;
	ushort bit_16 = 0;	
	ubyte bit_8 = 0;
	ubyte al = 0;
	ubyte r, g, b;
	anim *a = ai->parent;
	Assert(a);	

	// if this is an aabitmap, don't run through the palette
	if(aabitmap){
		switch(bpp){
		case 16 : 
			bit_16 = (ushort)pix;
			break;
		case 8:
			bit_8 = pix;
			break;
		default:
			Int3();
		}
	} else {		
		// if the pixel value is 255, or is the xparent color, make it so		
		if(((a->palette[pix*3] == a->xparent_r) && (a->palette[pix*3+1] == a->xparent_g) && (a->palette[pix*3+2] == a->xparent_b)) ){
			r = b = 0;
			g = 255;
			bm_set_components((ubyte*)&bit_16, &r, &g, &b, &al);
		} else {
			// stuff the 24 bit value
			memcpy(&bit_24, &ai->parent->palette[pix * 3], 3);

			// convert to 16 bit
			convert_24_to_16(bit_24, &bit_16);
		}
	}

	// stuff the pixel
	switch(bpp){
	case 16 :
		memcpy(data, &bit_16, sizeof(ushort));
		return sizeof(ushort);

	case 8 :
		*data = bit_8;		
		return sizeof(ubyte);	
	}

	Int3();
	return 0;
}

// unpack a pixel given the passed index and the anim_instance's palette, return bytes stuffed
int unpack_pixel_count(anim_instance *ai, ubyte *data, ubyte pix, int count, int aabitmap, int bpp)
{
	int bit_24;
	int idx;
	ubyte al = 0;
	ushort bit_16 = 0;	
	ubyte bit_8 = 0;
	anim *a = ai->parent;
	ubyte r, g, b;
	Assert(a);	

	// if this is an aabitmap, don't run through the palette
	if(aabitmap){
		switch(bpp){
		case 16 : 
			bit_16 = (ushort)pix;
			break;
		case 8 :
			bit_8 = pix;
			break;
		default :
			Int3();			
		}
	} else {		
		// if the pixel value is 255, or is the xparent color, make it so		
		if(((a->palette[pix*3] == a->xparent_r) && (a->palette[pix*3+1] == a->xparent_g) && (a->palette[pix*3+2] == a->xparent_b)) ){
			r = b = 0;
			g = 255;
			bm_set_components((ubyte*)&bit_16, &r, &g, &b, &al);
		} else {
			// stuff the 24 bit value
			memcpy(&bit_24, &ai->parent->palette[pix * 3], 3);

			// convert to 16 bit
			convert_24_to_16(bit_24, &bit_16);
		}
	}
	
	// stuff the pixel
	for(idx=0; idx<count; idx++){
		switch(bpp){
		case 16 :
			memcpy(data + (idx*2), &bit_16, sizeof(ushort));
			break;
		case 8 :
			*(data + idx) = bit_8;
			break;
		}
	}

	if(bpp == 16){
		return sizeof(ushort) * count;
	}
	return sizeof(ubyte) * count;
}

// ptr = packed data to unpack
// frame = where to store unpacked data to
// size = total number of unpacked pixels requested
// pal_translate = color translation lookup table (NULL if no palette translation desired)
ubyte	*unpack_frame(anim_instance *ai, ubyte *ptr, ubyte *frame, int size, ubyte *pal_translate, int aabitmap, int bpp)
{
	int	xlate_pal, value, count = 0;
	int stuffed;			
	int pixel_size = (bpp == 16) ? 2 : 1;

	if ( pal_translate == NULL ) {
		xlate_pal = 0;
	}
	else {
		xlate_pal = 1;
	}

	if (*ptr == PACKING_METHOD_RLE_KEY) {  // key frame, Hoffoss's RLE format
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if (value != packer_code) {
				if ( xlate_pal ){
					stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
				} else {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				}
				frame += stuffed;
				size--;
			} else {
				count = *ptr++;
				if (count < 2){
					value = packer_code;
				} else {
					value = *ptr++;
				}

				if (++count > size){
					count = size;
				}

				if ( xlate_pal ){
					stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
				} else {					
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				}

				frame += stuffed;
				size -= count;
			}
		}
	}
	else if ( *ptr == PACKING_METHOD_STD_RLE_KEY) {	// key frame, with high bit as count
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if ( !(value & STD_RLE_CODE) ) {
				if ( xlate_pal ){
					stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
				} else {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = *ptr++;

				size -= count;
				Assert(size >= 0);

				if ( xlate_pal ){
					stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
				} else {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				}

				frame += stuffed;
			}
		}
	}
	else if (*ptr == PACKING_METHOD_RLE) {  // normal frame, Hoffoss's RLE format

// test code, to show unused pixels
// memset(frame, 255, size);
	
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if (value != packer_code) {
				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
					} else {
						stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
					}
				} else {
					// temporary pixel
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = *ptr++;
				if (count < 2){
					value = packer_code;
				} else {
					value = *ptr++;
				}

				if (++count > size){
					count = size;
				}

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code ) {
					if ( xlate_pal ) {
						stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
					} else {
						stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
					}
				} else {
					stuffed = count * pixel_size;
				}

				frame += stuffed;
			}
		}

	}
	else if ( *ptr == PACKING_METHOD_STD_RLE) {	// normal frame, with high bit as count
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if ( !(value & STD_RLE_CODE) ) {
				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
					} else {
						stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
					}
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = *ptr++;

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
					} else {
						stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
					}
				} else {					
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}
	}
	else {
		Assert(0);  // unknown packing method
	}

	return ptr;
}

// ptr = packed data to unpack
// frame = where to store unpacked data to
// size = total number of unpacked pixels requested
// pal_translate = color translation lookup table (NULL if no palette translation desired)
int unpack_frame_from_file(anim_instance *ai, ubyte *frame, int size, ubyte *pal_translate, int aabitmap, int bpp)
{
	int	xlate_pal, value, count = 0;
	int	offset = 0;
	int stuffed;	
	int pixel_size = (bpp == 16) ? 2 : 1;

	if ( pal_translate == NULL ) {
		xlate_pal = 0;
	}
	else {
		xlate_pal = 1;
	}

	if (anim_instance_get_byte(ai,offset) == PACKING_METHOD_RLE_KEY) {  // key frame, Hoffoss's RLE format
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if (value != packer_code) {
				if ( xlate_pal ){
					stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
				} else {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				}

				frame += stuffed;
				size--;
			} else {
				count = anim_instance_get_byte(ai,offset);
				offset++;
				if (count < 2) {
					value = packer_code;
				} else {
					value = anim_instance_get_byte(ai,offset);
					offset++;
				}

				if (++count > size){
					count = size;
				}

				if ( xlate_pal ){
					stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
				} else {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				}

				frame += stuffed;
				size -= count;
			}
		}
	}
	else if ( anim_instance_get_byte(ai,offset) == PACKING_METHOD_STD_RLE_KEY) {	// key frame, with high bit as count
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if ( !(value & STD_RLE_CODE) ) {
				if ( xlate_pal ){
					stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
				} else {
					stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = anim_instance_get_byte(ai,offset);
				offset++;

				size -= count;
				Assert(size >= 0);

				if ( xlate_pal ){
					stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
				} else {
					stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
				}

				frame += stuffed;
			}
		}
	}
	else if (anim_instance_get_byte(ai,offset) == PACKING_METHOD_RLE) {  // normal frame, Hoffoss's RLE format

// test code, to show unused pixels
// memset(frame, 255, size);
	
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if (value != packer_code) {
				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
					} else {
						stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
					}
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = anim_instance_get_byte(ai,offset);
				offset++;

				if (count < 2) {
					value = packer_code;
				} else {
					value = anim_instance_get_byte(ai,offset);
					offset++;
				}
				if (++count > size){
					count = size;
				}

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code ) {
					if ( xlate_pal ) {
						stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
					} else {
						stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
					}
				} else {
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}

	}
	else if ( anim_instance_get_byte(ai,offset) ) {	// normal frame, with high bit as count
		offset++;
		while (size > 0) {
			value = anim_instance_get_byte(ai,offset);
			offset++;
			if ( !(value & STD_RLE_CODE) ) {
				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel(ai, frame, pal_translate[value], aabitmap, bpp);
					} else {
						stuffed = unpack_pixel(ai, frame, (ubyte)value, aabitmap, bpp);
					}
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = anim_instance_get_byte(ai,offset);
				offset++;

				size -= count;
				Assert(size >= 0);

				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel_count(ai, frame, pal_translate[value], count, aabitmap, bpp);
					} else {
						stuffed = unpack_pixel_count(ai, frame, (ubyte)value, count, aabitmap, bpp);
					}
				} else {					
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}
	}
	else {
		Int3();  // unknown packing method
	}

	return ai->file_offset + offset;
}


// TODO: actually convert the frame data to correct palette at this point
void anim_set_palette(anim *ptr)
{
	int i, xparent_found = 0;
	
	// create the palette translation look-up table
	for ( i = 0; i < 256; i++ ) {

		//if ( (ptr->palette[i*3] == ptr->xparent_r) && (ptr->palette[i*3+1] == ptr->xparent_g) && (ptr->palette[i*3+2] == ptr->xparent_b) ) {
		//	ptr->palette_translation[i] = 255;
		//	xparent_found = 1;
		//} else	{
			// ptr->palette_translation[i] = (ubyte)palette_find( ptr->palette[i*3], ptr->palette[i*3+1], ptr->palette[i*3+2] );
			ptr->palette_translation[i] = (ubyte)i;
		//}
	}	

	if ( xparent_found ) {
		ptr->flags |= ANF_XPARENT;
	}
	else {
		ptr->flags &= ~ANF_XPARENT;
	}
}
