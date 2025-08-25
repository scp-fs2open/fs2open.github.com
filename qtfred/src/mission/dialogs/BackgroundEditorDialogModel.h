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
	std::pair<int, int> getOrientLimit() const { return {0, 359}; }
	std::pair<float,float> getBitmapScaleLimit() const   { return {0.001f, 18.0f}; }
	std::pair<float,float> getSunScaleLimit() const{ return {0.1f,   50.0f}; }
	std::pair<int, int> getDivisionLimit() const { return {1, 5}; }

	// bitmap group
	SCP_vector<SCP_string> getAvailableBitmapNames() const;
	SCP_vector<SCP_string> getMissionBitmapNames() const;
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
	SCP_vector<SCP_string> getAvailableSunNames() const;
	SCP_vector<SCP_string> getMissionSunNames() const;
	void setSelectedSunIndex(int index);
	int getSelectedSunIndex() const;
	void addMissionSunByName(const SCP_string& name);
	void removeMissionSun();
	SCP_string getSunName() const;
	void setSunName(const SCP_string& name);
	int getSunPitch() const;
	void setSunPitch(int deg);
	int getSunBank() const; // unused for suns but added for consistency
	void setSunBank(int deg); // unused for suns but added for consistency
	int getSunHeading() const;
	void setSunHeading(int deg);
	float getSunScale() const; // uses scale_x for both x and y
	void setSunScale(float v);

	// nebula group
	SCP_vector<SCP_string> getLightningNames() const;
	SCP_vector<SCP_string> getNebulaPatternNames() const;
	SCP_vector<SCP_string> getPoofNames() const;
	bool getFullNebulaEnabled() const;
	void setFullNebulaEnabled(bool enabled);
	float getFullNebulaRange() const;
	void setFullNebulaRange(float range);
	SCP_string getNebulaFullPattern() const;
	void setNebulaFullPattern(const SCP_string& name);
	SCP_string getLightning() const;
	void setLightning(const SCP_string& name);
	SCP_vector<SCP_string> getSelectedPoofs() const;
	void setSelectedPoofs(const SCP_vector<SCP_string>& names);
	bool getShipTrailsToggled() const;
	void setShipTrailsToggled(bool on);
	float getFogNearMultiplier() const;
	void setFogNearMultiplier(float v);
	float getFogFarMultiplier() const;
	void setFogFarMultiplier(float v);
	bool getDisplayBackgroundBitmaps() const;
	void setDisplayBackgroundBitmaps(bool on);
	bool getFogPaletteOverride() const;
	void setFogPaletteOverride(bool on);
	int getFogR() const;
	void setFogR(int r);
	int getFogG() const;
	void setFogG(int g);
	int getFogB() const;
	void setFogB(int b);

	// old nebula group
	SCP_vector<SCP_string> getOldNebulaPatternOptions() const;
	SCP_vector<SCP_string> getOldNebulaColorOptions() const;
	SCP_string getOldNebulaColorName() const;
	void setOldNebulaColorName(const SCP_string& name);
	SCP_string getOldNebulaPattern() const;
	void setOldNebulaPattern(const SCP_string& name);
	int getOldNebulaPitch() const;
	void setOldNebulaPitch(int deg);
	int getOldNebulaBank() const;
	void setOldNebulaBank(int deg);
	int getOldNebulaHeading() const;
	void setOldNebulaHeading(int deg);

	// ambient light group
	int getAmbientR() const;
	void setAmbientR(int r);
	int getAmbientG() const;
	void setAmbientG(int g);
	int getAmbientB() const;
	void setAmbientB(int b);

  private:
	void refreshBackgroundPreview();
	background_t& getActiveBackground() const;
	starfield_list_entry* getActiveBitmap() const;
	starfield_list_entry* getActiveSun() const;

	int _selectedBitmapIndex = -1; // index into Backgrounds[Cur_background].bitmaps
	int _selectedSunIndex = -1;    // index into Backgrounds[Cur_background].suns

};
} // namespace fso::fred::dialogs