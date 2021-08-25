#ifndef _BMPMAN_H
#define _BMPMAN_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

/**
 * @file bmpman.h
 * Header file for the bitmap manager (bmpman)
 */

/**
 * @example bm_examples.cpp
 * @brief This containes examples of bmpman usage
 */

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"

#ifndef NDEBUG
#define BMPMAN_NDEBUG	//!< Enables BMPMAN debugging code
#endif

/**
 * @defgroup BMPMAN_CONSTANTS Macro constants used in/with bmpman.h
 *
 * @{
 */

// Flag positions for bitmap.flags
// ***** NOTE:  bitmap.flags is an 8-bit value, no more BMP_TEX_* flags can be added unless the type is changed!! ******
#define	BMP_AABITMAP        (1<<0)      //!< antialiased bitmap
#define	BMP_TEX_XPARENT     (1<<1)      //!< transparent texture
#define	BMP_TEX_OTHER       (1<<2)      //!< so we can identify all "normal" textures
#define BMP_TEX_DXT1        (1<<3)      //!< dxt1 compressed 8r8g8b1a (24bit)
#define BMP_TEX_DXT3        (1<<4)      //!< dxt3 compressed 8r8g8b4a (32bit)
#define BMP_TEX_DXT5        (1<<5)      //!< dxt5 compressed 8r8g8b8a (32bit)
#define BMP_TEX_BC7			(1<<6)		//!< BC7  compressed 8r8g8b8a (32bit)
#define BMP_TEX_CUBEMAP     (1<<7)      //!< a texture made for cubic environment map
#define BMP_MASK_BITMAP     (1<<8)      //!< a bitmap that will be used for masking mouse interaction. Typically not used in render operations

// Combined flags
#define BMP_TEX_COMP        ( BMP_TEX_DXT1 | BMP_TEX_DXT3 | BMP_TEX_DXT5 | BMP_TEX_BC7 )  //!< Compressed textures
#define BMP_TEX_NONCOMP     ( BMP_TEX_XPARENT | BMP_TEX_OTHER )             //!< Non-compressed textures
#define	BMP_TEX_ANY         ( BMP_TEX_COMP | BMP_TEX_NONCOMP )              //!< Any texture

// Flag positions for bitmap.type
#define BMP_FLAG_RENDER_TARGET_STATIC       (1<<0)      //!< Texture is a static type
#define BMP_FLAG_RENDER_TARGET_DYNAMIC      (1<<1)      //!< Texture is a dynamic type (animation)
#define BMP_FLAG_CUBEMAP                    (1<<2)      //!< Texture is a cubemap

// Bitmap types
enum BM_TYPE
{
	BM_TYPE_NONE = 0,   //!< No type
	BM_TYPE_USER,       //!< in-memory
	BM_TYPE_PCX,        //!< PCX
	BM_TYPE_TGA,        //!< 16 or 32 bit targa
	BM_TYPE_DDS,        //!< generic identifier for DDS
	BM_TYPE_PNG,        //!< PNG
	BM_TYPE_JPG,        //!< 32 bit jpeg
	BM_TYPE_ANI,        //!< in-house ANI format
	BM_TYPE_EFF,        //!< specifies any type of animated image, the EFF itself is just text

	// special types
	BM_TYPE_RENDER_TARGET_STATIC,   //!< 24/32 bit setup internally as a static render target
	BM_TYPE_RENDER_TARGET_DYNAMIC,  //!< 24/32 bit setup internally as a dynamic render target

	// Compressed types (bitmap.c_type)
	BM_TYPE_DXT1,           //!< 24 bit with switchable alpha
	BM_TYPE_DXT3,           //!< 32 bit with 4 bit alpha
	BM_TYPE_DXT5,           //!< 32 bit with 8 bit alpha
	BM_TYPE_BC7,			//!< 32-bit with variable alpha
	BM_TYPE_CUBEMAP_DDS,    //!< generic DDS cubemap (uncompressed cubemap surface)
	BM_TYPE_CUBEMAP_DXT1,   //!< 24-bit cubemap        (compressed cubemap surface)
	BM_TYPE_CUBEMAP_DXT3,   //!< 32-bit cubemap        (compressed cubemap surface)
	BM_TYPE_CUBEMAP_DXT5    //!< 32-bit cubemap        (compressed cubemap surface)
};

/**
 * @}
 */

