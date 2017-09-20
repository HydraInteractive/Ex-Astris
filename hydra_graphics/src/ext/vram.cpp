#include <hydra/ext/vram.hpp>

#include <glad/glad.h>
#include <string>
#include <cstdio>

// Bool glXQueryRendererIntegerMESA(Display *dpy, int screen, int renderer, int attribute, unsigned int *value);
// Bool glXQueryCurrentRendererIntegerMESA(int attribute, unsigned int *value);
// const char *glXQueryRendererStringMESA(Display *dpy, int screen, int renderer, int attribute);
// const char *glXQueryCurrentRendererStringMESA(int attribute);
// Bool glXQueryRendererIntegerMESA(Display *dpy, int screen, int renderer, int attribute, unsigned int *value);


// glXQueryCurrentRendererIntegerMESA
#define GLX_RENDERER_VENDOR_ID_MESA 0x8183
#define GLX_RENDERER_DEVICE_ID_MESA 0x8184
#define GLX_RENDERER_VERSION_MESA 0x8185
#define GLX_RENDERER_ACCELERATED_MESA 0x8186
#define GLX_RENDERER_VIDEO_MEMORY_MESA 0x8187
#define GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA 0x8188
#define GLX_RENDERER_PREFERRED_PROFILE_MESA 0x8189
#define GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA 0x818A
#define GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA 0x818B
#define GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA 0x818C
#define GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA 0x818D

#ifdef __linux__
size_t Hydra::Ext::getCurrentVRAM() {
	return 0;
}

size_t Hydra::Ext::getMaxVRAM() {
	return 0;
}

bool Hydra::Ext::isVRAMDedicated() {
	return true;
}
#elif defined(_WIN32)
#pragma comment(lib, "dxgi.lib")
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

static IDXGIAdapter3* getDXGIAdapter3() {
	static Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
	if (adapter)
		return adapter.Get();

	IDXGIFactory* dxgifactory = nullptr;
	IDXGIAdapter* firstAdapter = nullptr;
	IDXGIAdapter3* dxgiAdapter3 = nullptr;

	HRESULT status = ::CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgifactory));
	if (!SUCCEEDED(status))
		return nullptr;
	status = dxgifactory->EnumAdapters(0, &firstAdapter);

	if (!SUCCEEDED(firstAdapter->QueryInterface(__uuidof(IDXGIAdapter3), (void**)&dxgiAdapter3)))
		return nullptr;
	adapter = dxgiAdapter3;

	return adapter.Get();
}

size_t Hydra::Ext::getCurrentVRAM() {
	auto adapter = getDXGIAdapter3();
	if (!adapter)
		return 0;
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	if (!SUCCEEDED(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info)))
		return 0;

	return info.CurrentUsage;
}

size_t Hydra::Ext::getMaxVRAM() {
	auto adapter = getDXGIAdapter3();
	if (!adapter)
		return 0;
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	if (!SUCCEEDED(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info)))
		return 0;

	return info.Budget;
}

bool Hydra::Ext::isVRAMDedicated() {
	static bool init = false;
	static bool result;
	if (init)
		return result;

	const char * vendor = (const char*)glGetString(GL_VENDOR);
	result = strncmp(vendor, "Intel", 5);
	init = true;
	return result;
}
#else
// #warning "VRAM missing!"
size_t Hydra::Ext::getCurrentVRAM() { return 0; }
size_t Hydra::Ext::getMaxVRAM() { return 0; }
bool Hydra::Ext::isVRAMDedicated() { return true; }
#endif
