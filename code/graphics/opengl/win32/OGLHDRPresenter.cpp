#include "graphics/opengl/win32/OGLHDRPresenter.h"

#ifdef _WIN32

#include "graphics/2d.h"
#include "graphics/opengl/gropengldraw.h"
#include "graphics/opengl/gropenglstate.h"
#include <d3d11.h>
#include <dxgi1_6.h>
#include <glad/glad.h>
#include <cstring>
#include <algorithm>
#include <cmath>

// WGL_NV_DX_interop2 constants (not in GLAD)
#ifndef WGL_ACCESS_READ_ONLY_NV
#define WGL_ACCESS_READ_ONLY_NV 0x00000000
#endif
#ifndef WGL_ACCESS_READ_WRITE_NV
#define WGL_ACCESS_READ_WRITE_NV 0x00000001
#endif
#ifndef WGL_ACCESS_WRITE_DISCARD_NV
#define WGL_ACCESS_WRITE_DISCARD_NV 0x00000002
#endif

OGLHDRPresenter::OGLHDRPresenter()
	: _initialized(false)
	, _hwnd(nullptr)
	, _width(0)
	, _height(0)
	, _factory(nullptr)
	, _device(nullptr)
	, _context(nullptr)
	, _swapChain(nullptr)
	, _backBuffer(nullptr)
	, _stagingTexture(nullptr)
	, _readbackFBO(0)
	, _pbo{0, 0}
	, _currentPBO(0)
	, _interopAvailable(false)
	, _dxDevice(nullptr)
	, _dxObject(nullptr)
	, _interopTexture(0)
	, _wglDXOpenDeviceNV(nullptr)
	, _wglDXCloseDeviceNV(nullptr)
	, _wglDXRegisterObjectNV(nullptr)
	, _wglDXUnregisterObjectNV(nullptr)
	, _wglDXLockObjectsNV(nullptr)
	, _wglDXUnlockObjectsNV(nullptr)
{
}

OGLHDRPresenter::~OGLHDRPresenter()
{
	shutdown();
}

bool OGLHDRPresenter::initialize(HWND hwnd, int width, int height)
{
	if (_initialized) {
		shutdown();
	}

	_hwnd = hwnd;
	_width = width;
	_height = height;
	_initialized = false;
	_interopAvailable = false;
	_dxDevice = nullptr;
	_dxObject = nullptr;
	_interopTexture = 0;
	_readbackFBO = 0;
	_pbo[0] = 0;
	_pbo[1] = 0;
	_currentPBO = 0;

	if (!createD3D11Device()) {
		mprintf(("OGLHDRPresenter: Failed to create D3D11 device\n"));
		return false;
	}

	if (!createSwapChain()) {
		mprintf(("OGLHDRPresenter: Failed to create swap chain\n"));
		return false;
	}

	if (!setupHDRMetadata()) {
		mprintf(("OGLHDRPresenter: Failed to setup HDR metadata\n"));
		return false;
	}

	if (!createPBOs()) {
		mprintf(("OGLHDRPresenter: Failed to create PBOs\n"));
		return false;
	}

	// Try to setup WGL interop (optional, falls back to PBO if unavailable)
	setupWGLInterop();

	_initialized = true;
	mprintf(("OGLHDRPresenter: Initialized successfully (interop: %s)\n", _interopAvailable ? "yes" : "no"));
	return true;
}

bool OGLHDRPresenter::createD3D11Device()
{
	HRESULT hr;

	// Create DXGI factory
	hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory6), reinterpret_cast<void**>(&_factory));
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: CreateDXGIFactory2 failed: 0x%08X\n", hr));
		return false;
	}

	// Get adapter
	IDXGIAdapter1* adapter = nullptr;
	hr = _factory->EnumAdapters1(0, &adapter);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: EnumAdapters1 failed: 0x%08X\n", hr));
		return false;
	}

	// Create D3D11 device
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL featureLevel;
	hr = D3D11CreateDevice(
		adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		0,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&_device,
		&featureLevel,
		&_context
	);

	adapter->Release();

	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: D3D11CreateDevice failed: 0x%08X\n", hr));
		return false;
	}

	return true;
}