struct bitmap
{
	short	w;          //!< Width, in number of pixels
	short	h;          //!< Height, in number of pixels
	short	rowsize;    //!< What you need to add to go to next row
	int	bpp;        //!< Requested bitdepth of each pixel. ( 7, 8, 15, 16, 24, 32)
	int	true_bpp;   //!< The image's actual bitdepth
	ushort	flags;      //!< Various texture type flags. @see BMPMAN_CONSTANTS
	ptr_u	data;       //!< Pointer to data, or maybe offset into VRAM.
	ubyte *palette;     /**< @brief   Pointer to this bitmap's palette (if it has one).
	                     *   @details If BMP_NO_PALETTE_MAP flag is cleared, this palette just points to the screen palette. (gr_palette)
	                     */
};

// Forward definition for the graphics API
struct bitmap_entry;
struct bitmap_slot;

extern size_t bm_texture_ram;  //!< how many bytes of textures are used.

extern int Bm_paging;   //!< Bool type that indicates if BMPMAN is currently paging.

extern const BM_TYPE bm_type_list[];       //!< List of valid bitmap types
extern const char *bm_ext_list[];        //!< List of extensions for those types
extern const int BM_NUM_TYPES;           //!< Calculated number of bitmap types
extern const BM_TYPE bm_ani_type_list[];   //!< List of valid bitmap animation types
extern const char *bm_ani_ext_list[];    //!< List of extensions for those types
extern const int BM_ANI_NUM_TYPES;       //!< Calculated number of bitmap animation types

extern int ENVMAP;      //!< References a map that is for environment mapping -Bobboau

/**
 * @brief Initilizes the bitmap manager
 */
void bm_init();

/**
 * @brief Closes the bitmap manager, freeing any allocated memory used by bitmaps. Is called at program close.
 */
void bm_close();

#define BMP_FLAG_RENDER_TARGET_STATIC		(1<<0)
#define BMP_FLAG_RENDER_TARGET_DYNAMIC		(1<<1)
#define BMP_FLAG_CUBEMAP					(1<<2)
#define BMP_FLAG_RENDER_TARGET_MIPMAP		(1<<3)

/**
 * @brief Allocates memory for the given handle.
 *
 * @returns A pointer to the allocated vm if successful,
 * @returns Null, if unsuccessful
 *
 * @note z64 - This function looks fishy. Not only is handle not used in release builds, but bm_bitmaps[handle].size
 *   and bm_texture_size aren't modified unless this is a debug build
 */
void *bm_malloc(int handle, size_t size);

/**
 * @brief (DEBUG) Similar to bm_malloc, but only updates how much memory is used
 *
 * @note z64 - Also fishy (see bm_malloc)
 */
void bm_update_memory_used(int n, size_t size);

class bitmap_lookup {
	ubyte *Bitmap_data;

	int Width;
	int Height;
	int Num_channels;

	float map_texture_address(float address);
public:
	bitmap_lookup(int bitmap_num);
	~bitmap_lookup();

	bool valid();

	float get_channel_alpha(float u, float v);
};

void clear_bm_lookup_cache();

/**
 * @brief Loads a bitmap so we can draw with it later.
 *
 * @param filename
 *
 * @returns The bitmap number if successful, else
 * @returns a negative value if not
 */
int bm_load(const char* filename);

/**
 * @brief Loads a bitmap so we can draw with it later. (Preferred version)
 *
 * @param filename
 *
 * @returns The bitmap number if successful, else
 * @returns a negative value if not
 */
int bm_load(const SCP_string& filename);

/**
 * @brief Reloads a bitmap as a duplicate.
 *
 * @details This function basically allows you to load a bitmap which already exists (by filename). This is useful
 *   because in some cases we need to have a bitmap which is locked in screen format _and_ texture format, such as
 *   the pilot pics and squad logos
 *
 * @param filename
 *
 * @returns The bitmap number if successful, else
 * @returns a negative value if not
 */
int bm_load_duplicate(const char *filename);

/**
 * Loads a bitmap which exists somewhere in the RAM.
 *
 * @param bpp The bitdepth of the bitmap
 * @param w The width of the bitmap
 * @param h The height of the bitmap
 * @param[in] data The bitmap's data glob
 * @param flags
 *
 * @note The used RAM cannot be freed until bm_release is called on the created bitmap
 */
