#include "graphics/paths/PathRenderer.h"

#include "graphics/paths/NVGRenderer.h"
#include "graphics/paths/StubRenderer.h"

#include "graphics/2d.h"

namespace graphics
{
    namespace paths
    {
        std::unique_ptr<PathRenderer> PathRenderer::s_instance;
        
        bool PathRenderer::init()
        {
            if (gr_screen.mode == GR_OPENGL)
            {
                s_instance = std::unique_ptr<PathRenderer>(new NVGRenderer());
            }
            else
            {
                s_instance = std::unique_ptr<PathRenderer>(new StubRenderer());
            }

            return true;
        }

        void PathRenderer::shutdown()
        {
            s_instance = nullptr;
        }
    }
}