// Backend for platforms with no way to run AMD's runtime: FSR is reported as unavailable.
#include "../include/ffx_portable.h"

extern "C" {

bool ffxPortableCanLoad() { return false; }
const char* ffxPortableLoadError() { return "FSR is not available on this platform"; }

ffxReturnCode_t ffxCreateContext(ffxContext*, ffxCreateContextDescHeader*, const ffxAllocationCallbacks*) { return FFX_API_RETURN_NO_PROVIDER; }
ffxReturnCode_t ffxDestroyContext(ffxContext*, const ffxAllocationCallbacks*) { return FFX_API_RETURN_NO_PROVIDER; }
ffxReturnCode_t ffxConfigure(ffxContext*, const ffxConfigureDescHeader*) { return FFX_API_RETURN_NO_PROVIDER; }
ffxReturnCode_t ffxQuery(ffxContext*, ffxQueryDescHeader*) { return FFX_API_RETURN_NO_PROVIDER; }
ffxReturnCode_t ffxDispatch(ffxContext*, const ffxDispatchDescHeader*) { return FFX_API_RETURN_NO_PROVIDER; }

}  // extern "C"
