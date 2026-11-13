// What the DLL imports, mapped to Linux. Everything is ms_abi because that's how the DLL calls it.
// The import list is fixed by the DLL's pinned hash; an unknown name fails the load instead of crashing.
#include "shim.h"

#include <dlfcn.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
#include <utility>

namespace ffxp {

namespace {

// ---- Vulkan. No function the DLL names takes float arguments, so one 12-integer forwarder covers them.
// Reading unused stack slots is harmless, and extra sysv arguments are ignored.
using SysvFunction = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
constexpr size_t kVulkanThunks = 96;
SysvFunction g_vulkanTargets[kVulkanThunks];

template <size_t Slot>
FFXP_MSABI uint64_t vulkanThunk(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f, uint64_t g, uint64_t h, uint64_t i, uint64_t j, uint64_t k,
                                uint64_t l)
{
    return g_vulkanTargets[Slot](a, b, c, d, e, f, g, h, i, j, k, l);
}

void* g_vulkanThunkTable[kVulkanThunks];
template <size_t... Slots>
void fillVulkanThunks(std::index_sequence<Slots...>)
{
    ((g_vulkanThunkTable[Slots] = reinterpret_cast<void*>(&vulkanThunk<Slots>)), ...);
}

std::map<std::string, size_t> g_vulkanSlots;

void* vulkanThunkFor(const char* name, void* target)
{
    if (!target)
        return nullptr;
    if (g_vulkanSlots.empty())
        fillVulkanThunks(std::make_index_sequence<kVulkanThunks>{});
    auto it = g_vulkanSlots.find(name);
    size_t slot;
    if (it != g_vulkanSlots.end()) {
        slot = it->second;
    } else {
        if (g_vulkanSlots.size() == kVulkanThunks)
            return nullptr;
        slot = g_vulkanSlots.size();
        g_vulkanSlots[name] = slot;
    }
    g_vulkanTargets[slot] = reinterpret_cast<SysvFunction>(target);
    return g_vulkanThunkTable[slot];
}

void* g_libvulkan;
PFN_vkGetDeviceProcAddr g_clientDeviceProcAddr;

FFXP_MSABI void* msGetDeviceProcAddr(VkDevice device, const char* name)
{
    PFN_vkGetDeviceProcAddr getter = g_clientDeviceProcAddr;
    if (!getter)
        getter = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(g_libvulkan, "vkGetDeviceProcAddr"));
    return vulkanThunkFor(name, reinterpret_cast<void*>(getter(device, name)));
}

// ---- Stubs shared by many imports. ms_abi callers clean up after themselves, so one function fits any signature.
FFXP_MSABI uint64_t returnZero() { return 0; }
FFXP_MSABI uint64_t returnOne() { return 1; }
FFXP_MSABI void* returnSelf(void* self) { return self; }

constexpr const char* kFatalImports[] = {
    "_invoke_watson", "_purecall", "__std_terminate", "_CxxThrowException", "__C_specific_handler", "__CxxFrameHandler4", "__std_exception_copy",
    "TerminateProcess", "DebugBreak", "SleepConditionVariableSRW", "_Mbrtowc", "?_Xlength_error@std@@YAXPEBD@Z", "?_Xbad_alloc@std@@YAXXZ",
    "?_Throw_Cpp_error@std@@YAXH@Z", "?_Decref@facet@locale@std@@UEAAPEAV_Facet_base@3@XZ", "?_Incref@facet@locale@std@@UEAAXXZ",
    "?_Gettrue@_Locinfo@std@@QEBAPEBDXZ", "?_Getfalse@_Locinfo@std@@QEBAPEBDXZ", "?_Getlconv@_Locinfo@std@@QEBAPEBUlconv@@XZ", "??1_Locinfo@std@@QEAA@XZ",
    "??0_Locinfo@std@@QEAA@PEBD@Z", "??1facet@locale@std@@MEAA@XZ", "??0facet@locale@std@@IEAA@_K@Z", "?_Getcvt@_Locinfo@std@@QEBA?AU_Cvtvec@@XZ",
    "?_Getgloballocale@locale@std@@CAPEAV_Locimp@12@XZ", "?_Init@locale@std@@CAPEAV_Locimp@12@_N@Z"};

template <size_t Index>
[[noreturn]] FFXP_MSABI void fatalImport()
{
    dllFailure(std::string("the DLL reached an unsupported runtime path: ") + kFatalImports[Index]);
}

template <size_t... Indices>
void* fatalStub(size_t index, std::index_sequence<Indices...>)
{
    static void* const stubs[] = {reinterpret_cast<void*>(&fatalImport<Indices>)...};
    return stubs[index];
}

// The upscaler creates no threads and every entry into it holds a global lock (ffx_exports.cpp), so its
// locks can't be contended. Startup and shutdown hooks need no work because the image is never unloaded.
constexpr const char* kReturnZeroImports[] = {
    "_seh_filter_dll", "_configure_narrow_argv", "_initialize_narrow_environment", "_initialize_onexit_table", "_register_onexit_function",
    "_execute_onexit_table", "_crt_atexit", "_cexit", "__std_type_info_destroy_list", "_callnewh", "__std_exception_destroy",
    "IsProcessorFeaturePresent", "SetUnhandledExceptionFilter", "UnhandledExceptionFilter", "RtlVirtualUnwind", "RtlLookupFunctionEntry",
    "RtlCaptureContext", "IsDebuggerPresent", "GetFileVersionInfoA", "GetFileVersionInfoSizeA", "VerQueryValueA", "LoadLibraryA", "LoadLibraryExA",
    "GetProcAddress", "GetModuleFileNameA", "WakeAllConditionVariable", "AcquireSRWLockExclusive", "ReleaseSRWLockExclusive", "_Mtx_init_in_situ",
    "_Mtx_lock", "_Mtx_unlock", "??1_Lockit@std@@QEAA@XZ"};
constexpr const char* kReturnOneImports[] = {"DisableThreadLibraryCalls", "FreeLibrary"};

// MSVC locale ids, only touched by the fatal locale paths above.
alignas(16) uint8_t g_dataImports[3][32];
const std::pair<const char*, void*> kDataImports[] = {{"?_Id_cnt@id@locale@std@@0HA", g_dataImports[0]},
                                                      {"?id@?$numpunct@D@std@@2V0locale@2@A", g_dataImports[1]},
                                                      {"?id@?$numpunct@_W@std@@2V0locale@2@A", g_dataImports[2]}};

// ---- libc functions with the same signature on both sides: generate the ms_abi wrapper.
template <auto Function>
struct Ms;
template <typename R, typename... A, R (*Function)(A...)>
struct Ms<Function> {
    static FFXP_MSABI R call(A... args) { return Function(args...); }
};

template <typename Signature>
constexpr Signature* pick(Signature* function) { return function; }

// ---- Imports that need real work.
std::string formatMs(const std::string& format, const char* args, bool wideByDefault)
{
    std::string out;
    auto next = [&]() {
        uint64_t value;
        std::memcpy(&value, args, 8);
        args += 8;
        return value;
    };
    for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] != '%') {
            out += format[i];
            continue;
        }
        if (format[i + 1] == '%') {
            out += '%';
            ++i;
            continue;
        }
        std::string spec = "%";
        ++i;
        while (i < format.size() && std::strchr("-+ #0", format[i]))
            spec += format[i++];
        while (i < format.size() && (std::isdigit(static_cast<unsigned char>(format[i])) || format[i] == '.' || format[i] == '*')) {
            if (format[i] == '*')
                spec += std::to_string(static_cast<int>(next()));
            else
                spec += format[i];
            ++i;
        }
        std::string length;
        while (i < format.size() && std::strchr("hlLzjtwI", format[i])) {
            length += format[i++];
            while (length.back() == 'I' && i < format.size() && std::isdigit(static_cast<unsigned char>(format[i])))
                length += format[i++], length += 'I';
        }
        const bool wideInteger = length.find("ll") != std::string::npos || length.find('6') != std::string::npos || length == "z" || length == "j";
        const char conversion = format[i];
        char buffer[512];
        switch (conversion) {
        case 'd': case 'i': {
            const uint64_t value = next();
            std::snprintf(buffer, sizeof(buffer), (spec + "lld").c_str(), wideInteger ? static_cast<long long>(value) : static_cast<long long>(static_cast<int>(value)));
            out += buffer;
            break;
        }
        case 'u': case 'x': case 'X': case 'o': {
            const uint64_t value = next();
            std::snprintf(buffer, sizeof(buffer), (spec + "ll" + conversion).c_str(), static_cast<unsigned long long>(wideInteger ? value : static_cast<uint32_t>(value)));
            out += buffer;
            break;
        }
        case 'p':
            std::snprintf(buffer, sizeof(buffer), "%p", reinterpret_cast<void*>(next()));
            out += buffer;
            break;
        case 'c':
            out += static_cast<char>(next());
            break;
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': {
            const uint64_t bits = next();
            double value;
            std::memcpy(&value, &bits, 8);
            std::snprintf(buffer, sizeof(buffer), (spec + conversion).c_str(), value);
            out += buffer;
            break;
        }
        case 's': case 'S': {
            const bool wide = length == "l" || length == "w" || (length != "h" && ((conversion == 's') == wideByDefault));
            void* pointer = reinterpret_cast<void*>(next());
            std::string storage;
            out += !pointer ? "(null)" : wide ? utf16ToAscii(static_cast<char16_t*>(pointer), storage) : static_cast<const char*>(pointer);
            break;
        }
        default:
            out += spec + conversion;
            break;
        }
    }
    return out;
}

