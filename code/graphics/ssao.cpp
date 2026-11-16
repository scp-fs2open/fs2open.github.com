#include "graphics/ssao.h"

#include "graphics/2d.h"
#include "graphics/rtao.h"
#include "options/Option.h"

int Ssao_quality = 0;
ssao_settings Ssao_settings;

// coverity[GLOBAL_INIT_ORDER] -- safe; OptionBuilder::finish() uses Meyers singleton
auto SsaoQualityOption = options::OptionBuilder<int>("Graphics.SsaoQuality",
                     std::pair<const char*, int>{"Screen-Space Ambient Occlusion", -1},
                     std::pair<const char*, int>{"Ambient occlusion from the screen contents (CORTAO); raytraced ambient occlusion has priority when it is on", -1})
                     .enumerator([]() -> SCP_vector<int> {
                         if (ssao_supported()) {
                             return {0, 1};
                         }
                         return {0};
                     })
                     .display(options::MapValueDisplay<int>({
                         {0, {"Off", -1}},
                         {1, {"On", -1}}
                     }))
                     .bind_to(&Ssao_quality)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .level(options::ExpertLevel::Advanced)
                     .category(std::make_pair("Graphics", 1825))
                     .default_func([]() { return 0; })
                     .importance(70)
                     .finish();

bool ssao_supported()
{
	return gr_screen.mode == GraphicsAPI::Vulkan;
}

bool ssao_enabled()
{
	return Ssao_quality > 0 && ssao_supported() && !rtao_enabled();
}

void ssao_remove_unsupported_options()
{
	if (!ssao_supported()) {
		options::OptionsManager::instance()->removeOption(SsaoQualityOption);
	}
}
