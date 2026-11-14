// The FFX API as the client links it. Each call crosses from sysv into the DLL's ms_abi, so anything in
// the descriptor chain that the DLL will call back into has to be swapped, then restored for the client.
#include "../../include/ffx_portable.h"
#include "shim.h"

#include <FidelityFX/upscalers/include/ffx_upscale.h>

#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>

namespace ffxp {

namespace {

// The DLL's own lock imports are no-ops; this makes that correct, since it creates no threads of its own.
std::recursive_mutex g_mutex;

enum class State { NotLoaded, Loaded, Unavailable };
State g_state = State::NotLoaded;
std::string g_error;
DllExports g_dll;
std::jmp_buf* g_activeCall;

ffxApiMessage g_clientMessage;
FfxApiConstantBufferAllocator g_clientConstantBufferAllocator;

// Failure handling has to cross the DLL's frames, which have no Linux unwind information.
template <typename Function>
bool runGuarded(Function&& function)
{
    std::jmp_buf call;
    std::jmp_buf* previous = g_activeCall;
    g_activeCall = &call;
    const bool completed = setjmp(call) == 0;
    if (completed)
        function();
    g_activeCall = previous;
    return completed;
}

bool ensureLoaded()
{
    if (g_state == State::NotLoaded) {
        bool loaded = false;
        const bool completed = runGuarded([&] { loaded = loadDll(g_dll, g_error); });
        if (!completed) {
            g_state = State::Unavailable;
        } else {
            g_state = loaded ? State::Loaded : State::Unavailable;
        }
    }
    return g_state == State::Loaded;
}

FFXP_MSABI void messageThunk(uint32_t type, const char16_t* message)
{
    std::wstring wide;
    for (; message && *message; ++message)
        wide += static_cast<wchar_t>(*message);
    if (g_clientMessage)
        g_clientMessage(type, wide.c_str());
}

FFXP_MSABI FfxApiConstantBufferAllocation constantBufferThunk(void* data, const uint64_t size) { return g_clientConstantBufferAllocator(data, size); }

FFXP_MSABI void* allocThunk(void* callbacks, uint64_t size)
{
    auto* client = static_cast<const ffxAllocationCallbacks*>(callbacks);
    return client->alloc(client->pUserData, size);
}
FFXP_MSABI void deallocThunk(void* callbacks, void* memory)
{
    auto* client = static_cast<const ffxAllocationCallbacks*>(callbacks);
    client->dealloc(client->pUserData, memory);
}

// Same layout as ffxAllocationCallbacks, with functions the DLL can call. The docs promise pUserData isn't stored.
struct MsAllocationCallbacks {
    MsAllocationCallbacks(const ffxAllocationCallbacks* client)
    {
        callbacks.pUserData = const_cast<ffxAllocationCallbacks*>(client);
        callbacks.alloc = reinterpret_cast<ffxAlloc>(&allocThunk);
        callbacks.dealloc = reinterpret_cast<ffxDealloc>(&deallocThunk);
        pointer = client ? &callbacks : nullptr;
    }
    ffxAllocationCallbacks callbacks;
    const ffxAllocationCallbacks* pointer;
};

// Swaps client pointers in a descriptor chain for the duration of one call.
class ChainPatch {
public:
    explicit ChainPatch(ffxApiHeader* chain)
    {
        for (ffxApiHeader* node = chain; node; node = node->pNext) {
            switch (node->type) {
            case FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK:
                patchDeviceContext(reinterpret_cast<ffxCreateBackendVKDesc*>(node)->deviceContext);
                break;
            case FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK_DEPRECATED:
                patchGetter(reinterpret_cast<ffxCreateBackendVKDesc_DEPRECATED*>(node)->vkDeviceProcAddr);
                break;
            case FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK_ALLOCATION_CALLBACKS: {
                auto* allocation = reinterpret_cast<ffxCreateBackendVKAllocationCallbacksDesc*>(node);
                if (allocation->pfnFfxConstantBufferAllocator) {
                    g_clientConstantBufferAllocator = allocation->pfnFfxConstantBufferAllocator;
                    patch(allocation->pfnFfxConstantBufferAllocator, reinterpret_cast<FfxApiConstantBufferAllocator>(&constantBufferThunk));
                }
                break;
            }
            case FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE:
                patchMessage(reinterpret_cast<ffxCreateContextDescUpscale*>(node)->fpMessage);
                break;
            case FFX_API_CONFIGURE_DESC_TYPE_GLOBALDEBUG:
                patchMessage(reinterpret_cast<ffxConfigureDescGlobalDebug*>(node)->fpMessage);
                break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            case FFX_API_CONFIGURE_DESC_TYPE_GLOBALDEBUG1:
                patchMessage(reinterpret_cast<ffxConfigureDescGlobalDebug1*>(node)->fpMessage);
                break;
#pragma GCC diagnostic pop
            case FFX_API_QUERY_DESC_TYPE_GET_VERSIONS:
                patchDevicePointer(reinterpret_cast<ffxQueryDescGetVersions*>(node)->device);
                break;
            case FFX_API_QUERY_DESC_TYPE_UPSCALE_GPU_MEMORY_USAGE_V2:
                patchDevicePointer(reinterpret_cast<ffxQueryDescUpscaleGetGPUMemoryUsageV2*>(node)->device);
                break;
            default:
                break;
            }
        }
    }
    ~ChainPatch()
    {
        for (int i = m_count - 1; i >= 0; --i)
            *m_entries[i].slot = m_entries[i].original;
    }
    ChainPatch(const ChainPatch&) = delete;
    ChainPatch& operator=(const ChainPatch&) = delete;

private:
    struct Entry {
        void** slot;
        void* original;
    };