FFXP_MSABI int msVsprintf(uint64_t, char* buffer, size_t size, const char* format, void*, char* args)
{
    const std::string text = formatMs(format, args, false);
    if (buffer && size) {
        const size_t count = std::min(size - 1, text.size());
        std::memcpy(buffer, text.data(), count);
        buffer[count] = 0;
    }
    return static_cast<int>(text.size());
}

FFXP_MSABI int msVswprintf(uint64_t, char16_t* buffer, size_t size, const char16_t* format, void*, char* args)
{
    std::string storage;
    const std::string text = formatMs(utf16ToAscii(format, storage), args, true);
    if (buffer && size) {
        const size_t count = std::min(size - 1, text.size());
        for (size_t i = 0; i < count; ++i)
            buffer[i] = static_cast<unsigned char>(text[i]);
        buffer[count] = 0;
    }
    return static_cast<int>(text.size());
}

FFXP_MSABI int msStrncpyS(char* destination, size_t size, const char* source, size_t count)
{
    if (!destination || !size)
        return 22;
    const size_t length = std::min(strnlen(source, count == size_t(-1) ? size - 1 : count), size - 1);
    std::memcpy(destination, source, length);
    destination[length] = 0;
    return 0;
}

FFXP_MSABI int msWcscpyS(char16_t* destination, size_t size, const char16_t* source)
{
    size_t i = 0;
    for (; source[i] && i + 1 < size; ++i)
        destination[i] = source[i];
    if (size)
        destination[i] = 0;
    return 0;
}

