/*
	Main code file for the new DirectX 8.1 and up graphics routines.
	Doing things this way makes sure that we don't break any of the existing code.

	Notes about coding the update:
	
	 *Copy what you need from [V]'s code - it's faster
	
	 *When you rewrite a function from the original, append an 8 after the 'd3d' so we and the compiler
	 can differentiate the functions - this is going to be completely separate so no overloading
	
	 *This is by no means the only file, just a starter one, it will essentially be identical to
	 [V]'s original structurally.

	 *By no means have I completely stripped all the files for their functions yet, this was just
	 added late at night. There will be problems initially.


	Post all updates to the developer mailing list or the forum, preferably in the DX8.1 thread

	##UnknownPlayer##
*/

// Commented out includes are those awaiting rewriting
#include <math.h>

#include "graphics/directx8/grd3d81internal.h"	// Updated to point to the correct include file

#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "osapi/osregistry.h"
#include "grd3d81.h"		// Updated to point to the include for this file
#include "graphics/line.h"
#include "graphics/font.h"
#include "graphics/grinternal.h"
#include "io/mouse.h"
#include "globalincs/alphacolors.h"
#include "globalincs/systemvars.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"

void gr_d3d8_init()
{

}

void gr_d3d8_cleanup()
{

}

void gr_d3d8_get_tex_format(int alpha)
{

}

void gr_d3d8_bitmap(int x, int y)
{

}

void gr_d3d8_bitmap_ex(int x, int y, int w, int h, int sx, int sy)
{

}

int gr_d3d8_create_rendering_objects(int clear)
{

}

void gr_d3d8_release_rendering_objects()
{

}


void gr_d3d8_set_initial_render_state()
{

}

char* d3d8_error_string(HRESULT error)
{
    switch(error) 
	{
        case D3DERR_BADMAJORVERSION:
            return "D3DERR_BADMAJORVERSION\0";
        case D3DERR_BADMINORVERSION:
            return "D3DERR_BADMINORVERSION\0";
        case D3DERR_EXECUTE_LOCKED:
            return "D3DERR_EXECUTE_LOCKED\0";
        case D3DERR_EXECUTE_NOT_LOCKED:
            return "D3DERR_EXECUTE_NOT_LOCKED\0";
        case D3DERR_EXECUTE_CREATE_FAILED:
            return "D3DERR_EXECUTE_CREATE_FAILED\0";
        case D3DERR_EXECUTE_DESTROY_FAILED:
            return "D3DERR_EXECUTE_DESTROY_FAILED\0";
        case D3DERR_EXECUTE_LOCK_FAILED:
            return "D3DERR_EXECUTE_LOCK_FAILED\0";
        case D3DERR_EXECUTE_UNLOCK_FAILED:
            return "D3DERR_EXECUTE_UNLOCK_FAILED\0";
        case D3DERR_EXECUTE_FAILED:
            return "D3DERR_EXECUTE_FAILED\0";
        case D3DERR_EXECUTE_CLIPPED_FAILED:
            return "D3DERR_EXECUTE_CLIPPED_FAILED\0";
        case D3DERR_TEXTURE_NO_SUPPORT:
            return "D3DERR_TEXTURE_NO_SUPPORT\0";
        case D3DERR_TEXTURE_NOT_LOCKED:
            return "D3DERR_TEXTURE_NOT_LOCKED\0";
        case D3DERR_TEXTURE_LOCKED:
            return "D3DERR_TEXTURELOCKED\0";
        case D3DERR_TEXTURE_CREATE_FAILED:
            return "D3DERR_TEXTURE_CREATE_FAILED\0";
        case D3DERR_TEXTURE_DESTROY_FAILED:
            return "D3DERR_TEXTURE_DESTROY_FAILED\0";
        case D3DERR_TEXTURE_LOCK_FAILED:
            return "D3DERR_TEXTURE_LOCK_FAILED\0";
        case D3DERR_TEXTURE_UNLOCK_FAILED:
            return "D3DERR_TEXTURE_UNLOCK_FAILED\0";
        case D3DERR_TEXTURE_LOAD_FAILED:
            return "D3DERR_TEXTURE_LOAD_FAILED\0";
        case D3DERR_MATRIX_CREATE_FAILED:
            return "D3DERR_MATRIX_CREATE_FAILED\0";
        case D3DERR_MATRIX_DESTROY_FAILED:
            return "D3DERR_MATRIX_DESTROY_FAILED\0";
        case D3DERR_MATRIX_SETDATA_FAILED:
            return "D3DERR_MATRIX_SETDATA_FAILED\0";
        case D3DERR_SETVIEWPORTDATA_FAILED:
            return "D3DERR_SETVIEWPORTDATA_FAILED\0";
        case D3DERR_MATERIAL_CREATE_FAILED:
            return "D3DERR_MATERIAL_CREATE_FAILED\0";
        case D3DERR_MATERIAL_DESTROY_FAILED:
            return "D3DERR_MATERIAL_DESTROY_FAILED\0";
        case D3DERR_MATERIAL_SETDATA_FAILED:
            return "D3DERR_MATERIAL_SETDATA_FAILED\0";
        case D3DERR_LIGHT_SET_FAILED:
            return "D3DERR_LIGHT_SET_FAILED\0";
        default:
            return "Unrecognized error value.\0";
    }
}
