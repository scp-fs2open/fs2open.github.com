// PE32+ loader for one exact file. The hash is checked before any parsing, so the parser can trust the
// file's structure and only has to handle what this one DLL contains.
#include "shim.h"

#include <asm/prctl.h>
#include <dlfcn.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>

namespace ffxp {

namespace {

constexpr const char* kDllSha256 = FFX_PORTABLE_UPSCALER_SHA256;  // From CMakeLists.txt, which also downloads against it.

constexpr uint32_t kSectionExecute = 0x20000000, kSectionRead = 0x40000000, kSectionWrite = 0x80000000;

// Offsets into the fake Windows thread block that MSVC-compiled code reads through %gs.
constexpr size_t kTebStackBase = 0x08, kTebStackLimit = 0x10, kTebSelf = 0x30, kTebTlsVector = 0x58, kTebSize = 0x1000;

uint8_t* g_base;
size_t g_imageSize;
const uint8_t* g_tlsTemplate;
size_t g_tlsSize;

template <typename T>
T read(const uint8_t* at)
{
    T value;
    std::memcpy(&value, at, sizeof(T));
    return value;
}

std::string errnoText(const char* what) { return std::string(what) + ": " + std::strerror(errno); }

std::string directoryOf(const std::string& path)
{
    auto slash = path.find_last_of('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
}

// AMD's own lookup rule is next to the executable, then next to the library that loads effects. The "lib"
// locations cover Linux bin/ + lib/ installs such as AppImages. The hash check makes the order safe.
bool findDll(std::string& path)
{
    std::vector<std::string> directories;
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (length > 0) {
        const std::string executableDirectory = directoryOf(std::string(buffer, size_t(length)));
        directories.push_back(executableDirectory);
        directories.push_back(executableDirectory + "/lib");
        directories.push_back(executableDirectory + "/../lib");
    }
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&findDll), &info) && info.dli_fname)
        directories.push_back(directoryOf(info.dli_fname));
    for (const auto& directory : directories) {
        path = directory + "/" + kDllName;
        if (access(path.c_str(), R_OK) == 0)
            return true;
    }
    return false;
}

struct ThreadBlock {
    uint8_t teb[kTebSize] = {};
    void* tlsVector[8] = {};
    std::unique_ptr<uint8_t[]> tlsData;
    bool ready = false;
    bool failed = false;
};

}  // namespace

uint8_t* imageBase() { return g_base; }

bool enterThread()
{
    thread_local ThreadBlock block;
    if (block.ready)
        return true;
    if (block.failed)
        return false;

    void* self = block.teb;
    std::memcpy(block.teb + kTebSelf, &self, sizeof(void*));
    // A zero limit makes MSVC's __chkstk skip probing; Linux grows thread stacks on demand or has them mapped.
    const uint64_t stackBase = reinterpret_cast<uint64_t>(__builtin_frame_address(0)) + (1 << 20), stackLimit = 0;
    std::memcpy(block.teb + kTebStackBase, &stackBase, sizeof(stackBase));
    std::memcpy(block.teb + kTebStackLimit, &stackLimit, sizeof(stackLimit));
    block.tlsData.reset(new uint8_t[g_tlsSize + 4096]());
    if (g_tlsTemplate)
        std::memcpy(block.tlsData.get(), g_tlsTemplate, g_tlsSize);
    block.tlsVector[0] = block.tlsData.get();
    void** vector = block.tlsVector;
    std::memcpy(block.teb + kTebTlsVector, &vector, sizeof(vector));

    if (syscall(SYS_arch_prctl, ARCH_SET_GS, block.teb) != 0) {
        block.failed = true;
        return false;
    }
    block.ready = true;
    return true;
}