FFXP_MSABI int msGetenvS(size_t* required, char* buffer, size_t size, const char* name)
{
    const char* value = std::getenv(name);
    const size_t needed = value ? std::strlen(value) + 1 : 0;
    if (required)
        *required = needed;
    if (buffer && size) {
        if (value && needed <= size)
            std::memcpy(buffer, value, needed);
        else
            buffer[0] = 0;
    }
    return 0;
}

short msFloatClass(int category)
{
    switch (category) {
    case FP_NAN: return 2;
    case FP_INFINITE: return 1;
    case FP_ZERO: return 0;
    case FP_SUBNORMAL: return -2;
    default: return -1;
    }
}
FFXP_MSABI short msDclass(double x) { return msFloatClass(std::fpclassify(x)); }
FFXP_MSABI short msFdclass(float x) { return msFloatClass(std::fpclassify(x)); }
FFXP_MSABI int msDsign(double x) { return std::signbit(x) ? 0x8000 : 0; }
FFXP_MSABI int msFdsign(float x) { return std::signbit(x) ? 0x8000 : 0; }

using InitFunction = void(FFXP_MSABI*)();
using InitFunctionE = int(FFXP_MSABI*)();
FFXP_MSABI void msInitterm(InitFunction* first, InitFunction* last)
{
    for (; first < last; ++first)
        if (*first)
            (*first)();
}
FFXP_MSABI int msInittermE(InitFunctionE* first, InitFunctionE* last)
{
    for (; first < last; ++first)
        if (*first)
            if (int result = (*first)())
                return result;
    return 0;
}

