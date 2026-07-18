#include "graphics/rtao.h"

#include "graphics/2d.h"
#include "options/Option.h"

int Rtao_samples = 0;

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto RtaoQualityOption = options::OptionBuilder<int>("Graphics.RtaoQuality",
                     std::pair<const char*, int>{"Raytraced Ambient Occlusion", -1},
                     std::pair<const char*, int>{"Rays traced per pixel for ambient occlusion in the deferred ambient pass; Off keeps baked AO maps only", -1})
                     .enumerator([]() -> SCP_vector<int> {
                         if (rtao_supported()) {
                             return {0, 4, 8, 16};
                         }
                         return {0}; // inert default when RT isn't supported
                     })
                     .display(options::MapValueDisplay<int>({
                         {0, {"Off", -1}},
                         {4, {"Low", -1}},
                         {8, {"Medium", -1}},
                         {16, {"High", -1}}
                     }))
                     .bind_to(&Rtao_samples)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return 0; })
                     .importance(71)
                     .finish();

bool rtao_supported()
{
	return gr_is_capable(gr_capability::CAPABILITY_RAYTRACED_SHADOWS);
}

bool rtao_enabled()
{
	return Rtao_samples > 0 && rtao_supported();
}
