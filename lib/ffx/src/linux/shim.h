// Internals of the Linux backend, which runs AMD's unmodified Windows x64 DLL inside this process.
#pragma once

#include <FidelityFX/api/include/vk/ffx_api_vk.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#define FFXP_MSABI __attribute__((ms_abi))

namespace ffxp {

struct DllExports {
    ffxReturnCode_t(FFXP_MSABI* createContext)(ffxContext*, ffxCreateContextDescHeader*, const ffxAllocationCallbacks*);
    ffxReturnCode_t(FFXP_MSABI* destroyContext)(ffxContext*, const ffxAllocationCallbacks*);
    ffxReturnCode_t(FFXP_MSABI* configure)(ffxContext*, const ffxConfigureDescHeader*);
    ffxReturnCode_t(FFXP_MSABI* query)(ffxContext*, ffxQueryDescHeader*);
    ffxReturnCode_t(FFXP_MSABI* dispatch)(ffxContext*, const ffxDispatchDescHeader*);
};

constexpr const char* kDllName = "amd_fidelityfx_upscaler_vk.dll";

// pe_image.cpp. On false, `error` holds the reason and nothing stays mapped.
bool loadDll(DllExports& exports, std::string& error);
bool enterThread();
uint8_t* imageBase();

// win32_shims.cpp
void* resolveImport(const std::string& dll, const std::string& name, std::string& error);
void setClientDeviceProcAddr(PFN_vkGetDeviceProcAddr getDeviceProcAddr);
PFN_vkGetDeviceProcAddr dllDeviceProcAddr();
const char* utf16ToAscii(const char16_t* text, std::string& storage);

// ffx_exports.cpp. Ends the current DLL call with an error if one is active, otherwise aborts.
[[noreturn]] void dllFailure(const std::string& reason);

// sha256.cpp
std::string sha256Hex(const uint8_t* data, size_t size);

}  // namespace ffxp
