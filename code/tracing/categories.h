
#ifndef _TRACING_CATEGORIES_H
#define _TRACING_CATEGORIES_H
#pragma once

#include "globalincs/pstypes.h"

/** @file
 *  @ingroup tracing
 *
 *  This file contains the tracing categories. In order to add a new category you must add the instance in categories.cpp,
 *  declare the @c extern reference here and then use it with the appropriate functions wherever you want to trace.
 */

namespace tracing {

class Category {
	const SCP_string _name;
	bool _graphics_category;
 public:
	Category(const char* name, bool is_graphics);

	const char* getName() const;

	bool usesGPUCounter() const;
};

extern Category LuaOnFrame;
extern Category LuaHooks;

extern Category DrawSceneTexture;
extern Category UpdateDistortion;

extern Category SceneTextureBegin;
extern Category SceneTextureEnd;
extern Category Tonemapping;
extern Category Bloom;
extern Category BloomBrightPass;
extern Category BloomIterationStep;
extern Category BloomCompositeStep;
extern Category FXAA;
extern Category SMAA;
extern Category SMAAEdgeDetection;
extern Category SMAACalculateBlendingWeights;
extern Category SMAANeighborhoodBlending;
extern Category SMAAResolve;
extern Category Lightshafts;
extern Category DrawPostEffects;

extern Category RenderBatchItem;
extern Category RenderBatchBuffer;
extern Category LoadBatchingBuffers;

extern Category SortColliders;
extern Category FindOverlapColliders;
extern Category CollidePair;

extern Category WeaponPostMove;
extern Category ShipPostMove;
extern Category FireballPostMove;
extern Category DebrisPostMove;
extern Category AsteroidPostMove;
extern Category PreMove;
extern Category Physics;
extern Category PostMove;
extern Category CollisionDetection;

extern Category RenderBuffer;

extern Category QueueRender;
extern Category BuildModelUniforms;
extern Category UploadModelUniforms;
extern Category SubmitDraws;
extern Category ApplyLights;
extern Category DrawEffects;
extern Category SetupNebula;
extern Category DrawPoofs;
extern Category DrawStars;
extern Category DrawShields;
extern Category DrawBeams;
extern Category DrawStarfield;
extern Category DrawMotionDebris;
extern Category DrawBackground;
extern Category DrawSuns;
extern Category DrawBitmaps;
extern Category SunspotProcess;

extern Category RepeatingEvents;
extern Category NonrepeatingEvents;

extern Category ParticlesRenderAll;
extern Category ParticlesMoveAll;

extern Category EnvironmentMapping;
extern Category BuildShadowMap;
extern Category RenderScene;
extern Category RenderTrails;
extern Category MoveObjects;
extern Category ProcessParticleEffects;
extern Category TrailsMoveAll;
extern Category Simulation;
extern Category RenderMainFrame;
extern Category RenderHUD;
extern Category RenderHUDHook;
extern Category RenderHUDGauge;
extern Category RenderTargetingBracket;
extern Category RenderNavBracket;
extern Category MainFrame;
extern Category PageFlip;

extern Category NanoVGFlushFrame;
extern Category NanoVGDrawFill;
extern Category NanoVGDrawConvexFill;
extern Category NanoVGDrawStroke;
extern Category NanoVGDrawTriangles;

extern Category LineDrawListFlush;

extern Category CutsceneStep;
extern Category CutsceneDrawVideoFrame;
extern Category CutsceneProcessDecoder;
extern Category CutsceneProcessVideoData;
extern Category CutsceneProcessAudioData;

extern Category CutsceneFFmpegVideoDecoder;
extern Category CutsceneFFmpegAudioDecoder;

extern Category RocketCompileGeometry;
extern Category RocketRenderCompiledGeometry;
extern Category RocketLoadTexture;
extern Category RocketGenerateTexture;
extern Category RocketRenderGeometry;

// Loading scopes
extern Category LoadMissionLoad;
extern Category LoadPostMissionLoad;
extern Category LoadModelFile;
extern Category ReadModelFile;
extern Category ModelCreateVertexBuffers;
extern Category ModelCreateOctants;
extern Category ModelParseAllBSPTrees;
extern Category ModelParseBSPTree;
extern Category ModelConfigureVertexBuffers;
extern Category ModelCreateTransparencyIndexBuffer;
extern Category ModelCreateDetailIndexBuffers;

extern Category PreloadMissionSounds;
extern Category LoadSound;

extern Category LevelPageIn;
extern Category PageInStop;
extern Category PageInSingleBitmap;
extern Category ShipPageIn;
extern Category WeaponPageIn;

extern Category RenderDecals;
extern Category RenderSingleDecal;

extern Category GpuHeapAllocate;
extern Category GpuHeapDeallocate;

extern Category ProgramStepOne;

}

#endif // _TRACING_CATEGORIES_H
