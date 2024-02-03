#ifdef MODEL_SDR_FLAG_MODE_CPP
#define SDR_FLAG(name, value, geoshader) constexpr int name = value;
#elif defined MODEL_SDR_FLAG_MODE_CPP_ARRAY
#define SDR_FLAG(name, value, geoshader) {SDR_TYPE_MODEL, geoshader, value, #name, {}, #name},
#elif defined MODEL_SDR_FLAG_MODE_GLSL
#define SDR_FLAG(name, value, geoshader) const int name = value;
#else
#error Make sure to properly define the usage mode for this file!
#endif

SDR_FLAG(MODEL_SDR_FLAG_LIGHT	      , (1 << 0) , false)
SDR_FLAG(MODEL_SDR_FLAG_DEFERRED      , (1 << 1) , false)
SDR_FLAG(MODEL_SDR_FLAG_HDR		      , (1 << 2) , false)
SDR_FLAG(MODEL_SDR_FLAG_DIFFUSE	      , (1 << 3) , false)
SDR_FLAG(MODEL_SDR_FLAG_GLOW	      , (1 << 4) , false)
SDR_FLAG(MODEL_SDR_FLAG_SPEC	      , (1 << 5) , false)
SDR_FLAG(MODEL_SDR_FLAG_ENV		      , (1 << 6) , false)
SDR_FLAG(MODEL_SDR_FLAG_NORMAL	      , (1 << 7) , false)
SDR_FLAG(MODEL_SDR_FLAG_AMBIENT	      , (1 << 8) , false)
SDR_FLAG(MODEL_SDR_FLAG_MISC	      , (1 << 9) , false)
SDR_FLAG(MODEL_SDR_FLAG_TEAMCOLOR     , (1 << 10), false)
SDR_FLAG(MODEL_SDR_FLAG_FOG		      , (1 << 11), false)
SDR_FLAG(MODEL_SDR_FLAG_TRANSFORM     , (1 << 12), false)
SDR_FLAG(MODEL_SDR_FLAG_SHADOWS	      , (1 << 13), false)
SDR_FLAG(MODEL_SDR_FLAG_THRUSTER      , (1 << 14), false)
SDR_FLAG(MODEL_SDR_FLAG_ALPHA_MULT    , (1 << 15), false)

#ifndef MODEL_SDR_FLAG_MODE_GLSL
//The following ones are used ONLY as compile-time flags, but they still need to be defined here to ensure no conflict occurs
//But since these are checked with ifdefs even for the large shader, they must never be available in GLSL mode

SDR_FLAG(MODEL_SDR_FLAG_SHADOW_MAP    ,	(1 << 16), true)
SDR_FLAG(MODEL_SDR_FLAG_THICK_OUTLINES, (1 << 17), true)

#endif