#pragma once

#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {

enum class GeneratorAxis { XZ, XY, ZY };

class WaypointPathGeneratorDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	explicit WaypointPathGeneratorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	const SCP_string& getPathName() const;
	void setPathName(const SCP_string& v);

	bool getUseObjectCenter() const;
	void setUseObjectCenter(bool v);

	int getCenterObjectObjnum() const;
	void setCenterObjectObjnum(int objnum);

	float getCenterX() const;
	void setCenterX(float v);
	float getCenterY() const;
	void setCenterY(float v);
	float getCenterZ() const;
	void setCenterZ(float v);

	GeneratorAxis getAxis() const;
	void setAxis(GeneratorAxis v);

	int getNumPoints() const;
	void setNumPoints(int v);

	int getLoops() const;
	void setLoops(int v);

	float getRadius() const;
	void setRadius(float v);

	float getDrift() const;
	void setDrift(float v);

	float getVarianceX() const;
	void setVarianceX(float v);
	float getVarianceY() const;
	void setVarianceY(float v);
	float getVarianceZ() const;
	void setVarianceZ(float v);

	const SCP_vector<std::pair<SCP_string, int>>& getSceneObjects() const;

  private:
	bool validateData();
	void showErrorDialog(const SCP_string& message);

	SCP_string _pathName;
	bool _useObjectCenter = false;
	int _centerObjectObjnum = -1;
	float _centerX = 0.0f;
	float _centerY = 0.0f;
	float _centerZ = 0.0f;
	GeneratorAxis _axis = GeneratorAxis::XZ;
	int _numPoints = 8;
	int _loops = 1;
	float _radius = 1000.0f;
	float _drift = 0.0f;
	float _varianceX = 0.0f;
	float _varianceY = 0.0f;
	float _varianceZ = 0.0f;

	SCP_vector<std::pair<SCP_string, int>> _sceneObjects; // (display name, objnum)

	bool _bypass_errors = false;
};

} // namespace fso::fred::dialogs