bool loadDll(DllExports& exports, std::string& error)
{
    std::string path;
    if (!findDll(path)) {
        error = std::string(kDllName) + " was not found next to the executable or in its lib directory";
        return false;
    }

    std::ifstream in(path, std::ios::binary | std::ios::ate);
    std::vector<uint8_t> file(size_t(in.tellg()));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(file.data()), std::streamsize(file.size()));
    if (sha256Hex(file.data(), file.size()) != kDllSha256) {
        error = path + " is not the expected build (SHA-256 mismatch)";
        return false;
    }

    const uint8_t* bytes = file.data();
    const uint32_t pe = read<uint32_t>(bytes + 0x3c), optional = pe + 24;
    const uint16_t sectionCount = read<uint16_t>(bytes + pe + 6), optionalSize = read<uint16_t>(bytes + pe + 20);
    const uint64_t preferredBase = read<uint64_t>(bytes + optional + 24);
    const uint32_t entryPoint = read<uint32_t>(bytes + optional + 16), headersSize = read<uint32_t>(bytes + optional + 60);
    g_imageSize = read<uint32_t>(bytes + optional + 56);
    auto directory = [&](int index) { return read<uint32_t>(bytes + optional + 112 + 8 * index); };

    g_base = static_cast<uint8_t*>(mmap(nullptr, g_imageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (g_base == MAP_FAILED) {
        g_base = nullptr;
        error = errnoText("mmap");
        return false;
    }
    auto fail = [&](const std::string& reason) {
        munmap(g_base, g_imageSize);
        g_base = nullptr;
        error = reason;
        return false;
    };

    std::memcpy(g_base, bytes, headersSize);
    struct Section {
        uint32_t address, size, flags;
    };
    std::vector<Section> sections;
    for (uint16_t i = 0; i < sectionCount; ++i) {
        const uint8_t* header = bytes + optional + optionalSize + i * 40u;
        const uint32_t virtualSize = read<uint32_t>(header + 8), address = read<uint32_t>(header + 12), rawSize = read<uint32_t>(header + 16),
                       rawOffset = read<uint32_t>(header + 20);
        if (rawSize)
            std::memcpy(g_base + address, bytes + rawOffset, std::min(rawSize, virtualSize ? virtualSize : rawSize));
        sections.push_back({address, std::max(virtualSize, rawSize), read<uint32_t>(header + 36)});
    }

    const int64_t delta = reinterpret_cast<int64_t>(g_base) - static_cast<int64_t>(preferredBase);
    const uint32_t relocations = directory(5), relocationsSize = read<uint32_t>(bytes + optional + 112 + 8 * 5 + 4);
    for (uint32_t offset = 0; offset < relocationsSize;) {
        const uint32_t page = read<uint32_t>(g_base + relocations + offset), blockSize = read<uint32_t>(g_base + relocations + offset + 4);
        for (uint32_t i = 0; i < (blockSize - 8) / 2; ++i) {
            const uint16_t entry = read<uint16_t>(g_base + relocations + offset + 8 + i * 2);
            if (entry >> 12 == 10)
                *reinterpret_cast<uint64_t*>(g_base + page + (entry & 0xfff)) += delta;
        }
        offset += blockSize;
    }

    for (auto* descriptor = reinterpret_cast<uint32_t*>(g_base + directory(1)); descriptor[3]; descriptor += 5) {
        const std::string dll = reinterpret_cast<char*>(g_base + descriptor[3]);
        auto* names = reinterpret_cast<uint64_t*>(g_base + (descriptor[0] ? descriptor[0] : descriptor[4]));
        auto* slots = reinterpret_cast<uint64_t*>(g_base + descriptor[4]);
        for (; *names; ++names, ++slots) {
            std::string resolveError;
            const std::string name = reinterpret_cast<char*>(g_base + (*names & 0x7fffffff) + 2);
            void* target = (*names >> 63) ? nullptr : resolveImport(dll, name, resolveError);
            if (!target)
                return fail(resolveError.empty() ? "unsupported import " + dll + "!" + name : resolveError);
            *slots = reinterpret_cast<uint64_t>(target);
        }
    }

    uint64_t tlsCallbacks = 0;
    if (const uint32_t tls = directory(9)) {
        auto* tlsDirectory = reinterpret_cast<uint64_t*>(g_base + tls);
        g_tlsSize = size_t(tlsDirectory[1] - tlsDirectory[0]);
        g_tlsTemplate = reinterpret_cast<const uint8_t*>(tlsDirectory[0]);
        *reinterpret_cast<uint32_t*>(tlsDirectory[2]) = 0;
        tlsCallbacks = tlsDirectory[3];
    }

    if (mprotect(g_base, (headersSize + 4095) & ~4095u, PROT_READ) != 0)
        return fail(errnoText("mprotect"));
    for (const Section& section : sections) {
        const int protection = ((section.flags & kSectionRead) ? PROT_READ : 0) | ((section.flags & kSectionWrite) ? PROT_WRITE : 0) |
                               ((section.flags & kSectionExecute) ? PROT_EXEC : 0);
        if (mprotect(g_base + section.address, (uint64_t(section.size) + 4095) & ~4095ull, protection) != 0)
            return fail(errnoText("executable memory refused by system policy: mprotect"));
    }

    if (!enterThread())
        return fail(errnoText("thread setup refused by system policy: arch_prctl"));

    using DllMain = int(FFXP_MSABI*)(void*, uint32_t, void*);
    constexpr uint32_t kProcessAttach = 1;
    if (tlsCallbacks)
        for (auto* callback = reinterpret_cast<uint64_t*>(tlsCallbacks); *callback; ++callback)
            reinterpret_cast<DllMain>(*callback)(g_base, kProcessAttach, nullptr);
    if (entryPoint && !reinterpret_cast<DllMain>(g_base + entryPoint)(g_base, kProcessAttach, nullptr))
        return fail(std::string(kDllName) + ": DllMain failed");

    auto* exportDirectory = reinterpret_cast<uint32_t*>(g_base + directory(0));
    auto* functions = reinterpret_cast<uint32_t*>(g_base + exportDirectory[7]);
    auto* names = reinterpret_cast<uint32_t*>(g_base + exportDirectory[8]);
    auto* ordinals = reinterpret_cast<uint16_t*>(g_base + exportDirectory[9]);
    std::map<std::string, void*> table;
    for (uint32_t i = 0; i < exportDirectory[6]; ++i)
        table[reinterpret_cast<char*>(g_base + names[i])] = g_base + functions[ordinals[i]];
    exports.createContext = reinterpret_cast<decltype(exports.createContext)>(table["ffxCreateContext"]);
    exports.destroyContext = reinterpret_cast<decltype(exports.destroyContext)>(table["ffxDestroyContext"]);
    exports.configure = reinterpret_cast<decltype(exports.configure)>(table["ffxConfigure"]);
    exports.query = reinterpret_cast<decltype(exports.query)>(table["ffxQuery"]);
    exports.dispatch = reinterpret_cast<decltype(exports.dispatch)>(table["ffxDispatch"]);
    if (!exports.createContext || !exports.destroyContext || !exports.configure || !exports.query || !exports.dispatch)
        return fail("DLL does not export the FFX API");
    return true;
}

}  // namespace ffxp
