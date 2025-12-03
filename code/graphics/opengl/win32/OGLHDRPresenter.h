#pragma once

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <glad/glad.h>

// Forward declarations to avoid including D3D/DXGI headers (conflicts with ddsutils.h)
struct IDXGIFactory6;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain1;
struct ID3D11Texture2D;

class OGLHDRPresenter {
public:
	OGLHDRPresenter();
	~OGLHDRPresenter();

	bool initialize(HWND hwnd, int width, int height);
	bool resize(int width, int height);
	bool presentFromGL(GLuint sourceTexture, int width, int height, bool vsync);
	void shutdown();
	bool isInitialized() const { return _initialized; }

private:
	bool _initialized;
	HWND _hwnd;
	int _width;
	int _height;

	IDXGIFactory6* _factory;
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	IDXGISwapChain1* _swapChain;
	ID3D11Texture2D* _backBuffer;
	ID3D11Texture2D* _stagingTexture;

	GLuint _readbackFBO;
	GLuint _pbo[2];
	int _currentPBO;

	bool _interopAvailable;
	HANDLE _dxDevice;
	HANDLE _dxObject;
	GLuint _interopTexture;

	// WGL_NV_DX_interop2 function pointers (loaded at runtime)
	typedef HANDLE (APIENTRY *PFNWGLDXOPENDEVICENVPROC)(void* dxDevice);
	typedef BOOL (APIENTRY *PFNWGLDXCLOSEDEVICENVPROC)(HANDLE hDevice);
	typedef HANDLE (APIENTRY *PFNWGLDXREGISTEROBJECTNVPROC)(HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access);
	typedef BOOL (APIENTRY *PFNWGLDXUNREGISTEROBJECTNVPROC)(HANDLE hDevice, HANDLE hObject);
	typedef BOOL (APIENTRY *PFNWGLDXLOCKOBJECTSNVPROC)(HANDLE hDevice, GLint count, HANDLE* hObjects);
	typedef BOOL (APIENTRY *PFNWGLDXUNLOCKOBJECTSNVPROC)(HANDLE hDevice, GLint count, HANDLE* hObjects);
	PFNWGLDXOPENDEVICENVPROC _wglDXOpenDeviceNV;
	PFNWGLDXCLOSEDEVICENVPROC _wglDXCloseDeviceNV;
	PFNWGLDXREGISTEROBJECTNVPROC _wglDXRegisterObjectNV;
	PFNWGLDXUNREGISTEROBJECTNVPROC _wglDXUnregisterObjectNV;
	PFNWGLDXLOCKOBJECTSNVPROC _wglDXLockObjectsNV;
	PFNWGLDXUNLOCKOBJECTSNVPROC _wglDXUnlockObjectsNV;

	bool createD3D11Device();
	bool createSwapChain();
	bool setupHDRMetadata();
	bool createPBOs();
	void destroyPBOs();
	bool setupWGLInterop();
	void destroyWGLInterop();
	bool readbackViaPBO(GLuint sourceTexture, int width, int height);
	bool readbackViaInterop(GLuint sourceTexture, int width, int height);
};

extern OGLHDRPresenter* g_hdr_presenter;

#endif // _WIN32