bool OGLHDRPresenter::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = _width;
	swapChainDesc.Height = _height;
	swapChainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	HRESULT hr = _factory->CreateSwapChainForHwnd(
		_device,
		_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&_swapChain
	);

	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: CreateSwapChainForHwnd failed: 0x%08X\n", hr));
		return false;
	}

	// Get back buffer
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&_backBuffer));
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: GetBuffer failed: 0x%08X\n", hr));
		return false;
	}

	// Create staging texture for CPU upload
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	_backBuffer->GetDesc(&stagingDesc);
	stagingDesc.Usage = D3D11_USAGE_DYNAMIC;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingDesc.BindFlags = 0;

	hr = _device->CreateTexture2D(&stagingDesc, nullptr, &_stagingTexture);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: CreateTexture2D (staging) failed: 0x%08X\n", hr));
		return false;
	}

	return true;
}

bool OGLHDRPresenter::setupHDRMetadata()
{
	IDXGISwapChain4* swapChain4 = nullptr;
	HRESULT hr = _swapChain->QueryInterface(__uuidof(IDXGISwapChain4), reinterpret_cast<void**>(&swapChain4));
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: QueryInterface(IDXGISwapChain4) failed: 0x%08X\n", hr));
		return false;
	}

	// Set color space
	hr = swapChain4->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: SetColorSpace1 failed: 0x%08X\n", hr));
		swapChain4->Release();
		return false;
	}

	// Set HDR metadata
	DXGI_HDR_METADATA_HDR10 metadata = {};
	metadata.RedPrimary[0] = static_cast<UINT16>(34000);   // Rec.2020 red
	metadata.RedPrimary[1] = static_cast<UINT16>(16000);
	metadata.GreenPrimary[0] = static_cast<UINT16>(13250);
	metadata.GreenPrimary[1] = static_cast<UINT16>(34500);
	metadata.BluePrimary[0] = static_cast<UINT16>(7500);
	metadata.BluePrimary[1] = static_cast<UINT16>(3000);
	metadata.WhitePoint[0] = static_cast<UINT16>(15635);
	metadata.WhitePoint[1] = static_cast<UINT16>(16450);
	metadata.MaxMasteringLuminance = static_cast<UINT>(Gr_hdr_max_nits * 10000.0f);  // Convert to 0.0001 nits units
	metadata.MinMasteringLuminance = 50;  // 0.005 nits
	metadata.MaxContentLightLevel = static_cast<UINT>(Gr_hdr_max_nits * 10000.0f);
	metadata.MaxFrameAverageLightLevel = static_cast<UINT>(Gr_hdr_paper_white_nits * 10000.0f);

	hr = swapChain4->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(metadata), &metadata);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: SetHDRMetaData failed: 0x%08X\n", hr));
		swapChain4->Release();
		return false;
	}

	swapChain4->Release();
	return true;
}

bool OGLHDRPresenter::createPBOs()
{
	glGenBuffers(2, _pbo);
	if (_pbo[0] == 0 || _pbo[1] == 0) {
		mprintf(("OGLHDRPresenter: glGenBuffers failed\n"));
		return false;
	}

	// Allocate PBOs for RGB10_A2 format (4 bytes per pixel)
	const size_t bufferSize = _width * _height * sizeof(uint32_t);
	for (int i = 0; i < 2; i++) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[i]);
		glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, nullptr, GL_STREAM_READ);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Create scratch FBO for readback
	glGenFramebuffers(1, &_readbackFBO);

	_currentPBO = 0;
	return true;
}

void OGLHDRPresenter::destroyPBOs()
{
	if (_pbo[0] != 0) {
		glDeleteBuffers(2, _pbo);
		_pbo[0] = 0;
		_pbo[1] = 0;
	}
	if (_readbackFBO != 0) {
		glDeleteFramebuffers(1, &_readbackFBO);
		_readbackFBO = 0;
	}
}