int bm_create(int bpp, int w, int h, void *data = NULL, int flags = 0);

/**
 * @brief Unloads a bitmap's data, but not the bitmap info
 *
 * @details The bm number (n) may be reused once this function has been called on it. However, it will have to be paged
 *   in the next time it is locked.

 * @param handle                The index number of the bitmap to free
 * @param clear_render_targets  If true, release a render target
 * @param nodebug               If true, exclude certain debug messages
 *
 * @returns 0 if not successful,
 * @returns 1 if successful
 */
int bm_unload(int handle, int clear_render_targets = 0, bool nodebug = false);

/**
 * @brief Quickly unloads a bitmap's data, ignoring the load_count
 *
 * @details Similar to bm_unload(), except that it can be safely used to free data without worrying about load_count.
 *   It's safe to use in relation to bm_release() and in gr_*_texture functions
 *
 * @param handle                The bm number of the bitmap to free
 * @param clear_render_targets  If true, release a render target
 *
 * @note bm_free_data_fast() is used here and NOT bm_free_data()
 */
int bm_unload_fast(int handle, int clear_render_targets = 0);

/**
 * @brief Frees both a bitmap's data and it's associated slot.
 *
 * @details Once called, the bm number 'n' cannot be used until bm_load or bm_create is re-inits the slot.
 *
 * @param handle               The index number of the bitmap to release
 * @param clear_render_targets If nonzero, also release render targets
 *
 * @returns 1 on success,
 * @returns 0 otherwise
 *
 * @note If the passed handle is that of an ANI, it frees EVERY frame. Be sure to only pass the handle of the first frame!
 *
 * @todo upgrade return type and clear_render_targets type to bools
 */
int bm_release(int handle, int clear_render_targets = 0);

/**
 * @brief Detaches the render target of a bitmap if it exists
 *
 * @details Once called, this handle cannot be used as a target to switch the rendering context to
 *
 * @param handle               The index number of the bitmap to release
 *
 * @returns 1 on success,
 * @returns 0 otherwise
 *
 * @note If the passed handle is that of an ANI, it frees the render target of EVERY frame. Be sure to only pass the handle of the first frame!
 *
 */
bool bm_release_rendertarget(int handle);

/**
 * @brief Loads a bitmap sequance so we can draw with it.
 *
 * @param[in] filename
 * @param[out] nframes          If non-null, set to the number of frames the animation has
 * @param[out] fps              If non-null, set to the fps of this animation
 * @param[out] keyframe         if non null, set to the keyframe index of this animation
 * @param[in] can_drop_frames   If set, allows dropped frames
 * @param[in] dir_type          Directory type
 *
 * @returns The bm number of the first bitmap in the sequence if successful, or
 * @returns A negative value if unsuccessful
 */
int bm_load_animation(const char *filename, int *nframes = nullptr, int *fps = nullptr, int *keyframe = nullptr, float *total_time = nullptr, bool can_drop_frames = 0, int dir_type = CF_TYPE_ANY);

/**
 * @brief Loads either animation (bm_load_animation) or still image (bm_load)
 *
 * @param[in] filename
 * @param[out] nframes  If non-null and loading was successful, set to the number of frames the animation has
 * @param[out] fps      If non-null and loading was successful, set to the fps of this animation
 * @param[out] keyframe if non null and loading was successful, set to the keyframe index of this animation
 * @param[in] can_drop_frames
 * @param[in] dir_type
 *
 * @returns The bm number of the first bitmap in the sequence if successful, or
 * @returns A negative value if unsuccessful
 */
int bm_load_either(const char *filename, int *nframes = NULL, int *fps = NULL, int *keyframe = NULL, bool can_drop_frames = false, int dir_type = CF_TYPE_ANY);

/**
 * @brief Locks down the bitmap indexed by bitmapnum.
 *
 * @details Also converts the bitmap to the appropriate format specified by bpp and flags. Only lock a bitmap when you
 *   need it!
 *
 * @param handle    The number indexing the desired bitmap
 * @param bpp       The desired bpp of the bitmep
 * @param flags     The desired bitmap format
 * @param nodebug

 * @returns A pointer to the bitmap that's valid until bm_unlock is called if successful, or
 * @returns NULL if unsuccessful
 */
bitmap* bm_lock(int handle, int bpp, ushort flags, bool nodebug = false);

