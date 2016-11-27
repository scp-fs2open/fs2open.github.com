
#include "tracing/categories.h"

namespace tracing {

Category::Category(const char* name, bool is_graphics) : _name(name), _graphics_category(is_graphics) {
}
const char* Category::getName() const {
	return _name;
}
bool Category::usesGPUCounter() const {
	return _graphics_category;
}

Category LuaOnFrame("LUA On Frame", true);

Category DrawSceneTexture("Draw scene texture", true);
Category UpdateDistortion("Update distortion", true);

Category Tonemapping("Tonemapping", true);
Category Bloom("Bloom", true);
Category BloomBrightPass("Bloom bright pass", true);
Category BloomIterationStep("Bloom iteration step", true);
Category BloomCompositeStep("Bloom composite step", true);
Category FXAA("FXAA", true);
Category Lightshafts("Lightshafts", true);
Category DrawPostEffects("Draw post effects", true);

Category RenderBatchItem("Render batch item", true);
Category RenderBatchBuffer("Render batch buffer", true);
Category LoadBatchingBuffers("Load batching buffers", true);

Category SortColliders("Sort Colliders", false);
Category FindOverlapColliders("Find overlap colliders", false);
Category CollidePair("Collide Pair", false);

Category WeaponPostMove("Weapon post move", false);
Category ShipPostMove("Ship post move", false);
Category FireballPostMove("Fireball post move", false);
Category DebrisPostMove("Debris post move", false);
Category AsteroidPostMove("Asteroid post move", false);
Category PreMove("Pre Move", false);
Category Physics("Physics", false);
Category PostMove("Post Move", false);
Category CollisionDetection("Collision Detection", false);

Category RenderBuffer("Render Buffer", true);

Category QueueRender("Queue Render", false);
Category SubmitDraws("Submit Draws", true);
Category ApplyLights("Apply Lights", true);
Category DrawEffects("Draw Effects", true);

Category RepeatingEvents("Repeating events", false);
Category NonrepeatingEvents("Nonrepeating events", false);

Category ParticlesRenderAll("Render particles", true);
Category ParticlesMoveAll("Move particles", false);

Category TrailDraw("Trail Draw", true);

Category EnvironmentMapping("Environment Mapping", true);
Category BuildShadowMap("Build Shadow Map", true);
Category RenderScene("Render scene", true);
Category RenderTrails("Render trails", true);
Category MoveObjects("Move Objects", false);
Category ProcessParticleEffects("Process particle effects", false);
Category TrailsMoveAll("Trails move all", false);
Category Simulation("Simulation", false);
Category RenderMainFrame("Render frame", true);
Category MainFrame("Main Frame", true);
Category PageFlip("Page flip", true);

Category CutsceneStep("Cutscene step", true);
Category CutsceneDrawVideoFrame("Draw cutscene frame", true);
Category CutsceneProcessDecoder("Process decoder data", false);
Category CutsceneProcessVideoData("Process video data", true);
Category CutsceneProcessAudioData("Process audio data", false);

Category CutsceneFFmpegVideoDecoder("FFmpeg decode video", false);
Category CutsceneFFmpegAudioDecoder("FFmpeg decode video", false);

}