bool OGLHDRPresenter::setupWGLInterop()
{
	// Load WGL interop function pointers
	_wglDXOpenDeviceNV = reinterpret_cast<PFNWGLDXOPENDEVICENVPROC>(wglGetProcAddress("wglDXOpenDeviceNV"));
	_wglDXCloseDeviceNV = reinterpret_cast<PFNWGLDXCLOSEDEVICENVPROC>(wglGetProcAddress("wglDXCloseDeviceNV"));
	_wglDXRegisterObjectNV = reinterpret_cast<PFNWGLDXREGISTEROBJECTNVPROC>(wglGetProcAddress("wglDXRegisterObjectNV"));
	_wglDXUnregisterObjectNV = reinterpret_cast<PFNWGLDXUNREGISTEROBJECTNVPROC>(wglGetProcAddress("wglDXUnregisterObjectNV"));
	_wglDXLockObjectsNV = reinterpret_cast<PFNWGLDXLOCKOBJECTSNVPROC>(wglGetProcAddress("wglDXLockObjectsNV"));
	_wglDXUnlockObjectsNV = reinterpret_cast<PFNWGLDXUNLOCKOBJECTSNVPROC>(wglGetProcAddress("wglDXUnlockObjectsNV"));

	// Check if all functions are available
	if (!_wglDXOpenDeviceNV || !_wglDXCloseDeviceNV || !_wglDXRegisterObjectNV || 
	    !_wglDXUnregisterObjectNV || !_wglDXLockObjectsNV || !_wglDXUnlockObjectsNV) {
		mprintf(("OGLHDRPresenter: WGL_NV_DX_interop2 not available\n"));
		return false;
	}

	// Open D3D11 device for interop
	_dxDevice = _wglDXOpenDeviceNV(_device);
	if (_dxDevice == nullptr) {
		mprintf(("OGLHDRPresenter: wglDXOpenDeviceNV failed\n"));
		return false;
	}

	// Create GL texture for interop - must match DXGI backbuffer format (R10G10B10A2_UNORM)
	glGenTextures(1, &_interopTexture);
	glBindTexture(GL_TEXTURE_2D, _interopTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, _width, _height, 0, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Register backbuffer texture
	_dxObject = _wglDXRegisterObjectNV(_dxDevice, _backBuffer, _interopTexture, GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);
	if (_dxObject == nullptr) {
		mprintf(("OGLHDRPresenter: wglDXRegisterObjectNV failed\n"));
		glDeleteTextures(1, &_interopTexture);
		_interopTexture = 0;
		_wglDXCloseDeviceNV(_dxDevice);
		_dxDevice = nullptr;
		return false;
	}

	_interopAvailable = true;
	return true;
}

void OGLHDRPresenter::destroyWGLInterop()
{
	if (_dxObject != nullptr && _wglDXUnregisterObjectNV) {
		_wglDXUnregisterObjectNV(_dxDevice, _dxObject);
		_dxObject = nullptr;
	}
	if (_interopTexture != 0) {
		glDeleteTextures(1, &_interopTexture);
		_interopTexture = 0;
	}
	if (_dxDevice != nullptr && _wglDXCloseDeviceNV) {
		_wglDXCloseDeviceNV(_dxDevice);
		_dxDevice = nullptr;
	}
	_interopAvailable = false;
	_wglDXOpenDeviceNV = nullptr;
	_wglDXCloseDeviceNV = nullptr;
	_wglDXRegisterObjectNV = nullptr;
	_wglDXUnregisterObjectNV = nullptr;
	_wglDXLockObjectsNV = nullptr;
	_wglDXUnlockObjectsNV = nullptr;
}

bool OGLHDRPresenter::presentFromGL(GLuint sourceTexture, int width, int height, bool vsync)
{
	if (!_initialized) {
		return false;
	}

	// Resize if needed
	if (width != _width || height != _height) {
		if (!resize(width, height)) {
			return false;
		}
	}

	bool success = false;
	if (_interopAvailable) {
		success = readbackViaInterop(sourceTexture, width, height);
	} else {
		success = readbackViaPBO(sourceTexture, width, height);
	}

	if (!success) {
		return false;
	}

	// Present
	UINT syncInterval = vsync ? 1 : 0;
	HRESULT hr = _swapChain->Present(syncInterval, 0);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: Present failed: 0x%08X\n", hr));
		return false;
	}

	return true;
}

bool OGLHDRPresenter::readbackViaPBO(GLuint sourceTexture, int width, int height)
{
	// Save current GL state
	GLint oldReadFBO, oldDrawFBO, oldReadBuffer;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFBO);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFBO);
	glGetIntegerv(GL_READ_BUFFER, &oldReadBuffer);

	// Bind scratch FBO with source texture
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _readbackFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sourceTexture, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// Map previous PBO (from last frame) and upload to D3D11
	int prevPBO = 1 - _currentPBO;
	glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[prevPBO]);
	void* mapped = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	if (mapped != nullptr) {
		// Source is already RGB10_A2 (same as DXGI R10G10B10A2), direct copy
		const int pixelCount = width * height;
		const size_t dataSize = pixelCount * sizeof(uint32_t);

		// Upload directly to D3D11 staging texture (no conversion needed)
		D3D11_MAPPED_SUBRESOURCE mappedRes;
		HRESULT hr = _context->Map(_stagingTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
		if (SUCCEEDED(hr)) {
			memcpy(mappedRes.pData, mapped, dataSize);
			_context->Unmap(_stagingTexture, 0);

			// Copy staging to backbuffer
			_context->CopyResource(_backBuffer, _stagingTexture);
		}
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	// Issue async readback for next frame (RGB10_A2 format)
	glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[_currentPBO]);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, nullptr);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Swap PBOs
	_currentPBO = 1 - _currentPBO;

	// Restore GL state
	glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
	glReadBuffer(oldReadBuffer);

	return true;
}

