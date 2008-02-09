/*
 * $Logfile: /Freespace2/code/glide/Glide.cpp $
 * $Revision: 2.2 $
 * $Date: 2002-08-01 01:41:04 $
 * $Author: penguin $
 *
 * Code for dynamically loading glide libraries
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 4     3/18/98 1:57p John
 * Made non-perspective correct textures do 2d clipping before rendering.
 * 
 * 3     2/21/98 11:58a John
 * Put in some stuff to externalize strings
 * 
 * 2     2/19/98 6:13p John
 * Made Glide do texturing & zbuffering.
 * 
 * 1     1/05/98 10:38a John
 *
 * $NoKeywords: $
 */

#include <windows.h>
#include <windowsx.h>

#include "glide/glide.h"
#include "globalincs/pstypes.h"

static HMODULE Glide_lib_handle;

void vglide_close()
{
	if ( Glide_lib_handle )	{
		FreeLibrary(Glide_lib_handle);
		Glide_lib_handle = NULL;
	}	
}

// Returns 1 if Glide found, else 0 if not
int vglide_init()
{
	HMODULE lib_handle;
	Glide_lib_handle = LoadLibrary(NOX("glide2x.dll"));
	if (Glide_lib_handle == NULL) {
		//No lib so no glide
		return 0;
	}

	lib_handle = Glide_lib_handle;

	//XSTR:OFF

	grDrawLine = (void (__stdcall*)(const GrVertex *, const GrVertex *))GetProcAddress(lib_handle, "_grDrawLine@8");
	grDrawPlanarPolygon = (void (__stdcall *)(int,const int [],const GrVertex []))GetProcAddress(lib_handle, "_grDrawPlanarPolygon@12");
	grDrawPlanarPolygonVertexList = (void (__stdcall *)(int,const GrVertex []))GetProcAddress(lib_handle, "_grDrawPlanarPolygonVertexList@8");
	grDrawPoint = (void (__stdcall *)(const GrVertex *))GetProcAddress(lib_handle, "_grDrawPoint@4");
	grDrawPolygon = (void (__stdcall *)(int,const int [],const GrVertex []))GetProcAddress(lib_handle, "_grDrawPolygon@12");
	grDrawPolygonVertexList = (void (__stdcall *)(int,const GrVertex []))GetProcAddress(lib_handle, "_grDrawPolygonVertexList@8");
	grDrawTriangle = (void (__stdcall *)(const GrVertex *,const GrVertex *,const GrVertex *))GetProcAddress(lib_handle, "_grDrawTriangle@12");
	guDrawTriangleWithClip = (void (__stdcall *)(const GrVertex *,const GrVertex *,const GrVertex *))GetProcAddress(lib_handle, "_guDrawTriangleWithClip@12");
	grBufferClear = (void (__stdcall *)(unsigned long,unsigned char,unsigned short))GetProcAddress(lib_handle, "_grBufferClear@12");
	grBufferNumPending = GetProcAddress(lib_handle, "_grBufferNumPending@0");
	grBufferSwap = (void (__stdcall *)(int))GetProcAddress(lib_handle, "_grBufferSwap@4");
	grRenderBuffer = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grRenderBuffer@4");

	grErrorSetCallback = (void (__stdcall *)(void (__cdecl *)(const char *,int)))GetProcAddress(lib_handle, "_grErrorSetCallback@4");
	grSstIdle = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstIdle@0");
	grSstVideoLine = (unsigned long (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstVideoLine@0");
	grSstVRetraceOn = (int (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstVRetraceOn@0");
	grSstIsBusy = (int (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstIsBusy@0");
	grSstWinOpen = (int (__stdcall *)(unsigned long,long,long,long,long,int,int))GetProcAddress(lib_handle, "_grSstWinOpen@28");
	grSstWinClose = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstWinClose@0");
	grSstControl = (int (__stdcall *)(unsigned long))GetProcAddress(lib_handle, "_grSstControl@4");
	grSstQueryHardware = (int (__stdcall *)(GrHwConfiguration *))GetProcAddress(lib_handle, "_grSstQueryHardware@4");
	grSstQueryBoards = (int (__stdcall *)(GrHwConfiguration *))GetProcAddress(lib_handle, "_grSstQueryBoards@4");

	grSstOrigin = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grSstOrigin@4");
	grSstSelect = (void (__stdcall *)(int))GetProcAddress(lib_handle, "_grSstSelect@4");
	grSstScreenHeight = (unsigned long (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstScreenHeight@0");
	grSstScreenWidth = (unsigned long (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstScreenWidth@0");
	grSstStatus = (unsigned long (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstStatus@0");
	grSstPerfStats = (void (__stdcall *)(struct GrSstPerfStats_s *))GetProcAddress(lib_handle, "_grSstPerfStats@4");
	grSstResetPerfStats = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grSstResetPerfStats@0");
	grResetTriStats = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grResetTriStats@0");
	grTriStats = (void (__stdcall *)(unsigned long *,unsigned long *))GetProcAddress(lib_handle, "_grTriStats@8");
	grAlphaBlendFunction = (void (__stdcall *)(long,long,long,long))GetProcAddress(lib_handle, "_grAlphaBlendFunction@16");

	grAlphaCombine = (void (__stdcall *)(long,long,long,long,int))GetProcAddress(lib_handle, "_grAlphaCombine@20");
	grAlphaControlsITRGBLighting = (void (__stdcall *)(int))GetProcAddress(lib_handle, "_grAlphaControlsITRGBLighting@4");
	grAlphaTestFunction = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grAlphaTestFunction@4");
	grAlphaTestReferenceValue = (void (__stdcall *)(unsigned char))GetProcAddress(lib_handle, "_grAlphaTestReferenceValue@4");
	grChromakeyMode = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grChromakeyMode@4");
	grChromakeyValue = (void (__stdcall *)(unsigned long))GetProcAddress(lib_handle, "_grChromakeyValue@4");
	grClipWindow = (void (__stdcall *)(unsigned long,unsigned long,unsigned long,unsigned long))GetProcAddress(lib_handle, "_grClipWindow@16");

	grColorCombine = (void (__stdcall *)(long,long,long,long,int))GetProcAddress(lib_handle, "_grColorCombine@20");
	grColorMask = (void (__stdcall *)(int,int))GetProcAddress(lib_handle, "_grColorMask@8");
	grCullMode = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grCullMode@4");
	grConstantColorValue = (void (__stdcall *)(unsigned long))GetProcAddress(lib_handle, "_grConstantColorValue@4");
	grConstantColorValue4 = (void (__stdcall *)(float,float,float,float))GetProcAddress(lib_handle, "_grConstantColorValue4@16");
	grDepthBiasLevel = (void (__stdcall *)(short))GetProcAddress(lib_handle, "_grDepthBiasLevel@4");
	grDepthBufferFunction = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grDepthBufferFunction@4");
	grDepthBufferMode = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grDepthBufferMode@4");
	grDepthMask = (void (__stdcall *)(int))GetProcAddress(lib_handle, "_grDepthMask@4");
	grDisableAllEffects = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grDisableAllEffects@0");
	grDitherMode = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grDitherMode@4");

	grFogColorValue = (void (__stdcall *)(unsigned long))GetProcAddress(lib_handle, "_grFogColorValue@4");
	grFogMode = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grFogMode@4");
	grFogTable = (void (__stdcall *)(const unsigned char []))GetProcAddress(lib_handle, "_grFogTable@4");
	grGammaCorrectionValue = (void (__stdcall *)(float))GetProcAddress(lib_handle, "_grGammaCorrectionValue@4");
	grSplash = (void (__stdcall *)(float,float,float,float,unsigned long))GetProcAddress(lib_handle, "_grSplash@20");

	grTexCalcMemRequired = (unsigned long (__stdcall *)(long,long,long,long))GetProcAddress(lib_handle, "_grTexCalcMemRequired@16");
	grTexTextureMemRequired = (unsigned long (__stdcall *)(unsigned long,GrTexInfo *))GetProcAddress(lib_handle, "_grTexTextureMemRequired@8");
	grTexMinAddress = (unsigned long (__stdcall *)(long))GetProcAddress(lib_handle, "_grTexMinAddress@4");
	grTexMaxAddress = (unsigned long (__stdcall *)(long))GetProcAddress(lib_handle, "_grTexMaxAddress@4");
	grTexNCCTable = (void (__stdcall *)(long,unsigned long))GetProcAddress(lib_handle, "_grTexNCCTable@8");
	grTexSource = (void (__stdcall *)(long,unsigned long,unsigned long,GrTexInfo *))GetProcAddress(lib_handle, "_grTexSource@16");
	grTexClampMode = (void (__stdcall *)(long,long,long))GetProcAddress(lib_handle, "_grTexClampMode@12");
	grTexCombine = (void (__stdcall *)(long,long,long,long,long,int,int))GetProcAddress(lib_handle, "_grTexCombine@28");
	grTexCombineFunction = (void (__stdcall *)(long,long))GetProcAddress(lib_handle, "_grTexCombineFunction@8");
	grTexDetailControl = (void (__stdcall *)(long,int,unsigned char,float))GetProcAddress(lib_handle, "_grTexDetailControl@16");
	grTexFilterMode = (void (__stdcall *)(long,long,long))GetProcAddress(lib_handle, "_grTexFilterMode@12");
	grTexLodBiasValue = (void (__stdcall *)(long,float))GetProcAddress(lib_handle, "_grTexLodBiasValue@8");
	grTexDownloadMipMap = (void (__stdcall *)(long,unsigned long,unsigned long,GrTexInfo *))GetProcAddress(lib_handle, "_grTexDownloadMipMap@16");
	grTexDownloadMipMapLevel = (void (__stdcall *)(long,unsigned long,long,long,long,long,unsigned long,void *))GetProcAddress(lib_handle, "_grTexDownloadMipMapLevel@32");
	grTexDownloadMipMapLevelPartial = (void (__stdcall *)(long,unsigned long,long,long,long,long,unsigned long,void *,int,int))GetProcAddress(lib_handle, "_grTexDownloadMipMapLevelPartial@40");
	ConvertAndDownloadRle = (void (__stdcall *)(long,unsigned long,long,long,long,long,unsigned long,unsigned char *,long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned short *))GetProcAddress(lib_handle, "_ConvertAndDownloadRle@64");

	grCheckForRoom = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grCheckForRoom@4");
	grTexDownloadTable = (void (__stdcall *)(long,unsigned long,void *))GetProcAddress(lib_handle, "_grTexDownloadTable@12");
	grTexDownloadTablePartial = (void (__stdcall *)(long,unsigned long,void *,int,int))GetProcAddress(lib_handle, "_grTexDownloadTablePartial@20");
	grTexMipMapMode = (void (__stdcall *)(long,long,int))GetProcAddress(lib_handle, "_grTexMipMapMode@12");
	grTexMultibase = (void (__stdcall *)(long,int))GetProcAddress(lib_handle, "_grTexMultibase@8");
	grTexMultibaseAddress = (void (__stdcall *)(long,unsigned long,unsigned long,unsigned long,GrTexInfo *))GetProcAddress(lib_handle, "_grTexMultibaseAddress@20");

	guTexAllocateMemory = (unsigned long (__stdcall *)(long,unsigned char,int,int,long,long,long,long,long,long,long,long,long,float,int))GetProcAddress(lib_handle, "_guTexAllocateMemory@60");
	guTexChangeAttributes = (int (__stdcall *)(unsigned long,int,int,long,long,long,long,long,long,long,long,long))GetProcAddress(lib_handle, "_guTexChangeAttributes@48");
	guTexCombineFunction = (void (__stdcall *)(long,long))GetProcAddress(lib_handle, "_guTexCombineFunction@8");
	guTexGetCurrentMipMap = (unsigned long (__stdcall *)(long))GetProcAddress(lib_handle, "_guTexGetCurrentMipMap@4");
	guTexGetMipMapInfo = (GrMipMapInfo *(__stdcall *)(unsigned long))GetProcAddress(lib_handle, "_guTexGetMipMapInfo@4");
	guTexMemQueryAvail = (unsigned long (__stdcall *)(long))GetProcAddress(lib_handle, "_guTexMemQueryAvail@4");
	guTexMemReset = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_guTexMemReset@0");
	guTexDownloadMipMap = (void (__stdcall *)(unsigned long,const void *,const GuNccTable *))GetProcAddress(lib_handle, "_guTexDownloadMipMap@12");
	guTexDownloadMipMapLevel = (void (__stdcall *)(unsigned long,long,const void ** ))GetProcAddress(lib_handle, "_guTexDownloadMipMapLevel@12");
	guTexSource = (void (__stdcall *)(unsigned long))GetProcAddress(lib_handle, "_guTexSource@4");
	guColorCombineFunction = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_guColorCombineFunction@4");

	grLfbLock = (int (__stdcall *)(unsigned long,long,long,long,int,GrLfbInfo_t *))GetProcAddress(lib_handle, "_grLfbLock@24");
	grLfbUnlock = (int (__stdcall *)(unsigned long,long))GetProcAddress(lib_handle, "_grLfbUnlock@8");
	grLfbConstantAlpha = (void (__stdcall *)(unsigned char))GetProcAddress(lib_handle, "_grLfbConstantAlpha@4");
	grLfbConstantDepth = (void (__stdcall *)(unsigned short))GetProcAddress(lib_handle, "_grLfbConstantDepth@4");
	grLfbWriteColorSwizzle = (void (__stdcall *)(int,int))GetProcAddress(lib_handle, "_grLfbWriteColorSwizzle@8");
	grLfbWriteColorFormat = (void (__stdcall *)(long))GetProcAddress(lib_handle, "_grLfbWriteColorFormat@4");
	grLfbWriteRegion = (int (__stdcall *)(long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,long,void *))GetProcAddress(lib_handle, "_grLfbWriteRegion@32");
	grLfbReadRegion = (int (__stdcall *)(long,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long,void *))GetProcAddress(lib_handle, "_grLfbReadRegion@28");

	grAADrawLine = (void (__stdcall *)(const GrVertex *,const GrVertex *))GetProcAddress(lib_handle, "_grAADrawLine@8");
	grAADrawPoint = (void (__stdcall *)(const GrVertex *))GetProcAddress(lib_handle, "_grAADrawPoint@4");
	grAADrawPolygon = (void (__stdcall *)(const int,const int [],const GrVertex []))GetProcAddress(lib_handle, "_grAADrawPolygon@12");
	grAADrawPolygonVertexList = (void (__stdcall *)(const int,const GrVertex []))GetProcAddress(lib_handle, "_grAADrawPolygonVertexList@8");
	grAADrawTriangle = (void (__stdcall *)(const GrVertex *,const GrVertex *,const GrVertex *,int,int,int))GetProcAddress(lib_handle, "_grAADrawTriangle@24");

	grGlideInit = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grGlideInit@0");
	grGlideShutdown = (void (__stdcall *)(void))GetProcAddress(lib_handle, "_grGlideShutdown@0");
	grGlideGetVersion = (void (__stdcall *)(char []))GetProcAddress(lib_handle, "_grGlideGetVersion@4");
	grGlideGetState = (void (__stdcall *)(struct _GrState_s *))GetProcAddress(lib_handle, "_grGlideGetState@4");
	grGlideSetState = (void (__stdcall *)(const struct _GrState_s *))GetProcAddress(lib_handle, "_grGlideSetState@4");
	grGlideShamelessPlug = (void (__stdcall *)(const int))GetProcAddress(lib_handle, "_grGlideShamelessPlug@4");
	grHints = (void (__stdcall *)(unsigned long,unsigned long))GetProcAddress(lib_handle, "_grHints@8");

	guFogTableIndexToW = (float (__stdcall *)(int))GetProcAddress(lib_handle, "_guFogTableIndexToW@4");
	guFogGenerateExp = (void (__stdcall *)( GrFog_t fogtable[GR_FOG_TABLE_SIZE], float))GetProcAddress(lib_handle, "_guFogGenerateExp@8");
	guFogGenerateExp2 = (void (__stdcall *)(GrFog_t fogtable[GR_FOG_TABLE_SIZE], float))GetProcAddress(lib_handle, "_guFogGenerateExp2@8");
	guFogGenerateLinear = (void (__stdcall *)(GrFog_t fogtable[GR_FOG_TABLE_SIZE], float , float))GetProcAddress(lib_handle, "_guFogGenerateLinear@12");

	//XSTR:ON

	return 1;
}


void (__stdcall *grDrawLine)(const GrVertex *v1, const GrVertex *v2) = NULL;
void (__stdcall *grDrawPlanarPolygon)(int nverts, const int ilist[], const GrVertex vlist[]) = NULL;
void (__stdcall *grDrawPlanarPolygonVertexList)(int nverts, const GrVertex vlist[]) = NULL;
void (__stdcall *grDrawPoint)(const GrVertex *pt) = NULL;
void (__stdcall *grDrawPolygon)(int nverts, const int ilist[], const GrVertex vlist[]) = NULL;
void (__stdcall *grDrawPolygonVertexList)(int nverts, const GrVertex vlist[]) = NULL;
void (__stdcall *grDrawTriangle)(const GrVertex *a, const GrVertex *b, const GrVertex *c) = NULL;
void (__stdcall *guDrawTriangleWithClip)(const GrVertex *a, const GrVertex *b, const GrVertex *c) = NULL;
void (__stdcall *grBufferClear)(GrColor_t color, GrAlpha_t alpha, FxU16 depth) = NULL;
int (__stdcall *grBufferNumPending)(void) = NULL;
void (__stdcall *grBufferSwap)(int swap_interval) = NULL;
void (__stdcall *grRenderBuffer)(GrBuffer_t buffer) = NULL;

typedef void (*GrErrorCallbackFnc_t)(const char *string, FxBool fatal);

void (__stdcall *grErrorSetCallback)(GrErrorCallbackFnc_t fnc) = NULL;
void (__stdcall *grSstIdle)(void) = NULL;
FxU32 (__stdcall *grSstVideoLine)(void) = NULL;
FxBool (__stdcall *grSstVRetraceOn)(void) = NULL;
FxBool (__stdcall *grSstIsBusy)(void) = NULL;
FxBool (__stdcall *grSstWinOpen)(
          FxU32                hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int                  nColBuffers,
          int                  nAuxBuffers) = NULL;
void (__stdcall *grSstWinClose)(void) = NULL;
FxBool (__stdcall *grSstControl)(FxU32 code) = NULL;
FxBool (__stdcall *grSstQueryHardware)(GrHwConfiguration *hwconfig) = NULL;
FxBool (__stdcall *grSstQueryBoards)(GrHwConfiguration *hwconfig) = NULL;
void (__stdcall *grSstOrigin)(GrOriginLocation_t origin) = NULL;
void (__stdcall *grSstSelect)(int which_sst) = NULL;
FxU32 (__stdcall *grSstScreenHeight)(void) = NULL;
FxU32 (__stdcall *grSstScreenWidth)(void) = NULL;
FxU32 (__stdcall *grSstStatus)(void) = NULL;
void (__stdcall *grSstPerfStats)(GrSstPerfStats_t *pStats) = NULL;
void (__stdcall *grSstResetPerfStats)(void) = NULL;
void (__stdcall *grResetTriStats)() = NULL;
void (__stdcall *grTriStats)(FxU32 *trisProcessed, FxU32 *trisDrawn) = NULL;
void (__stdcall *grAlphaBlendFunction)(GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df) = NULL;
void (__stdcall *grAlphaCombine)(GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert) = NULL;
void (__stdcall *grAlphaControlsITRGBLighting)(FxBool enable) = NULL;
void (__stdcall *grAlphaTestFunction)(GrCmpFnc_t function) = NULL;
void (__stdcall *grAlphaTestReferenceValue)(GrAlpha_t value) = NULL;
void (__stdcall *grChromakeyMode)(GrChromakeyMode_t mode) = NULL;
void (__stdcall *grChromakeyValue)(GrColor_t value) = NULL;
void (__stdcall *grClipWindow)(FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy) = NULL;
void (__stdcall *grColorCombine)(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert) = NULL;
void (__stdcall *grColorMask)(FxBool rgb, FxBool a) = NULL;
void (__stdcall *grCullMode)(GrCullMode_t mode) = NULL;
void (__stdcall *grConstantColorValue)(GrColor_t value) = NULL;
void (__stdcall *grConstantColorValue4)(float a, float r, float g, float b) = NULL;
void (__stdcall *grDepthBiasLevel)(FxI16 level) = NULL;
void (__stdcall *grDepthBufferFunction)(GrCmpFnc_t function) = NULL;
void (__stdcall *grDepthBufferMode)(GrDepthBufferMode_t mode) = NULL;
void (__stdcall *grDepthMask)(FxBool mask) = NULL;
void (__stdcall *grDisableAllEffects)(void) = NULL;
void (__stdcall *grDitherMode)( GrDitherMode_t mode) = NULL;
void (__stdcall *grFogColorValue)( GrColor_t fogcolor) = NULL;
void (__stdcall *grFogMode)(GrFogMode_t mode) = NULL;
void (__stdcall *grFogTable)(const GrFog_t ft[GR_FOG_TABLE_SIZE]) = NULL;
void (__stdcall *grGammaCorrectionValue)(float value) = NULL;
void (__stdcall *grSplash)(float x, float y, float width, float height, FxU32 frame) = NULL;
FxU32 (__stdcall *grTexCalcMemRequired)(
                     GrLOD_t lodmin, GrLOD_t lodmax,
                     GrAspectRatio_t aspect, GrTextureFormat_t fmt) = NULL;
FxU32 (__stdcall *grTexTextureMemRequired)(FxU32 evenOdd, GrTexInfo *info) = NULL;
FxU32 (__stdcall *grTexMinAddress)(GrChipID_t tmu) = NULL;
FxU32 (__stdcall *grTexMaxAddress)(GrChipID_t tmu) = NULL;
void (__stdcall *grTexNCCTable)(GrChipID_t tmu, GrNCCTable_t table) = NULL;
void (__stdcall *grTexSource)(GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo  *info) = NULL;
void (__stdcall *grTexClampMode)(GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode) = NULL;
void (__stdcall *grTexCombine)(
             GrChipID_t tmu,
             GrCombineFunction_t rgb_function,
             GrCombineFactor_t rgb_factor, 
             GrCombineFunction_t alpha_function,
             GrCombineFactor_t alpha_factor,
             FxBool rgb_invert,
             FxBool alpha_invert
             ) = NULL;
void (__stdcall *grTexCombineFunction)(
                     GrChipID_t tmu,
                     GrTextureCombineFnc_t fnc
                     ) = NULL;
void (__stdcall *grTexDetailControl)(
                   GrChipID_t tmu,
                   int lod_bias,
                   FxU8 detail_scale,
                   float detail_max
                   ) = NULL;
void (__stdcall *grTexFilterMode)(
                GrChipID_t tmu,
                GrTextureFilterMode_t minfilter_mode,
                GrTextureFilterMode_t magfilter_mode
                ) = NULL;
void (__stdcall *grTexLodBiasValue)(GrChipID_t tmu, float bias) = NULL;
void (__stdcall *grTexDownloadMipMap)(GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info) = NULL;

void (__stdcall *grTexDownloadMipMapLevel)(GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data) = NULL;

void (__stdcall *grTexDownloadMipMapLevelPartial)(GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end) = NULL;


void (__stdcall *ConvertAndDownloadRle)(GrChipID_t        tmu,
                        FxU32             startAddress,
                        GrLOD_t           thisLod,
                        GrLOD_t           largeLod,
                        GrAspectRatio_t   aspectRatio,
                        GrTextureFormat_t format,
                        FxU32             evenOdd,
                        FxU8              *bm_data,
                        long              bm_h,
                        FxU32             u0,
                        FxU32             v0,
                        FxU32             width,
                        FxU32             height,
                        FxU32             dest_width,
                        FxU32             dest_height,
                        FxU16             *tlut) = NULL;

void (__stdcall *grCheckForRoom)(FxI32 n);

void (__stdcall *grTexDownloadTable)(GrChipID_t   tmu,
                    GrTexTable_t type, 
                    void         *data) = NULL;

void (__stdcall *grTexDownloadTablePartial)(GrChipID_t   tmu,
                           GrTexTable_t type, 
                           void         *data,
                           int          start,
                           int          end) = NULL;

void (__stdcall *grTexMipMapMode)(GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend) = NULL;

void (__stdcall *grTexMultibase)(GrChipID_t tmu,
                FxBool     enable) = NULL;

void (__stdcall *grTexMultibaseAddress)(GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info) = NULL;

GrMipMapId_t (__stdcall *guTexAllocateMemory)(
                    GrChipID_t tmu,
                    FxU8 odd_even_mask,
                    int width, int height,
                    GrTextureFormat_t fmt,
                    GrMipMapMode_t mm_mode,
                    GrLOD_t smallest_lod, GrLOD_t largest_lod,
                    GrAspectRatio_t aspect,
                    GrTextureClampMode_t s_clamp_mode,
                    GrTextureClampMode_t t_clamp_mode,
                    GrTextureFilterMode_t minfilter_mode,
                    GrTextureFilterMode_t magfilter_mode,
                    float lod_bias,
                    FxBool trilinear
                    ) = NULL;

FxBool (__stdcall *guTexChangeAttributes)(
                      GrMipMapId_t mmid,
                      int width, int height,
                      GrTextureFormat_t fmt,
                      GrMipMapMode_t mm_mode,
                      GrLOD_t smallest_lod, GrLOD_t largest_lod,
                      GrAspectRatio_t aspect,
                      GrTextureClampMode_t s_clamp_mode,
                      GrTextureClampMode_t t_clamp_mode,
                      GrTextureFilterMode_t minFilterMode,
                      GrTextureFilterMode_t magFilterMode
                      ) = NULL;
void (__stdcall *guTexCombineFunction)(
                     GrChipID_t tmu,
                     GrTextureCombineFnc_t fnc
                     ) = NULL;
GrMipMapId_t (__stdcall *guTexGetCurrentMipMap)(GrChipID_t tmu) = NULL;
GrMipMapInfo* (__stdcall *guTexGetMipMapInfo)(GrMipMapId_t mmid);
FxU32 (__stdcall *guTexMemQueryAvail)(GrChipID_t tmu) = NULL;
void (__stdcall *guTexMemReset)(void) = NULL;
void (__stdcall *guTexDownloadMipMap)(
                    GrMipMapId_t mmid,
                    const void *src,
                    const GuNccTable *table
                    ) = NULL;
void (__stdcall *guTexDownloadMipMapLevel)(
                         GrMipMapId_t mmid,
                         GrLOD_t lod,
                         const void **src
                         ) = NULL;
void (__stdcall *guTexSource)(GrMipMapId_t id) = NULL;
void (__stdcall *guColorCombineFunction)(GrColorCombineFnc_t fnc) = NULL;

FxBool (__stdcall *grLfbLock)( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode,
           GrOriginLocation_t origin, FxBool pixelPipeline, 
           GrLfbInfo_t *info) = NULL;
FxBool (__stdcall *grLfbUnlock)(GrLock_t type, GrBuffer_t buffer) = NULL;
void (__stdcall *grLfbConstantAlpha)(GrAlpha_t alpha) = NULL;
void (__stdcall *grLfbConstantDepth)(FxU16 depth) = NULL;
void (__stdcall *grLfbWriteColorSwizzle)(FxBool swizzleBytes, FxBool swapWords) = NULL;
void (__stdcall *grLfbWriteColorFormat)(GrColorFormat_t colorFormat) = NULL;
FxBool (__stdcall *grLfbWriteRegion)(GrBuffer_t dst_buffer, 
                  FxU32 dst_x, FxU32 dst_y, 
                  GrLfbSrcFmt_t src_format, 
                  FxU32 src_width, FxU32 src_height, 
                  FxI32 src_stride, void *src_data) = NULL;

FxBool (__stdcall *grLfbReadRegion)(GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data) = NULL;

void (__stdcall *grAADrawLine)(const GrVertex *v1, const GrVertex *v2) = NULL;
void (__stdcall *grAADrawPoint)(const GrVertex *pt) = NULL;
void (__stdcall *grAADrawPolygon)(const int nverts, const int ilist[], const GrVertex vlist[]) = NULL;
void (__stdcall *grAADrawPolygonVertexList)(const int nverts, const GrVertex vlist[]) = NULL;
void (__stdcall *grAADrawTriangle)(
                 const GrVertex *a, const GrVertex *b, const GrVertex *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 ) = NULL;
void (__stdcall *grGlideInit)(void) = NULL;
void (__stdcall *grGlideShutdown)(void) = NULL;
void (__stdcall *grGlideGetVersion)(char version[80]) = NULL;
void (__stdcall *grGlideGetState)(GrState *state) = NULL;
void (__stdcall *grGlideSetState)( const GrState *state) = NULL;
void (__stdcall *grGlideShamelessPlug)(const FxBool on) = NULL;
void (__stdcall *grHints)(GrHint_t hintType, FxU32 hintMask) = NULL;

float (__stdcall *guFogTableIndexToW)(int i) = NULL;
void (__stdcall *guFogGenerateExp)( GrFog_t fogtable[GR_FOG_TABLE_SIZE], float density ) = NULL;
void (__stdcall *guFogGenerateExp2)( GrFog_t fogtable[GR_FOG_TABLE_SIZE], float density ) = NULL;
void (__stdcall *guFogGenerateLinear)( GrFog_t fogtable[GR_FOG_TABLE_SIZE], float nearZ, float farZ ) = NULL;
