#pragma once

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {

class ShipSpecialStatsDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	ShipSpecialStatsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	bool getSpecialExp() const;
	void setSpecialExp(bool specialExp);
	bool getShockwave() const;
	void setShockwave(bool shockwave);
	bool getDeathRoll() const;
	void setDeathRoll(bool deathRoll);
	int getDamage() const;
	void setDamage(int damage);
	int getBlast() const;
	void setBlast(int blast);
	int getInnerRadius() const;
	void setInnerRadius(int innerRadius);
	int getOuterRadius() const;
	void setOuterRadius(int outerRadius);
	int getShockwaveSpeed() const;
	void setShockwaveSpeed(int speed);
	int getRollDuration() const;
	void setRollDuration(int duration);

	bool getSpecialShield() const;
	void setSpecialShield(bool specialShield);
	int getShield() const;
	void setShield(int shield);
	bool getSpecialHull() const;
	void setSpecialHull(bool specialHull);
	int getHull() const;
	void setHull(int hull);

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();

	int _ship;
	int _numSelectedShips;
	SCP_vector<int> _selectedShips;

	bool _specialExp;
	bool _shockwave;
	bool _deathRoll;
	int _innerRad;
	int _outerRad;
	int _damage;
	int _shockSpeed;
	int _duration;
	int _blast;

	bool _specialHitpointsEnabled;
	bool _specialShieldEnabled;
	int _shields;
	int _hull;
};

} // namespace fso::fred::dialogs