/**
 * @brief Returns the image type of the given bitmap handle
 */
BM_TYPE bm_get_type(int handle);

/**
 * @brief Unlocks a bitmap
 *
 * @details Decrements the ref_count member of the bitmap_entry struct, A bitmap can only be unloaded when the
 * ref_count is 0
 */
void bm_unlock(int handle);

/**
 * @brief Checks if the bitmap indexed by handle is valid
 *
 * @details Some early known false or out of range handles (such as negative) are checked within
 * an initial routine pass, followed by a more thorough check routine to ensure the
 * series of bitmaps within the bm_bitmaps[] structure have not become invalid.
 *
 * @returns Nonzero if valid, else
 * @returns Zero otherwise
 *
 * @todo z64 - Returned value is essentially a bool type, need to check all caller functions to see if it can safely
 *   be updated to reflect this
 */
int bm_is_valid(int handle);

/**
 * @brief Gets info on the bitmap indexed by handle.
 *
 * @param[out] w      If non-null, gets the width
 * @param[out] h      If non-null, gets the heigth
 * @param[out] flags  If non-null, gets the flags
 * @param[out] nframs If non-null, gets the nframes
 * @param[out] fps    If non-null, gets the fps
 *
 * @returns The handle on success, or
 * @returns The handle to the first frame on success, or
 * @returns -1 on failure
 */
int bm_get_info(int handle, int *w = nullptr, int * h = nullptr, ushort* flags = nullptr, int *nframes = nullptr, int *fps = nullptr);

/**
 * @brief Gets the filename of the bitmap indexed by handle
 *
 * @param[out] filename The filename of the bitmap, if successful. Is set to an empty string if unseccessful
 *
 * @todo z64 - maybe deprecate this in favor of an SCP_string version
 */
void bm_get_filename(int bitmapnum, char *filename);

/**
 * @brief Gets the filename of the bitmap indexed by handle, which must exist.
 *
 * @details If the bitmap does not exist (i.e. the handle is invalid), then an Assertion halts program execution.
 *
 * @returns A const char* to the filename of the bitmap
 *
 * @todo z64 - maybe deprecate this in favor of an SCP_string version.
 * @todo z64 - maybe combine this with the other bm_get_filename function like so:
 *   void bm_get_filename(int handle, SCP_string *filename, bool must_exist = false);
 */
const char *bm_get_filename(int handle);

/**
 * @brief Unloads all used bitmaps, should only ever be called by game_shutdown()
 *
 * @todo Maybe move this declaration into bmpman.cpp and then extern this function within game_shutdown() to
 *  ensure it only ever gets called there.
 */
void bm_unload_all();

/**
 * @brief Gets the palette for a given bitmap indexed by handle, and optionally the filename
 *
 * @param[out] pal Is set to reference the bitmap's palette
 * @param[out] name (optional) Is set to the bitmap's filename
 *
 * @todo Maybe get rid of the optional filename and have the callers call bm_get_filename. Less efficient, however.
 */
void bm_get_palette(int handle, ubyte *pal, char *name);

/**
 * @brief (DEBUG) Gets memory size, in bytes, of the locked bitmaps
 *
 * @details Both params are non-optional, and must be non-null. There's currently no safety checks on either.
 *
 * @param ntotal[out] Memory size, in bytes, of the locked bitmaps
 * @param nnew[out]   Memory size, in bytes, of the bitmaps that were locked since the last frame
 *
 * @note This is a debug function, and is undefined within release builds
 */
void bm_get_frame_usage(int *ntotal, int *nnew);

/**
 * @brief Reloads an existing bmpman slot with different bitmap
 *
 * @param[in] filename The filename of the image to load
 *
 * @returns The bitmap handle on success, or
 * @returns A negative value on failure
 *
 * @note This should only be used if you are certain the new picture is the same type, has same dimensions, etc.
 */
int bm_reload(int bitmap_handle, const char* filename);

/**
 * @brief Tells bmpman to start keeping track of what bitmaps are used where
 */
void bm_page_in_start();

/**
 * @brief Tells bmpman to stop paging (?)
 */
void bm_page_in_stop();

// Paging code in a library should call these functions
// in its page in function.

/**
 * @brief Marks a texture as being used for this level
 *
 * @param[in] num_frames If specified, assumes this is an animated texture
 */
void bm_page_in_texture(int bitmapnum, int num_frames = 0);

