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

#include "graphics/directx8/grd3d81.h"
#include "osapi/osapi.h"
#include "cmdline/cmdline.h"

bool D3D8_inited = false;

void gr_d3d8_init()
{
	int depth;
	// Initialize the D3DRENDER object and InitD3D
	if (Cmdline_force_32bit)
	{
		depth = 32;
	}
	else
	{
		depth = 16;
	}
	if (FAILED((D3D8RENDER::Instance())->InitD3D(gr_screen.max_w, gr_screen.max_h, depth ))
	{
		D3D8_inited = false;
		gr_d3d8_cleanup();
		return;
	}
	D3D8_inited = true;
	(D3D8RENDER::Instance())->AssignFunctions();	// Fill out the gr_screen structure with function pointers
}

void gr_d3d8_cleanup()
{
	if (!D3D8_inited)
	{
		return;
	}
	// Go ahead and delete the singleton - this must be invoked or we're going to get funkiness
	D3D8RENDER* prender = (D3D8RENDER::Instance());
	delete prender;
	D3D8_inited = false;
}

D3D8RENDER* D3D8RENDER::pInstance = 0;	// Initialize pointer
D3D8RENDER* D3D8RENDER::Instance()
{
	if (pInstance == 0)		// Is this the first call?
	{
		pInstance = new D3D8RENDER;		// Make a new object then.
	}
	return pInstance;					// Return a pointer to the object
}

D3D8RENDER::D3D8RENDER()
{
	
}

D3D8RENDER::~D3D8RENDER()
{
	lpD3DDevice->Release();
	lpD3D->Release();
}

/*
 *	This function initializes D3D - it is the equivalent of gr_d3d_initdevice
 *  This is the function in which to initialize features of D3D.
 *  For example, if we add an option for anti-aliasing, initialize it here.
 *	I have made some assumptions about what formats freespace expects and
 *	this code will explicitely search for formats with equal pixel allocation
 */
HRESULT D3D8RENDER::InitD3D(int screenx, int screeny, int depth)
{
	if (D3D8_inited)
	{
		return D3D_OK;
	}
	HWND hwnd;					// Stores the main window handle
	hwnd = (HWND)os_get_window();

	// Create the D3D8 object
	if (NULL == (lpD3D = Direct3DCreate8(D3D_SDK_VERSION)))
	{
		MessageBox(NULL, "Failed to create a D3D object", "##UnknownPlayer##", MB_OK);
		return E_FAIL;
	}

	// Check for windowed mode
	if (D3D_window)
	{
//		SetWindowPos(hwnd, HWND_TOP, 0, 0, gr_screen.max_w, gr_screen.max_h, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);
	}
	else
	{
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
//		The next line seems to hang my debugger, so its out for now
//		SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
	}
	
	// Maybe delete this ???
	D3D_cursor_clip_rect.left = 0;
	D3D_cursor_clip_rect.top = 0;
	D3D_cursor_clip_rect.right = gr_screen.max_w-1;
	D3D_cursor_clip_rect.bottom = gr_screen.max_h-1;
	
	D3DDISPLAYMODE d3ddm;			// D3D display mode structure
	D3DPRESENT_PARAMETERS d3dpp;	// Used to set the way D3D will display what we render
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	// It's here that we setup what type of display we want using d3dpp
	if (D3D_window)
	{
		// We must grab the current display settings here if we render to a window
		lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

		d3dpp.Windowed = true;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hwnd;
		d3dpp.BackBufferFormat = d3ddm.Format;
	}
	else
	{
		// TODO: This needs some way of setting the window to be fullscreen I think
		
		// Here we need to enumerate modes and find a match for the type of mode we want
		// Currently we look for matches to screenx, screeny, depth and then get the fastest refresh rate
		uint modecount = lpD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT);
		uint goodmode = NULL;		// Stores the mode we want to use in the end
		
		if (depth == 32)
		{
			// Search for a mode that's above 60hz and 32 bit
			for (uint i = 0; i < modecount; i++)
			{
				lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3ddm);
				if ((d3ddm.Height == screeny) && (d3ddm.Width == screenx) && (d3ddm.RefreshRate > 60) && (d3ddm.Format = D3DFMT_A8R8B8G8)
				{
					goodmode = i;
				}
			}

			// Just try and get a mode then...
			if (goodmode == NULL)
			{
				for (uint i = 0; i < modecount; i++)
				{
					lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3ddm);
					if ((d3ddm.Height == screeny) && (d3ddm.Width == screenx) && (d3ddm.Format = D3DFMT_A8R8B8G8)
					{
						goodmode = i;
					}
				}
			}
		}
		else
		{
			// Search for a mode that's above 60hz and 32 bit
			for (uint i = 0; i < modecount; i++)
			{
				lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3ddm);
				if ((d3ddm.Height == screeny) && (d3ddm.Width == screenx) && (d3ddm.RefreshRate > 60) && (d3ddm.Format = D3DFMT_A4R4G4B4)
				{
					goodmode = i;
				}
			}

			// Just try and get a mode then...
			if (goodmode == NULL)
			{
				for (uint i = 0; i < modecount; i++)
				{
					lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3ddm);
					if ((d3ddm.Height == screeny) && (d3ddm.Width == screenx) && (d3ddm.Format = D3DFMT_A4R4G4B4)
					{
						goodmode = i;
					}
				}
			}
		}

		// Did we fail to find a mode at all?
		if (goodmode == NULL)
		{
			MessageBox(NULL, "Could not find an appropriate full screen mode!", "Unknown Player", MB_OK);
			return D3DERR_NOTAVAILABLE;
		}

		// Get the properties of the mode we settled on
		lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, goodmode, &d3ddm);

		// Assign the mode to the properties it should be
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
		d3dpp.Windowed = false;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hwnd;
	}
	// Let D3D manage the depth and stencil buffers
	d3dpp.AutoDepthStencilFormat = D3DFMT_16;	// 16-bit z-buffer
	d3dpp.EnableAutoDepthStencil = true;
	
	// Create the D3D device here
	lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp,
		&lpD3DDevice);
	
	lpD3DDevice->GetDeviceCaps(&D3DCaps);		// Grab the device capabilities now
	
	// Viewport Stuff

