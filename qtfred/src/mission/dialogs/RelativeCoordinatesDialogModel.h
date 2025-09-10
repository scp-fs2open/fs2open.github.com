#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class RelativeCoordinatesDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	RelativeCoordinatesDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	float getDistance() const;
	float getPitch() const;
	float getBank() const;
	float getHeading() const;

	int getOrigin() const;
	void setOrigin(int index);
	int getSatellite() const;
	void setSatellite(int index);

	SCP_vector<std::pair<SCP_string, int>> getObjectsList() const;

  private:
	void computeCoordinates();
	static float to_degrees(float rad);
	static float normalize_degrees(float deg);

	int _originIndex = -1;
	int _satelliteIndex = -1;

	float _distance = 0.0f;
	float _orientation_p = 0.0f;
	float _orientation_b = 0.0f;
	float _orientation_h = 0.0f;

	SCP_vector<std::pair<SCP_string, int>> _objects; // (name, obj index)
	
};

} // namespace fred::dialogs