FFXP_MSABI void* msGetCurrentProcess() { return reinterpret_cast<void*>(-1); }
FFXP_MSABI uint32_t msGetCurrentThreadId() { return static_cast<uint32_t>(syscall(SYS_gettid)); }
FFXP_MSABI uint32_t msGetCurrentProcessId() { return static_cast<uint32_t>(getpid()); }
FFXP_MSABI void msGetSystemTimeAsFileTime(uint64_t* out)
{
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    *out = (static_cast<uint64_t>(now.tv_sec) + 11644473600ull) * 10000000ull + static_cast<uint64_t>(now.tv_nsec) / 100;
}
FFXP_MSABI uint64_t msGetTickCount64()
{
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return static_cast<uint64_t>(now.tv_sec) * 1000ull + static_cast<uint64_t>(now.tv_nsec) / 1000000;
}
FFXP_MSABI int msQueryPerformanceCounter(int64_t* out)
{
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    *out = now.tv_sec * 10000000ll + now.tv_nsec / 100;
    return 1;
}
FFXP_MSABI void msInitializeSListHead(void* head) { std::memset(head, 0, 16); }

void debugOutput(const char* text)
{
    static const bool enabled = std::getenv("FFX_PORTABLE_TRACE") != nullptr;
    if (enabled)
        std::fprintf(stderr, "[ffx-portable] %s", text);
}
FFXP_MSABI void msOutputDebugStringA(const char* text) { debugOutput(text); }
FFXP_MSABI void msOutputDebugStringW(const char16_t* text)
{
    std::string storage;
    debugOutput(utf16ToAscii(text, storage));
}
FFXP_MSABI void* msGetModuleHandleA(const char* name) { return name ? nullptr : imageBase(); }

FFXP_MSABI int msMultiByteToWideChar(uint32_t, uint32_t, const char* source, int sourceLength, char16_t* target, int targetLength)
{
    const size_t length = sourceLength < 0 ? std::strlen(source) + 1 : static_cast<size_t>(sourceLength);
    if (!targetLength)
        return static_cast<int>(length);
    const size_t count = std::min<size_t>(length, targetLength);
    for (size_t i = 0; i < count; ++i)
        target[i] = static_cast<unsigned char>(source[i]);
    return static_cast<int>(count);
}
FFXP_MSABI int msWideCharToMultiByte(uint32_t, uint32_t, const char16_t* source, int sourceLength, char* target, int targetLength, const char*, int*)
{
    size_t length = static_cast<size_t>(sourceLength);
    if (sourceLength < 0) {
        length = 0;
        while (source[length])
            ++length;
        ++length;
    }
    if (!targetLength)
        return static_cast<int>(length);
    const size_t count = std::min<size_t>(length, targetLength);
    for (size_t i = 0; i < count; ++i)
        target[i] = source[i] < 128 ? static_cast<char>(source[i]) : '?';
    return static_cast<int>(count);
}

#define FFXP_MS(name, signature) {#name, reinterpret_cast<void*>(&Ms<pick<signature>(name)>::call)}
#define FFXP_BODY(name, function) {name, reinterpret_cast<void*>(&function)}

