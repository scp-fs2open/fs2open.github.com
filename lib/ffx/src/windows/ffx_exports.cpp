// AMD's recommended runtime loading. AMD's DLLs need the Visual C++ runtime, which an application linking its C
// runtime statically doesn't otherwise require, so linking them at startup could stop that application from starting.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WINDOWS
#define _WINDOWS  // ffx_api_loader.h keys off MSBuild's define, which other toolchains don't set.
#endif
#include <FidelityFX/api/include/ffx_api_loader.h>

#include <string>

extern "C" {
bool ffxPortableCanLoad();
const char* ffxPortableLoadError();
}

namespace {

constexpr ffxReturnCode_t kNoProvider = FFX_API_RETURN_NO_PROVIDER;

ffxFunctions g_functions;
std::string g_error;

std::wstring executableDirectory()
{
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, &path[0], static_cast<DWORD>(path.size()));
    path.resize(length);
    const auto slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? std::wstring() : path.substr(0, slash + 1);
}

// By full path, so a same-named DLL elsewhere on the search path (or in the working directory) can't be picked up.
HMODULE loadBesideExecutable(const wchar_t* name, const char* displayName)
{
    const std::wstring path = executableDirectory() + name;
    HMODULE module = LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (module)
        return module;

    const DWORD code = GetLastError();
    g_error = std::string("could not load ") + displayName + " (Windows error " + std::to_string(code) + ")";
    if (code == ERROR_MOD_NOT_FOUND)
        g_error += ": it, or something it needs such as the Visual C++ Redistributable or the Vulkan runtime, is missing";
    return nullptr;
}

bool load()
{
    // AMD's loader finds this itself, but only when an effect is first used. Loading it here means a missing
    // dependency is reported now. The loader reuses a module that's already loaded, and neither is ever unloaded.
    if (!loadBesideExecutable(L"amd_fidelityfx_upscaler_vk.dll", "amd_fidelityfx_upscaler_vk.dll"))
        return false;
    HMODULE loader = loadBesideExecutable(L"amd_fidelityfx_loader_vk.dll", "amd_fidelityfx_loader_vk.dll");
    if (!loader)
        return false;

    ffxLoadFunctions(&g_functions, loader);
    if (!g_functions.CreateContext || !g_functions.DestroyContext || !g_functions.Configure || !g_functions.Query || !g_functions.Dispatch) {
        g_error = "amd_fidelityfx_loader_vk.dll does not export the FFX API";
        return false;
    }
    return true;
}

bool ready()
{
    static const bool loaded = load();
    return loaded;
}

}  // namespace

extern "C" {

bool ffxPortableCanLoad() { return ready(); }
const char* ffxPortableLoadError()
{
    ready();
    return g_error.c_str();
}

ffxReturnCode_t ffxCreateContext(ffxContext* context, ffxCreateContextDescHeader* desc, const ffxAllocationCallbacks* memCb)
{
    return ready() ? g_functions.CreateContext(context, desc, memCb) : kNoProvider;
}

ffxReturnCode_t ffxDestroyContext(ffxContext* context, const ffxAllocationCallbacks* memCb)
{
    return ready() ? g_functions.DestroyContext(context, memCb) : kNoProvider;
}

ffxReturnCode_t ffxConfigure(ffxContext* context, const ffxConfigureDescHeader* desc)
{
    return ready() ? g_functions.Configure(context, desc) : kNoProvider;
}

ffxReturnCode_t ffxQuery(ffxContext* context, ffxQueryDescHeader* desc)
{
    return ready() ? g_functions.Query(context, desc) : kNoProvider;
}

ffxReturnCode_t ffxDispatch(ffxContext* context, const ffxDispatchDescHeader* desc)
{
    return ready() ? g_functions.Dispatch(context, desc) : kNoProvider;
}

}  // extern "C"
