#pragma once
#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {

struct ObjectPosition {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct ObjectOrientation {
	float p = 0.0f;
	float b = 0.0f;
	float h = 0.0f;
};

class ObjectOrientEditorDialogModel : public AbstractDialogModel {
  public:
	ObjectOrientEditorDialogModel(QObject* parent, EditorViewport* viewport);

	struct ObjectEntry {
		SCP_string name;
		int objIndex = -1;
		ObjectEntry(SCP_string name, int objIndex);
	};

	enum class PointToMode {
		Object,
		Location
	};
	enum class SetMode {
		Absolute,
		Relative
	};
	enum class TransformMode {
		Independent,
		Relative
	};

	bool apply() override;
	void reject() override;

	bool isOrientationEnabledForType() const {return _orientationEnabledForType;};
	const SCP_vector<ObjectEntry>& getPointToObjectList() const {return _pointToObjectList;};
	int getNumObjectsMarked() const {return _editor->getNumMarked();}

	// Position
	void setPositionX(float x);
	void setPositionY(float y);
	void setPositionZ(float z);
	ObjectPosition getPosition() const;

	// Orientation
	void setOrientationP(float deg);
	void setOrientationB(float deg);
	void setOrientationH(float deg);
	ObjectOrientation getOrientation() const;

	// Settings
	void setSetMode(SetMode mode);
	SetMode getSetMode() const;
	void setTransformMode(TransformMode mode);
	TransformMode getTransformMode() const;

	// Point to
	void setPointTo(bool point_to);
	bool getPointTo() const;
	void setPointMode(PointToMode pointMode);
	PointToMode getPointMode() const;
	void setPointToObjectIndex(int selectedObjectNum);
	int getPointToObjectIndex() const;
	void setLocationX(float x);
	void setLocationY(float y);
	void setLocationZ(float z);
	ObjectPosition getLocation() const;

  private:
	void initializeData();
	void updateObject(object* ptr);

	int _selectedPointToObjectIndex = -1;
	bool _pointTo = false;

	vec3d _position;       // UI fields: X/Y/Z
	vec3d _orientationDeg; // UI fields: Pitch/Bank/Heading in degrees
	vec3d _location;       // Point to location X/Y/Z

	vec3d _rebaseRefPos = vmd_zero_vector;
	vec3d _rebaseRefAnglesDeg = vmd_zero_vector;

	bool _orientationEnabledForType = true;

	SCP_vector<ObjectEntry> _pointToObjectList;

	PointToMode _pointMode = PointToMode::Object;
	SetMode _setMode = SetMode::Absolute;
	TransformMode _transformMode = TransformMode::Independent;

	// Helpers
	static constexpr float INPUT_THRESHOLD = 0.01f; // Same as FRED in orienteditor.cpp TODO would be nice if this was stored somewhere common
	static float normalize_degrees(float deg);
	static bool is_close(float a, float b)
	{
		return fabsf(a - b) < INPUT_THRESHOLD;
	}

	static float round1(float v);
};

} // namespace fso::fred::dialogs