    template <typename T>
    void patch(T& slot, T value)
    {
        static_assert(sizeof(T) == sizeof(void*));
        if (m_count == kMaxEntries) {
            std::fprintf(stderr, "ffx-portable: descriptor chain too long\n");
            std::abort();
        }
        m_entries[m_count++] = {reinterpret_cast<void**>(&slot), reinterpret_cast<void*>(slot)};
        slot = value;
    }
    void patchGetter(PFN_vkGetDeviceProcAddr& getter)
    {
        setClientDeviceProcAddr(getter);
        patch(getter, dllDeviceProcAddr());
    }
    void patchDeviceContext(VkDeviceContext& context) { patchGetter(context.vkDeviceProcAddr); }
    void patchDevicePointer(void*& device)
    {
        if (!device)
            return;
        m_contexts[m_contextCount] = *static_cast<VkDeviceContext*>(device);
        setClientDeviceProcAddr(m_contexts[m_contextCount].vkDeviceProcAddr);
        m_contexts[m_contextCount].vkDeviceProcAddr = dllDeviceProcAddr();
        patch(device, static_cast<void*>(&m_contexts[m_contextCount]));
        ++m_contextCount;
    }
    void patchMessage(ffxApiMessage& message)
    {
        if (!message)
            return;
        g_clientMessage = message;
        patch(message, reinterpret_cast<ffxApiMessage>(&messageThunk));
    }

    static constexpr int kMaxEntries = 16;
    Entry m_entries[kMaxEntries];
    VkDeviceContext m_contexts[kMaxEntries];
    int m_count = 0;
    int m_contextCount = 0;
};

// Every call into the DLL goes through here. `call` must not own anything needing destruction across
// the DLL call, because a failure inside the DLL leaves through longjmp.
template <typename Call>
ffxReturnCode_t enter(ffxApiHeader* chain, Call&& call)
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    if (!ensureLoaded() || !enterThread())
        return FFX_API_RETURN_NO_PROVIDER;

    ChainPatch patch(chain);
    ffxReturnCode_t result = FFX_API_RETURN_ERROR_RUNTIME_ERROR;
    if (!runGuarded([&] { result = call(); })) {
        g_state = State::Unavailable;
        return FFX_API_RETURN_ERROR_RUNTIME_ERROR;
    }
    return result;
}

}  // namespace

void dllFailure(const std::string& reason)
{
    if (g_error.empty())
        g_error = reason;
    if (g_activeCall)
        std::longjmp(*g_activeCall, 1);
    std::fprintf(stderr, "ffx-portable: %s\n", reason.c_str());
    std::abort();
}

}  // namespace ffxp

using namespace ffxp;

extern "C" {

bool ffxPortableCanLoad()
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    return ensureLoaded();
}

const char* ffxPortableLoadError()
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    ensureLoaded();
    return g_state == State::Loaded ? "" : g_error.c_str();
}

ffxReturnCode_t ffxCreateContext(ffxContext* context, ffxCreateContextDescHeader* desc, const ffxAllocationCallbacks* memCb)
{
    MsAllocationCallbacks callbacks(memCb);
    return enter(desc, [&] { return g_dll.createContext(context, desc, callbacks.pointer); });
}

ffxReturnCode_t ffxDestroyContext(ffxContext* context, const ffxAllocationCallbacks* memCb)
{
    MsAllocationCallbacks callbacks(memCb);
    return enter(nullptr, [&] { return g_dll.destroyContext(context, callbacks.pointer); });
}

ffxReturnCode_t ffxConfigure(ffxContext* context, const ffxConfigureDescHeader* desc)
{
    return enter(const_cast<ffxApiHeader*>(desc), [&] { return g_dll.configure(context, desc); });
}

ffxReturnCode_t ffxQuery(ffxContext* context, ffxQueryDescHeader* desc)
{
    return enter(desc, [&] { return g_dll.query(context, desc); });
}

ffxReturnCode_t ffxDispatch(ffxContext* context, const ffxDispatchDescHeader* desc)
{
#if defined(FFX_PORTABLE_TESTING)
    if (std::getenv("FFX_PORTABLE_TEST_FATAL")) {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        if (ensureLoaded() && !runGuarded([] { dllFailure("forced failure for testing"); })) {
            g_state = State::Unavailable;
            return FFX_API_RETURN_ERROR_RUNTIME_ERROR;
        }
    }
#endif
    return enter(const_cast<ffxApiHeader*>(desc), [&] { return g_dll.dispatch(context, desc); });
}

}  // extern "C"
