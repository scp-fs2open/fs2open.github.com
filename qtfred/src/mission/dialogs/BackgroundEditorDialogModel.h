#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

#include "starfield/starfield.h"
#include <globalincs/linklist.h>

namespace fso::fred::dialogs {

/**
 * @brief QTFred's Wing Editor's Model
 */
class BackgroundEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT

  public:
	BackgroundEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	// limits
	static std::pair<int, int> getOrientLimit() { return {0, 359}; }
	static std::pair<float,float> getBitmapScaleLimit() { return {0.001f, 18.0f}; }
	static std::pair<float,float> getSunScaleLimit() { return {0.1f,   50.0f}; }
	static std::pair<int, int> getDivisionLimit() { return {1, 5}; }
	static std::pair<int, int> getStarsLimit() { return {0, MAX_STARS}; }

	// backgrounds group
	static SCP_vector<SCP_string> getBackgroundNames();
	void setActiveBackgroundIndex(int idx); 
	static int getActiveBackgroundIndex();
	void addBackground();
	void removeActiveBackground();
	static int getImportableBackgroundCount(const SCP_string& fs2Path);
	bool importBackgroundFromMission(const SCP_string& fs2Path, int whichIndex);
	void swapBackgrounds();
	void setSwapWithIndex(int idx);
	int getSwapWithIndex() const;
	void setSaveAnglesCorrectFlag(bool on);
	static bool getSaveAnglesCorrectFlag();

	// bitmap group
	static SCP_vector<SCP_string> getAvailableBitmapNames();
	static SCP_vector<SCP_string> getMissionBitmapNames();
	void setSelectedBitmapIndex(int index);
	int getSelectedBitmapIndex() const;
	void addMissionBitmapByName(const SCP_string& name);
	void removeMissionBitmap();
	SCP_string getBitmapName() const;
	void setBitmapName(const SCP_string& name);
	int getBitmapPitch() const;
	void setBitmapPitch(int deg);
	int getBitmapBank() const;
	void setBitmapBank(int deg);
	int getBitmapHeading() const;
	void setBitmapHeading(int deg);
	float getBitmapScaleX() const;
	void setBitmapScaleX(float v);
	float getBitmapScaleY() const;
	void setBitmapScaleY(float v);
	int getBitmapDivX() const;
	void setBitmapDivX(int v);
	int getBitmapDivY() const;
	void setBitmapDivY(int v);

	// sun group
	static SCP_vector<SCP_string> getAvailableSunNames();
	static SCP_vector<SCP_string> getMissionSunNames();
	void setSelectedSunIndex(int index);
	int getSelectedSunIndex() const;
	void addMissionSunByName(const SCP_string& name);
	void removeMissionSun();
	SCP_string getSunName() const;
	void setSunName(const SCP_string& name);
	int getSunPitch() const;
	void setSunPitch(int deg);
	int getSunHeading() const;
	void setSunHeading(int deg);
	float getSunScale() const; // uses scale_x for both x and y
	void setSunScale(float v);

	// nebula group
	static SCP_vector<SCP_string> getLightningNames();
	static SCP_vector<SCP_string> getNebulaPatternNames();
	static SCP_vector<SCP_string> getPoofNames();
	static bool getFullNebulaEnabled();
	void setFullNebulaEnabled(bool enabled);
	static float getFullNebulaRange();
	void setFullNebulaRange(float range);
	static SCP_string getNebulaFullPattern();
	void setNebulaFullPattern(const SCP_string& name);
	static SCP_string getLightning();
	void setLightning(const SCP_string& name);
	static SCP_vector<SCP_string> getSelectedPoofs();
	void setSelectedPoofs(const SCP_vector<SCP_string>& names);
	static bool getShipTrailsToggled();
	void setShipTrailsToggled(bool on);
	static float getFogNearMultiplier();
	void setFogNearMultiplier(float v);
	static float getFogFarMultiplier();
	void setFogFarMultiplier(float v);
	static bool getDisplayBackgroundBitmaps();
	void setDisplayBackgroundBitmaps(bool on);
	static bool getFogPaletteOverride();
	void setFogPaletteOverride(bool on);
	static int getFogR();
	void setFogR(int r);
	static int getFogG();
	void setFogG(int g);
	static int getFogB();
	void setFogB(int b);

	// old nebula group
	static SCP_vector<SCP_string> getOldNebulaPatternOptions();
	static SCP_vector<SCP_string> getOldNebulaColorOptions();
	static SCP_string getOldNebulaColorName();
	void setOldNebulaColorName(const SCP_string& name);
	static SCP_string getOldNebulaPattern();
	void setOldNebulaPattern(const SCP_string& name);
	static int getOldNebulaPitch();
	void setOldNebulaPitch(int deg);
	static int getOldNebulaBank();
	void setOldNebulaBank(int deg);
	static int getOldNebulaHeading();
	void setOldNebulaHeading(int deg);

	// ambient light group
	static int getAmbientR();
	void setAmbientR(int r);
	static int getAmbientG();
	void setAmbientG(int g);
	static int getAmbientB();
	void setAmbientB(int b);

	// skybox group
	static SCP_string getSkyboxModelName();
	void setSkyboxModelName(const SCP_string& name);
	static bool getSkyboxNoLighting();
	void setSkyboxNoLighting(bool on);
	static bool getSkyboxAllTransparent();
	void setSkyboxAllTransparent(bool on);
	static bool getSkyboxNoZbuffer();
	void setSkyboxNoZbuffer(bool on);
	static bool getSkyboxNoCull();
	void setSkyboxNoCull(bool on);
	static bool getSkyboxNoGlowmaps();
	void setSkyboxNoGlowmaps(bool on);
	static bool getSkyboxForceClamp();
	void setSkyboxForceClamp(bool on);
	static int getSkyboxPitch();
	void setSkyboxPitch(int deg);
	static int getSkyboxBank();
	void setSkyboxBank(int deg);
	static int getSkyboxHeading();
	void setSkyboxHeading(int deg);

	// misc group
	static SCP_vector<SCP_string> getLightingProfileOptions();
	static int getNumStars();
	void setNumStars(int n);
	static bool getTakesPlaceInSubspace();
	void setTakesPlaceInSubspace(bool on);
	static SCP_string getEnvironmentMapName();
	void setEnvironmentMapName(const SCP_string& name);
	static SCP_string getLightingProfileName();
	void setLightingProfileName(const SCP_string& name);

  private:
	void refreshBackgroundPreview();
	static background_t& getActiveBackground();
	starfield_list_entry* getActiveBitmap() const;
	starfield_list_entry* getActiveSun() const;

	int _selectedBitmapIndex = -1; // index into Backgrounds[Cur_background].bitmaps
	int _selectedSunIndex = -1;    // index into Backgrounds[Cur_background].suns
	int _swapIndex = 0;            // index of background to swap with

};
} // namespace fso::fred::dialogs