/**
 * @brief Marks a textures as being used for level and is transparant
 *
 * @param[in] num_frames If specified, assumes this is an animated texture
 */
void bm_page_in_xparent_texture(int bitmapnum, int num_frames = 1);

/**
 * @brief Marks a texture as being used for this level, and is anti-aliased
 *
 * @param[in] num_frames If specified, assumes this is an animated texture
 */
void bm_page_in_aabitmap(int bitmapnum, int num_frames = 1);

/**
 * @brief Sets BMPMAN's memory mode
 *
 * @details 0 = High memory;
 *          1 = Low memory (loads every other frame of ani's);
 *          2 = Debug low memory (only use first frame of each ani)
 *
 * @todo This should use an enum, or instead allow an arbitrary number to drop frames (like 1/2, 1/3, etc.)
 */
void bm_set_low_mem(int mode);

/**
 * @brief Sets bm_set_components and bm_get_components to reference screen format functions
 */
void BM_SELECT_SCREEN_FORMAT();

/**
 * @brief Sets bm_set_components and bm_get_components to reference texture format functions
 */
void BM_SELECT_TEX_FORMAT();

/**
 * @brief Sets bm_set_components and bm_get_components to reference texture format functions (with alpha)
 */
void BM_SELECT_ALPHA_TEX_FORMAT();

/**
 * @brief Functional pointer that references any of the bm_set_components functions.
 *
 * @details The bm_set_components functions packs the RGBA values into the ubyte array referenced by pixel, whose
 * format differs according to its bpp value and presence of an alpha channel. The RGBA values are scaled accordingly.
 *
 * @param[out] pixel The pixel to set
 * @param[in] r Red value   (may not be NULL)
 * @param[in] g Green value (may not be NULL)
 * @param[in] b Blue value  (may not be NULL)
 * @param[in] a Alpha value (currently ignored)
 *
 * @note z64 - These functions were made predating the introduction of C++ classes, and are basically the equivalent
 * of Pixel::set(ubyte *r, ubyte *b, ubyte *g). The original comment mentions that any of the rgba params may be
 * NULL, but this is by far _NOT_ the case, as a NULL value will cause undefined behavior (really really bad chroma
 * corruption)
 *
 * @note These functions assume the incoming bitdepth will always be >= to the outgoing bitdepth of the pixel. Should
 * the incoming bitdepth be lower, the outgoing values will appear darker than they should be
 */
