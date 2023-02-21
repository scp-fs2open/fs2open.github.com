#define MODEL_SDR_FLAG_LIGHT		(1 << 0)
#define MODEL_SDR_FLAG_DEFERRED		(1 << 1)
#define MODEL_SDR_FLAG_HDR			(1 << 2)
#define MODEL_SDR_FLAG_DIFFUSE		(1 << 3)
#define MODEL_SDR_FLAG_GLOW			(1 << 4)
#define MODEL_SDR_FLAG_SPEC			(1 << 5)
#define MODEL_SDR_FLAG_ENV			(1 << 6)
#define MODEL_SDR_FLAG_NORMAL		(1 << 7)
#define MODEL_SDR_FLAG_AMBIENT		(1 << 8)
#define MODEL_SDR_FLAG_MISC			(1 << 9)
#define MODEL_SDR_FLAG_TEAMCOLOR	(1 << 10)
#define MODEL_SDR_FLAG_FOG			(1 << 11)
#define MODEL_SDR_FLAG_TRANSFORM	(1 << 12)
#define MODEL_SDR_FLAG_SHADOWS		(1 << 13)
#define MODEL_SDR_FLAG_THRUSTER		(1 << 14)
#define MODEL_SDR_FLAG_ALPHA_MULT	(1 << 15)


#define FLAG_ACTIVE(flag) (flags & flag) != 0