// Old Viewport Code
/*	
	this sets up W data so the fogging can work
	extern float z_mult;
	set_wbuffer_planes(lpD3DDevice, 0.0f, z_mult);

	viewdata.dwSize = sizeof( viewdata );
	viewdata.dwX = viewdata.dwY = 0;
	viewdata.dwWidth = gr_screen.max_w;
	viewdata.dwHeight = gr_screen.max_h;
	viewdata.dvScaleX = largest_side / 2.0f;
	viewdata.dvScaleY = largest_side / 2.0f;
	viewdata.dvMaxX = ( float ) ( viewdata.dwWidth / ( 2.0F * viewdata.dvScaleX ) );
	viewdata.dvMaxY = ( float ) ( viewdata.dwHeight / ( 2.0F * viewdata.dvScaleY ) );
	viewdata.dvMinZ = 0.0f;
	viewdata.dvMaxZ = 1.0f; // choose something appropriate here!
*/
	ZeroMemory(&viewdata, sizeof(viewdata));
	viewdata.MaxZ = 1.0f;
	viewdata.MinZ = 0.0f;
	viewdata.Height = gr_screen.max_h;
	viewdata.Width = gr_screen.max_w;

	lpD3DDevice->SetViewport(&viewdata);

	return D3D_OK;
}