extern void(*bm_set_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

/**
 * @brief Sets the 16bpp screen pixel to the specified RGBA value
 *
 * @see bm_set_components
 */
void bm_set_components_argb_16_screen(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

/**
 * @brief Sets the 16bpp texture pixel to the specified RGBA value
 *
 * @see bm_set_components
 */
void bm_set_components_argb_16_tex(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

/**
 * @brief Sets the 32bpp texture pixel to the specified RGBA value
 *
 * @see bm_set_components
 */
void bm_set_components_argb_32_tex(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

/**
 * @brief Gets the RGBA components of a pixel according to the selected mode
 *
 * @see BM_SELECT_SCREEN_FORMAT() BM_SELECT_TEX_FORMAT() BM_SELECT_ALPHA_TEX_FORMAT()
 */
void bm_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a);

extern int ENVMAP;	//this holds a reference to a map that is for environment mapping -Bobboau

/**
 * @brief Returns the compression type of the bitmap indexed by handle
 */
int bm_is_compressed(int handle);

/**
 * @brief Gets the correct TCACHE_TYPE for compressed graphics (uncompressed are assumed TCACHE_TYPE_NORMAL)
 */
int bm_get_tcache_type(int handle);


/**
 * @brief Gets the number of mipmaps of the indexed texture
 */
int bm_get_num_mipmaps(int handle);

/**
 * @brief Checks to see if the indexed bitmap has an alpha channel
 *
 * @note Currently just checks if the bitmap is 32bpp and is not a .PCX
 */
bool bm_has_alpha_channel(int handle);

/**
 * @brief (DEBUG) Prints all loaded bitmaps to an outwindow
 */
void bm_print_bitmaps();

/**
 * @brief Creates a render target as close to the desired resolution as possible.
 *
 * @returns the handle of an avilable render target if successful, or
 * @returns -1 if not successful
 *
 * @note BM_FLAG_RENDER_TARGET_STATIC are drawn once/infrequently, while BM_FLAG_RENDER_TARGET_DYNAMIC are drawn roughly once every frame
 */
int bm_make_render_target(int width, int height, int flags);

/**
 * @brief Checks to see if the given bitmap indexed by handle is a render target
 *
 * @returns The render type (BM_TYPE) if it is a render target, or
 * @returns 0 if it is not
 */
int bm_is_render_target(int handle);

/**
 * @brief (GR function) Calls gr_bm_set_render target for the given bitmap indexed by handle
 *
 * @returns true if successful, or
 * @returns false if unsuccessful
 */
bool bm_set_render_target(int handle, int face = -1);

/**
 * @brief Loads and parses an .EFF
 *
 * @param[in]  filename The filename of the .EFF
 * @param[in]  dir_type
 * @param[out] nframes (optional) If given, is set to the number of frames this .EFF has
 * @param[out] nfps    (optional) If given, is set to the fps of this .EFF
 * @param[out] key     (optional) If given, is set to the keyframe index of this .EFF
 * @param[out] type    (optional) If given, is set to the BM_TYPE of the .EFF
 *
 * @returns true If successful
 * @returns false If not successful
 */
bool bm_load_and_parse_eff(const char *filename, int dir_type, int *nframes, int *nfps, int *key, BM_TYPE *type);

/**
 * @brief Calculates & returns the current frame of an animation
 *
 * @param[in] frame1_handle  Handle of the animation
 * @param[in] elapsed_time   Time in seconds since the animation started playing
 * @param[in] loop           (optional) specifies if the animation should loop, default is false which causes animation to hold on the last frame
 * @param[in] divisor        (optional) if greater than zero, elapsed time will be divided by this value i.e. allows "percentage complete" to be used to determine the frame instead of the animations total time derived from fps * frames
 *
 * @returns current frame of the animation (range is zero to the number of frames minus one)
 */
int bm_get_anim_frame(const int frame1_handle, float elapsed_time, const float divisor = 0.0f, const bool loop = false);

/**
 * @brief Determines if the given handle could be used in a texture array
 *
 * @param handle The handle to query the value for
 * @return @c true if all frames have the same size, @c false otherwise. Always returns @c true for single images
 */
bool bm_is_texture_array(const int handle);

/**
 * @brief Gets the base frame of the specified animation
 *
 * @details The base frame is the frame of the animation that should be used to determine if two animation frames can be
 * used in the same draw call. Use this if you want to optimize your draw calls to make use of texture arrays.
 *
 * @param handle The handle of the animation to check. This may also be a single frame bitmap
 * @param num_frames Optionally return the number of frames in the animation
 * @return The bitmap handle of the base frame or -1 on error
 */
int bm_get_base_frame(const int handle, int* num_frames = nullptr);

/**
 * @brief Get the array index of the specified bitmap
 *
 * This should be used when passing the array index to the GPU (e.g. when building uniform structs or streaming particle
 * data)
 *
 * @param handle The handle of the bitmap
 * @return The index into the array
 */
int bm_get_array_index(const int handle);

/**
 * @brief Counts how many slots are used in bm_bitmaps
 *
 * This needs to iterate through all the slots in order to determine whether or not they're used, so shouldn't be used frivolously.
 *
 * @return The number of used slots
 */
int bmpman_count_bitmaps();

/**
 * @brief Counts how many slots are available to the bmpman system
 *
 * Since the number of slots is dynamic now, this should be used for determining the total amount of available slots at
 * the moment.
 *
 * @warning This is entirely for debugging and logging purposes. It should not be used in actual engine code.
 *
 * @return The number of available slots
 */
int bmpman_count_available_slots();

/**
 * @brief Checks if the given filename is a valid effect or texture file name
 *
 * @note At least one of @c single_frame or @c animation must be true since the result would make no sense otherwise. It
 * is also valid to set both parmeters to @c true.
 *
 * @todo This currently just tries to load the file but could be improved by not actually loading the file in the future.
 *
 * @param file The file name to check
 * @param single_frame If set to @c true then single frames are valid files
 * @param animation If set to @c true then animations are valid files
 * @return @c true if the file name is valid, @c false otherwise
 */
bool bm_validate_filename(const SCP_string& file, bool single_frame, bool animation);

SDL_Surface* bm_to_sdl_surface(int handle);

#endif
