// The one place where the checkpoint codec meets the generated field lists and the rules (see checkpoint/codec.h).
// Instantiates the codec for every scanned struct and enum, so that the build fails as soon as a struct gains a member
// the codec has no rule for, and so that engine files, which only see the declarations, can save these types.

#define CHECKPOINT_CODEC_IMPLEMENTATION
#include "checkpoint/codec_impl.h"

#include "checkpoint/checkpoint_fields.inc"

#include "checkpoint/rules.h"

namespace checkpoint {

#define CHECKPOINT_INSTANTIATE_SCANNED(T) CHECKPOINT_INSTANTIATE(T);
#define CHECKPOINT_INSTANTIATE_FLAGS(E) CHECKPOINT_INSTANTIATE(flagset<E>);

CHECKPOINT_SCANNED_STRUCTS(CHECKPOINT_INSTANTIATE_SCANNED)
CHECKPOINT_SCANNED_ENUMS(CHECKPOINT_INSTANTIATE_SCANNED)
CHECKPOINT_SCANNED_FLAGS(CHECKPOINT_INSTANTIATE_FLAGS)

#undef CHECKPOINT_INSTANTIATE_SCANNED
#undef CHECKPOINT_INSTANTIATE_FLAGS

namespace {

void mix(uint32_t& hash, const void* data, size_t size)
{
	auto bytes = static_cast<const ubyte*>(data);
	for (size_t i = 0; i < size; ++i) {
		hash ^= bytes[i];
		hash *= 16777619u;
	}
}

void mix(uint32_t& hash, const char* text)
{
	mix(hash, text, strlen(text) + 1);
}

template <typename T>
struct FingerprintVisitor {
	uint32_t& hash;

	template <auto Member>
	void field(const char* name)
	{
		using F = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T&>().*Member)>>;
		uint32_t size = sizeof(F);
		mix(hash, name);
		mix(hash, &size, sizeof(size));
	}
};

} // namespace

uint32_t scanned_struct_fingerprint()
{
	uint32_t hash = 2166136261u;
#define CHECKPOINT_FINGERPRINT_STRUCT(S)                                                                               \
	{                                                                                                                  \
		mix(hash, #S);                                                                                                 \
		FingerprintVisitor<S> visitor{hash};                                                                           \
		Fields<S>::each(visitor);                                                                                      \
	}
	CHECKPOINT_SCANNED_STRUCTS(CHECKPOINT_FINGERPRINT_STRUCT)
#undef CHECKPOINT_FINGERPRINT_STRUCT
	return hash;
}

} // namespace checkpoint