void D3D8RENDER::AssignFunctions()
{
	// All functions here should be encapsulated into the D3D8RENDER class as public
	// You need these stubs though to access them properly through the normal channels

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	gr_screen.gf_flip = gr_d3d8e_flip;
	gr_screen.gf_flip_window = gr_d3d8e_flip_window;
	gr_screen.gf_set_clip = gr_d3d8e_set_clip;
	gr_screen.gf_reset_clip = gr_d3d8e_reset_clip;
	gr_screen.gf_set_font = grx_set_font;

	gr_screen.gf_get_color = gr_d3d8e_get_color;
	gr_screen.gf_init_color = gr_d3d8e_init_color;
	gr_screen.gf_set_color_fast = gr_d3d8e_set_color_fast;
	gr_screen.gf_set_color = gr_d3d8e_set_color;
	gr_screen.gf_init_color = gr_d3d8e_init_color;
	gr_screen.gf_init_alphacolor = gr_d3d8e_init_alphacolor;

	gr_screen.gf_set_bitmap = gr_d3d8e_set_bitmap;
	gr_screen.gf_create_shader = gr_d3d8e_create_shader;
	gr_screen.gf_set_shader = gr_d3d8e_set_shader;
	gr_screen.gf_clear = gr_d3d8e_clear;
	// gr_screen.gf_bitmap = gr_d3d8e_bitmap;
	// gr_screen.gf_bitmap_ex = gr_d3d8e_bitmap_ex;
	gr_screen.gf_aabitmap = gr_d3d8e_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_d3d8e_aabitmap_ex;

	gr_screen.gf_rect = gr_d3d8e_rect;
	gr_screen.gf_shade = gr_d3d8e_shade;
	gr_screen.gf_string = gr_d3d8e_string;
	gr_screen.gf_circle = gr_d3d8e_circle;

	gr_screen.gf_line = gr_d3d8e_line;
	gr_screen.gf_aaline = gr_d3d8e_aaline;
	gr_screen.gf_pixel = gr_d3d8e_pixel;
	gr_screen.gf_scaler = gr_d3d8e_scaler;
	gr_screen.gf_aascaler = gr_d3d8e_aascaler;
	gr_screen.gf_tmapper = gr_d3d8e_tmapper;

	gr_screen.gf_gradient = gr_d3d8e_gradient;

	gr_screen.gf_set_palette = gr_d3d8e_set_palette;
	gr_screen.gf_print_screen = gr_d3d8e_print_screen;

	gr_screen.gf_fade_in = gr_d3d8e_fade_in;
	gr_screen.gf_fade_out = gr_d3d8e_fade_out;
	gr_screen.gf_flash = gr_d3d8e_flash;

	gr_screen.gf_zbuffer_get = gr_d3d8e_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_d3d8e_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_d3d8e_zbuffer_clear;

	gr_screen.gf_save_screen = gr_d3d8e_save_screen;
	gr_screen.gf_restore_screen = gr_d3d8e_restore_screen;
	gr_screen.gf_free_screen = gr_d3d8e_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_d3d8e_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_d3d8e_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_d3d8e_dump_frame;

	gr_screen.gf_set_gamma = gr_d3d8e_set_gamma;

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_d3d8e_lock;
	gr_screen.gf_unlock = gr_d3d8e_unlock;

	// screen region
	gr_screen.gf_get_region = gr_d3d8e_get_region;

	// fog stuff
	gr_screen.gf_fog_set = gr_d3d8e_fog_set;

	// pixel get
	gr_screen.gf_get_pixel = gr_d3d8e_get_pixel;

	// poly culling
	gr_screen.gf_set_cull = gr_d3d8e_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_d3d8e_cross_fade;

	// filtering
	gr_screen.gf_filter_set = gr_d3d8e_filter_set;

	// texture cache
	gr_screen.gf_tcache_set = d3d8e_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_d3d8e_set_clear_color;
	
}

void D3D8RENDER::gr_d3d8_flash(int r, int g, int b)
{

}

void D3D8RENDER::gr_d3d8_zbuffer_clear(int mode)
{

}

int D3D8RENDER::gr_d3d8_zbuffer_get()
{
	return 1;
}

int D3D8RENDER::gr_d3d8_zbuffer_set(int mode)
{
	return 1;
}

void D3D8RENDER::gr_d3d8_tmapper( int nverts, vertex **verts, uint flags )
{

}

void D3D8RENDER::gr_d3d8_scaler(vertex *va, vertex *vb )
{

}

void D3D8RENDER::gr_d3d8_aascaler(vertex *va, vertex *vb )
{

}

void D3D8RENDER::gr_d3d8_pixel(int x, int y)
{

}

void D3D8RENDER::gr_d3d8_clear()
{

}

void D3D8RENDER::gr_d3d8_set_clip(int x,int y,int w,int h)
{

}
void D3D8RENDER::gr_d3d8_reset_clip()
{

}

void D3D8RENDER::gr_d3d8_init_color(color *c, int r, int g, int b)
{

}

void D3D8RENDER::gr_d3d8_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{

}

void D3D8RENDER::gr_d3d8_set_color( int r, int g, int b )
{

}

void D3D8RENDER::gr_d3d8_get_color( int * r, int * g, int * b )
{

}

void D3D8RENDER::gr_d3d8_set_color_fast(color *dst)
{

}

void D3D8RENDER::gr_d3d8_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha, int sx, int sy )
{

}
void D3D8RENDER::gr_d3d8_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{

}

