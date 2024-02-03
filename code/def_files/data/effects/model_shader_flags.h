#ifdef MODEL_SDR_FLAG_MODE_CPP
#define SDR_FLAG(name, value) constexpr int name = value;
#elif defined MODEL_SDR_FLAG_MODE_CPP_ARRAY
#define SDR_FLAG(name, value) {SDR_TYPE_MODEL, true, value, #name, {}, #name},
#elif defined MODEL_SDR_FLAG_MODE_GLSL
#define SDR_FLAG(name, value) const int name = value;
#else
#error Make sure to properly define the usage mode for this file!
#endif

SDR_FLAG(MODEL_SDR_FLAG_LIGHT	      , (1 << 0))
SDR_FLAG(MODEL_SDR_FLAG_DEFERRED      , (1 << 1))
SDR_FLAG(MODEL_SDR_FLAG_HDR		      , (1 << 2))
SDR_FLAG(MODEL_SDR_FLAG_DIFFUSE	      , (1 << 3))
SDR_FLAG(MODEL_SDR_FLAG_GLOW	      , (1 << 4))
SDR_FLAG(MODEL_SDR_FLAG_SPEC	      , (1 << 5))
SDR_FLAG(MODEL_SDR_FLAG_ENV		      , (1 << 6))
SDR_FLAG(MODEL_SDR_FLAG_NORMAL	      , (1 << 7))
SDR_FLAG(MODEL_SDR_FLAG_AMBIENT	      , (1 << 8))
SDR_FLAG(MODEL_SDR_FLAG_MISC	      , (1 << 9))
SDR_FLAG(MODEL_SDR_FLAG_TEAMCOLOR     , (1 << 10))
SDR_FLAG(MODEL_SDR_FLAG_FOG		      , (1 << 11))
SDR_FLAG(MODEL_SDR_FLAG_TRANSFORM     , (1 << 12))
SDR_FLAG(MODEL_SDR_FLAG_SHADOWS	      , (1 << 13))
SDR_FLAG(MODEL_SDR_FLAG_THRUSTER      , (1 << 14))
SDR_FLAG(MODEL_SDR_FLAG_ALPHA_MULT    , (1 << 15))

#ifndef MODEL_SDR_FLAG_MODE_GLSL
//The following ones are used ONLY as compile-time flags, but they still need to be defined here to ensure no conflict occurs
//But since these are checked with ifdefs even for the large shader, they must never be available in GLSL mode

SDR_FLAG(MODEL_SDR_FLAG_SHADOW_MAP    ,	(1 << 16))
SDR_FLAG(MODEL_SDR_FLAG_THICK_OUTLINES, (1 << 17))

#endif