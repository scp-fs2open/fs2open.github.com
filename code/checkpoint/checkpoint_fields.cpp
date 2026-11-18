// Instantiates the codec for every struct in checkpoint_structs.txt, so that the build fails as soon as one of them
// gains a member the codec has no rule for, even before any code saves that struct.

#include "checkpoint/serialize.h"

namespace checkpoint {
namespace {

#define CHECKPOINT_CHECK_STRUCT(S)                                                                                     \
	[[maybe_unused]] void check_##S(Writer& w, Reader& r, S& v)                                                        \
	{                                                                                                                  \
		save(w, v);                                                                                                    \
		load(r, v);                                                                                                    \
	}

CHECKPOINT_SCANNED_STRUCTS(CHECKPOINT_CHECK_STRUCT)

} // namespace
} // namespace checkpoint
