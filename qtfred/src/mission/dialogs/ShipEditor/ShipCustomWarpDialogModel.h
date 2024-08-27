#pragma once
#include "../AbstractDialogModel.h"
namespace fso {
namespace fred {
namespace dialogs {
class ShipCustomWarpDialogModel : public AbstractDialogModel {
  private:
	void initializeData();
	template <typename T>
	void modify(T& a, const T& b);
	bool _modified = false;
	bool _m_departure;

	int _m_warp_type;
	SCP_string _m_start_sound;
	SCP_string _m_end_sound;
	float _m_warpout_engage_time;
	float _m_speed;
	float _m_time;
	float _m_accel_exp;
	float _m_radius;
	SCP_string _m_anim;
	bool _m_supercap_warp_physics;
	float _m_player_warpout_speed;

	bool _m_player = false;
	void set_modified();

  public:
	ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure);
	bool apply() override;
	void reject() override;

	//Getters
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

	bool query_modified() const;

	//Setters
	void setType(const int index);
	void setStartSound(const SCP_string);
	void setEndSound(const SCP_string);
	void setEngageTime(const double);
	void setSpeed(const double);
	void setTime(const double);
	void setExponent(const double);
	void setRadius(const double);
	void setAnim(const SCP_string&);
	void setSupercap(const bool);
	void setPlayerSpeed(const double);
};

template <typename T>
inline void ShipCustomWarpDialogModel::modify(T& a, const T& b)
{
	if (a != b) {
		a = b;
		set_modified();
		modelChanged();
	}
}

} // namespace dialogs
} // namespace fred
} // namespace fso