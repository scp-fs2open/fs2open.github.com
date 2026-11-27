#include "scripting/ScriptingTestFixture.h"

#include "graphics/2d.h"
#include "graphics/grstub.h"
#include "options/OptionsManager.h"

// RequirementMet asks the renderer what it can do, so these tests run on the stub renderer, as -get_options does.
class OptionsTest : public test::scripting::ScriptingTestFixture {
  public:
	OptionsTest() : test::scripting::ScriptingTestFixture(INIT_CFILE) { pushModDir("options"); }

  protected:
	void SetUp() override
	{
		ScriptingTestFixture::SetUp();
		_saved_screen = gr_screen;
		gr_stub_init_function_pointers();
	}
	void TearDown() override
	{
		options::OptionsManager::instance()->clearRequirements();
		gr_screen = _saved_screen;
		ScriptingTestFixture::TearDown();
	}

  private:
	screen _saved_screen;
};

TEST_F(OptionsTest, requirement)
{
	options::OptionsManager::instance()->setRequirement("Graphics.VSync", "vulkan", "Needs the Vulkan renderer");
	options::OptionsManager::instance()->setRequirement("Graphics.Gamma", "unknown", "Needs something");
	this->EvalTestScript();
}

TEST_F(OptionsTest, requirementMetOnVulkan)
{
	gr_screen.mode = GraphicsAPI::Vulkan;
	options::OptionsManager::instance()->setRequirement("Graphics.VSync", "vulkan", "Needs the Vulkan renderer");
	options::OptionsManager::instance()->setRequirement("Graphics.Gamma", "raytracing", "Needs ray tracing hardware");
	this->EvalTestScript();
}
