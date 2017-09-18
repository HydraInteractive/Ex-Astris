#include <hydra/ext/vram.hpp>
#include <memory>
#ifdef __linux__
size_t Hydra::Ext::getCurrentVRAM() {
	return 0;
}

size_t Hydra::Ext::getMaxVRAM() {
	return 0;
}

#elif defined(_WIN32)
#pragma comment(lib, "dxgi.lib")
#include <d3d11_3.h>
#include <dxgi1_4.h>

static IDXGIAdapter3* getDXGIAdapter3() {
	static IDXGIAdapter3* adapter;
	if (adapter)
		return adapter;

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

	return adapter;
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
#else
// #warning "VRAM missing!"
size_t Hydra::Ext::getCurrentVRAM() { return 0; }
size_t Hydra::Ext::getMaxVRAM() { return 0; }
#endif