bool OGLHDRPresenter::readbackViaInterop(GLuint sourceTexture, int width, int height)
{
	if (!_wglDXLockObjectsNV || !_wglDXUnlockObjectsNV) {
		return false;
	}

	// Load glCopyImageSubData function pointer (OpenGL 4.3+)
	typedef void (APIENTRY *PFNGLCOPYIMAGESUBDATAPROC)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
	PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubDataPtr = reinterpret_cast<PFNGLCOPYIMAGESUBDATAPROC>(wglGetProcAddress("glCopyImageSubData"));
	if (!glCopyImageSubDataPtr) {
		// Try ARB version
		glCopyImageSubDataPtr = reinterpret_cast<PFNGLCOPYIMAGESUBDATAPROC>(wglGetProcAddress("glCopyImageSubDataARB"));
	}
	if (!glCopyImageSubDataPtr) {
		mprintf(("OGLHDRPresenter: glCopyImageSubData not available\n"));
		return false;
	}

	// Lock interop object
	if (!_wglDXLockObjectsNV(_dxDevice, 1, &_dxObject)) {
		mprintf(("OGLHDRPresenter: wglDXLockObjectsNV failed\n"));
		return false;
	}

	// Copy from source texture to interop texture
	glCopyImageSubDataPtr(
		sourceTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
		_interopTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
		width, height, 1
	);

	// Unlock
	if (!_wglDXUnlockObjectsNV(_dxDevice, 1, &_dxObject)) {
		mprintf(("OGLHDRPresenter: wglDXUnlockObjectsNV failed\n"));
		return false;
	}

	return true;
}

bool OGLHDRPresenter::resize(int width, int height)
{
	if (!_initialized) {
		return false;
	}

	if (width == _width && height == _height) {
		return true;  // No resize needed
	}

	// Release old resources
	if (_backBuffer != nullptr) {
		_backBuffer->Release();
		_backBuffer = nullptr;
	}
	if (_stagingTexture != nullptr) {
		_stagingTexture->Release();
		_stagingTexture = nullptr;
	}

	// Destroy and recreate interop if active
	if (_interopAvailable) {
		destroyWGLInterop();
	}

	_width = width;
	_height = height;

	// Resize swap chain
	HRESULT hr = _swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R10G10B10A2_UNORM, 0);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: ResizeBuffers failed: 0x%08X\n", hr));
		return false;
	}

	// Recreate backbuffer and staging
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&_backBuffer));
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: GetBuffer after resize failed: 0x%08X\n", hr));
		return false;
	}

	D3D11_TEXTURE2D_DESC stagingDesc = {};
	_backBuffer->GetDesc(&stagingDesc);
	stagingDesc.Usage = D3D11_USAGE_DYNAMIC;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingDesc.BindFlags = 0;

	hr = _device->CreateTexture2D(&stagingDesc, nullptr, &_stagingTexture);
	if (FAILED(hr)) {
		mprintf(("OGLHDRPresenter: CreateTexture2D (staging) after resize failed: 0x%08X\n", hr));
		return false;
	}

	// Recreate PBOs with new size
	destroyPBOs();
	if (!createPBOs()) {
		return false;
	}

	// Recreate interop if it was active
	if (_interopAvailable) {
		setupWGLInterop();
	}

	return true;
}

void OGLHDRPresenter::shutdown()
{
	if (!_initialized) {
		return;
	}

	destroyWGLInterop();
	destroyPBOs();

	if (_stagingTexture != nullptr) {
		_stagingTexture->Release();
		_stagingTexture = nullptr;
	}
	if (_backBuffer != nullptr) {
		_backBuffer->Release();
		_backBuffer = nullptr;
	}
	if (_swapChain != nullptr) {
		_swapChain->Release();
		_swapChain = nullptr;
	}
	if (_context != nullptr) {
		_context->Release();
		_context = nullptr;
	}
	if (_device != nullptr) {
		_device->Release();
		_device = nullptr;
	}
	if (_factory != nullptr) {
		_factory->Release();
		_factory = nullptr;
	}

	_initialized = false;
}

#endif // _WIN32