const std::pair<const char*, void*> kFunctions[] = {
    FFXP_MS(malloc, void*(size_t)), FFXP_MS(free, void(void*)), FFXP_MS(calloc, void*(size_t, size_t)),
    FFXP_MS(memcpy, void*(void*, const void*, size_t)), FFXP_MS(memmove, void*(void*, const void*, size_t)), FFXP_MS(memset, void*(void*, int, size_t)),
    FFXP_MS(memcmp, int(const void*, const void*, size_t)), FFXP_MS(strcmp, int(const char*, const char*)),
    FFXP_MS(log2, double(double)), FFXP_MS(floor, double(double)), FFXP_MS(floorf, float(float)), FFXP_MS(cosf, float(float)), FFXP_MS(sinf, float(float)),
    FFXP_MS(tanf, float(float)), FFXP_MS(atanf, float(float)), FFXP_MS(powf, float(float, float)), FFXP_MS(roundf, float(float)), FFXP_MS(log2f, float(float)),
    FFXP_MS(exp2f, float(float)),
    FFXP_BODY("__stdio_common_vsprintf", msVsprintf), FFXP_BODY("__stdio_common_vswprintf_s", msVswprintf), FFXP_BODY("strncpy_s", msStrncpyS),
    FFXP_BODY("wcscpy_s", msWcscpyS), FFXP_BODY("getenv_s", msGetenvS), FFXP_BODY("_dclass", msDclass), FFXP_BODY("_ldclass", msDclass),
    FFXP_BODY("_fdclass", msFdclass), FFXP_BODY("_dsign", msDsign), FFXP_BODY("_ldsign", msDsign), FFXP_BODY("_fdsign", msFdsign),
    FFXP_BODY("_initterm", msInitterm), FFXP_BODY("_initterm_e", msInittermE), FFXP_BODY("GetCurrentProcess", msGetCurrentProcess),
    FFXP_BODY("GetCurrentThreadId", msGetCurrentThreadId), FFXP_BODY("GetCurrentProcessId", msGetCurrentProcessId),
    FFXP_BODY("GetSystemTimeAsFileTime", msGetSystemTimeAsFileTime), FFXP_BODY("GetTickCount64", msGetTickCount64),
    FFXP_BODY("QueryPerformanceCounter", msQueryPerformanceCounter), FFXP_BODY("InitializeSListHead", msInitializeSListHead),
    FFXP_BODY("OutputDebugStringA", msOutputDebugStringA), FFXP_BODY("OutputDebugStringW", msOutputDebugStringW),
    FFXP_BODY("GetModuleHandleA", msGetModuleHandleA), FFXP_BODY("MultiByteToWideChar", msMultiByteToWideChar),
    FFXP_BODY("WideCharToMultiByte", msWideCharToMultiByte), FFXP_BODY("??0_Lockit@std@@QEAA@H@Z", returnSelf),
};

template <size_t N>
bool listed(const char* const (&list)[N], const std::string& name, size_t* index = nullptr)
{
    for (size_t i = 0; i < N; ++i) {
        if (name == list[i]) {
            if (index)
                *index = i;
            return true;
        }
    }
    return false;
}

}  // namespace

const char* utf16ToAscii(const char16_t* text, std::string& storage)
{
    storage.clear();
    for (; text && *text; ++text)
        storage += *text < 128 ? static_cast<char>(*text) : '?';
    return storage.c_str();
}

void setClientDeviceProcAddr(PFN_vkGetDeviceProcAddr getDeviceProcAddr) { g_clientDeviceProcAddr = getDeviceProcAddr; }
PFN_vkGetDeviceProcAddr dllDeviceProcAddr() { return reinterpret_cast<PFN_vkGetDeviceProcAddr>(&msGetDeviceProcAddr); }

void* resolveImport(const std::string& dll, const std::string& name, std::string& error)
{
    if (dll == "vulkan-1.dll") {
        if (name == "vkGetDeviceProcAddr")
            return reinterpret_cast<void*>(&msGetDeviceProcAddr);
        if (!g_libvulkan && !(g_libvulkan = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL))) {
            error = "could not load libvulkan.so.1";
            return nullptr;
        }
        return vulkanThunkFor(name.c_str(), dlsym(g_libvulkan, name.c_str()));
    }

    size_t index;
    if (listed(kFatalImports, name, &index))
        return fatalStub(index, std::make_index_sequence<std::size(kFatalImports)>{});
    if (listed(kReturnZeroImports, name))
        return reinterpret_cast<void*>(&returnZero);
    if (listed(kReturnOneImports, name))
        return reinterpret_cast<void*>(&returnOne);
    for (const auto& entry : kFunctions)
        if (name == entry.first)
            return entry.second;
    for (const auto& entry : kDataImports)
        if (name == entry.first)
            return entry.second;
    return nullptr;
}

}  // namespace ffxp
