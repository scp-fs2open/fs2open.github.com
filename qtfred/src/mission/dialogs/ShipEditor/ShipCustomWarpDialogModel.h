#pragma once

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {

class ShipCustomWarpDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	enum class Target {
		Selection,
		Wing
	};

	ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure);
	ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure, Target target, int wingIndex);
	bool apply() override;
	void reject() override;

	int getType() const;
	SCP_string getStartSound() const;
	SCP_string getEndSound() const;
	float getEngageTime() const;
	float getSpeed() const;
	float getTime() const;
	float getExponent() const;
	float getRadius() const;
	SCP_string getAnim() const;
	bool getSupercap() const;
	float getPlayerSpeed() const;
	bool departMode() const;
	bool isPlayer() const;

	void setType(int index);
	void setStartSound(const SCP_string& sound);
	void setEndSound(const SCP_string& sound);
	void setEngageTime(double engageTime);
	void setSpeed(double speed);
	void setTime(double time);
	void setExponent(double exponent);
	void setRadius(double radius);
	void setAnim(const SCP_string& anim);
	void setSupercap(bool supercap);
	void setPlayerSpeed(double playerSpeed);

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();

	bool _departure;
	int _warpType;
	SCP_string _startSound;
	SCP_string _endSound;
	float _warpoutEngageTime;
	float _speed;
	float _time;
	float _accelExp;
	float _radius;
	SCP_string _anim;
	bool _supercapWarpPhysics;
	float _playerWarpoutSpeed;
	bool _player = false;
	Target _target = Target::Selection;
	int _wingIndex = -1;
};

} // namespace fso::fred::dialogs