void D3D8RENDER::gr_d3d8_bitmap(int x, int y)
{

}

void D3D8RENDER::gr_d3d8_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{

}

void D3D8RENDER::gr_d3d8_aabitmap(int x, int y)
{

}

void D3D8RENDER::gr_d3d8_rect(int x,int y,int w,int h)
{

}

void D3D8RENDER::gr_d3d8_create_shader(shader * shade, float r, float g, float b, float c )
{

}

void D3D8RENDER::gr_d3d8_set_shader( shader * shade )
{

}

void D3D8RENDER::gr_d3d8_shade(int x,int y,int w,int h)
{

}

void D3D8RENDER::gr_d3d8_create_font_bitmap()
{

}

void D3D8RENDER::gr_d3d8_char(int x,int y,int letter)
{

}

void D3D8RENDER::gr_d3d8_string( int sx, int sy, char *s )
{

}

void D3D8RENDER::gr_d3d8_circle( int xc, int yc, int d )
{

}

void D3D8RENDER::gr_d3d8_line(int x1,int y1,int x2,int y2)
{

}

void D3D8RENDER::gr_d3d8_aaline(vertex *v1, vertex *v2)
{

}

void D3D8RENDER::gr_d3d8_gradient(int x1,int y1,int x2,int y2)
{

}

void D3D8RENDER::gr_d3d8_set_palette(ubyte *new_palette, int restrict_alphacolor)
{

}

void D3D8RENDER::gr_d3d8_diamond(int x, int y, int width, int height)
{

}

void D3D8RENDER::gr_d3d8_print_screen(char *filename)
{

}

void D3D8RENDER::d3d8_tcache_init(int use_sections)
{

}

void D3D8RENDER::d3d8_tcache_cleanup()
{

}

void D3D8RENDER::d3d8_tcache_flush()
{

}

void D3D8RENDER::d3d8_tcache_frame()
{

}

void D3D8RENDER::d3d8_flush()
{

}

int D3D8RENDER::d3d8_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full, int sx, int sy, int force )
{
	return 1;
}

void D3D8RENDER::d3d8_dump_frame()
{

}

void D3D8RENDER::gr_d3d8_flip()
{

}

void D3D8RENDER::gr_d3d8_flip_cleanup()
{

}

void D3D8RENDER::gr_d3d8_flip_window(uint _hdc, int x, int y, int w, int h)
{

}

void D3D8RENDER::gr_d3d8_fade_out(int instantaneous)
{

}

void D3D8RENDER::gr_d3d8_fade_in(int instantaneous)
{

}

int D3D8RENDER::gr_d3d8_save_screen()
{
	return 1;
}

void D3D8RENDER::gr_d3d8_restore_screen(int id)
{

}

void D3D8RENDER::gr_d3d8_free_screen(int id)
{

}

void D3D8RENDER::gr_d3d8_dump_frame_start(int first_frame, int frames_between_dumps)
{

}

void D3D8RENDER::gr_d3d8_dump_frame_stop()
{

}

void D3D8RENDER::gr_d3d8_set_gamma(float gamma)
{

}

uint D3D8RENDER::gr_d3d8_lock()
{
	return 1;
}

void D3D8RENDER::gr_d3d8_unlock()
{/* VOID FUNCTION */}

void D3D8RENDER::gr_d3d8_get_region(int front, int w, int h, ubyte *data)
{

}

void D3D8RENDER::gr_d3d8_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{

}

void D3D8RENDER::gr_d3d8_get_pixel(int x, int y, int *r, int *g, int *b)
{

}

void D3D8RENDER::gr_d3d8_set_cull(int cull)
{

}

void D3D8RENDER::gr_d3d8_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{

}

void D3D8RENDER::gr_d3d8_filter_set(int filter)
{

}

void D3D8RENDER::gr_d3d8_set_clear_color(int r, int g, int b)
{

}

void D3D8RENDER::StartD3D8Frame()
{
	if (!D3D8_inited)
	{
		return;
	}

	if (in_frame)
	{
		return;
	}

	lpD3DDevice->BeginScene();
}

void D3D8RENDER::EndD3D8Frame()
{
	if (!D3D8_inited)
	{
		return;
	}

	if (!in_frame)
	{
		return;
	}

	lpD3DDevice->EndScene();
}