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
	std::pair<float, float> getScaleLimit() const { return {0.001f, 18.0f}; }
	std::pair<int, int> getDivisionLimit() const { return {1, 5}; }

	// bitmap group box
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

  private slots:
	void onEditorSelectionChanged(int); // currentObjectChanged
	void onEditorMissionChanged();      // missionChanged

  private: // NOLINT(readability-redundant-access-specifiers)
	void refreshBackgroundPreview();
	background_t& getActiveBackground() const;
	starfield_list_entry* getActiveBitmap() const;

	int _selectedBitmapIndex = -1; // index into Backgrounds[Cur_background].bitmaps

};
} // namespace fso::fred::dialogs