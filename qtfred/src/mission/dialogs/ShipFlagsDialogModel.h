#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class ShipFlagsDialogModel : public AbstractDialogModel {
  private:

	template <typename T>
	void modify(T& a, const T& b);

	bool _modified = false;

	int m_red_alert_carry;
	int m_scannable;
	int m_reinforcement;
	int m_protect_ship;
	int m_beam_protect_ship;
	int m_flak_protect_ship;
	int m_laser_protect_ship;
	int m_missile_protect_ship;
	int m_no_dynamic;
	int m_no_arrival_music;
	int m_kamikaze;
	int m_invulnerable;
	int m_targetable_as_bomb;
	int m_immobile;
	int m_ignore_count;
	int m_hidden;
	int m_primitive_sensors;
	int m_no_subspace_drive;
	int m_affected_by_gravity;
	int m_toggle_subsystem_scanning;
	int m_escort;
	int m_destroy;
	int m_cargo_known;
	int m_special_warpin;
	int m_disable_messages;
	int m_no_death_scream;
	int m_always_death_scream;
	int m_guardian;
	int m_vaporize;
	int m_stealth;
	int m_friendly_stealth_invisible;
	int m_nav_carry;
	int m_nav_needslink;
	int m_hide_ship_name;
	int m_disable_ets;
	int m_cloaked;
	int m_set_class_dynamically;
	int m_scramble_messages;
	int m_no_collide;
	int m_no_disabled_self_destruct;

	int m_kdamage;
	int m_destroy_value;
	int m_escort_value;
	int m_respawn_priority;

	void set_modified();
	static int tristate_set(const int val, const int cur_state);
	void update_ship(const int);

  public:
	ShipFlagsDialogModel(QObject* parent, EditorViewport* viewport);
	void initializeData();

	bool apply() override;
	void reject() override;

	void setDestroyed(const int);
	int getDestroyed() const;

	void setDestroyedSeconds(const int);
	int getDestroyedSeconds() const;

	void setScannable(const int);
	int getScannable() const;

	void setCargoKnown(const int);
	int getCargoKnown() const;

	void setSubsystemScanning(const int);
	int getSubsystemScanning() const;

	void setReinforcment(const int);
	int getReinforcment() const;

	void setProtectShip(const int);
	int getProtectShip() const;

	void setBeamProtect(const int);
	int getBeamProtect() const;

	void setFlakProtect(const int);
	int getFlakProtect() const;

	void setLaserProtect(const int);
	int getLaserProtect() const;

	void setMissileProtect(const int);
	int getMissileProtect() const;

	void setIgnoreForGoals(const int);
	int getIgnoreForGoals() const;

	void setEscort(const int);
	int getEscort() const;
	void setEscortValue(const int);
	int getEscortValue() const;

	void setNoArrivalMusic(const int);
	int getNoArrivalMusic() const;

	void setInvulnerable(const int);
	int getInvulnerable() const;

	void setGuardianed(const int);
	int getGuardianed() const;

	void setPrimitiveSensors(const int);
	int getPrimitiveSensors() const;

	void setNoSubspaceDrive(const int);
	int getNoSubspaceDrive() const;

	void setHidden(const int);
	int getHidden() const;

	void setStealth(const int);
	int getStealth() const;

	void setFriendlyStealth(const int);
	int getFriendlyStealth() const;

	void setKamikaze(const int);
	int getKamikaze() const;
	void setKamikazeDamage(const int);
	int getKamikazeDamage() const;

	void setImmobile(const int);
	int getImmobile() const;

	void setNoDynamicGoals(const int);
	int getNoDynamicGoals() const;

	void setRedAlert(const int);
	int getRedAlert() const;

	void setGravity(const int);
	int getGravity() const;

	void setWarpin(const int);
	int getWarpin() const;

	void setTargetableAsBomb(const int);
	int getTargetableAsBomb() const;

	void setDisableBuiltInMessages(const int);
	int getDisableBuiltInMessages() const;

	void setNeverScream(const int);
	int getNeverScream() const;

	void setAlwaysScream(const int);
	int getAlwaysScream() const;

	void setVaporize(const int);
	int getVaporize() const;

	void setRespawnPriority(const int);
	int getRespawnPriority() const;

	void setAutoCarry(const int);
	int getAutoCarry() const;

	void setAutoLink(const int);
	int getAutoLink() const;

	void setHideShipName(const int);
	int getHideShipName() const;

	void setClassDynamic(const int);
	int getClassDynamic() const;

	void setDisableETS(const int);
	int getDisableETS() const;

	void setCloak(const int);
	int getCloak() const;

	void setScrambleMessages(const int);
	int getScrambleMessages() const;

	void setNoCollide(const int);
	int getNoCollide() const;

	void setNoSelfDestruct(const int);
	int getNoSelfDestruct() const;

	bool query_modified();
};

template <typename T>
inline void ShipFlagsDialogModel::modify(T& a, const T& b)
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