#pragma once
#include "../AbstractDialogModel.h"
namespace fso::fred::dialogs {
/**
 * @brief Model for QtFRED's Custom warp dialog
 */
class ShipCustomWarpDialogModel : public AbstractDialogModel {
  private:
	/**
	 * @brief Initialises data for the model
	 */
	void initializeData();
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
	/**
	 * @brief Marks the model as modifed
	 */

  public:
	/**
	 * @brief Constructor
	 * @param [in] parent The parent dialog.
	 * @param [in] viewport The viewport this dialog is attacted to.
	 * @param [in] departure Whether the dialog is changeing warp-in or warp-out.
	 */
	ShipCustomWarpDialogModel(QObject* parent, EditorViewport* viewport, bool departure);
	bool apply() override;
	void reject() override;

	// Getters
	/**
	 * @brief Getter
	 * @return Index of warp type
	 */
	int getType() const;
	/**
	 * @brief Getter
	 * @return Sound name
	 */
	SCP_string getStartSound() const;
	/**
	 * @brief Getter
	 * @return Sound name
	 */
	SCP_string getEndSound() const;
	/**
	 * @brief Getter
	 * @return Engage time in seconds
	 */
	float getEngageTime() const;
	/**
	 * @brief Getter
	 * @return ship speed
	 */
	float getSpeed() const;
	/**
	 * @brief Getter
	 * @return Time in seconds
	 */
	float getTime() const;
	/**
	 * @brief Getter
	 * @return Exponent
	 */
	float getExponent() const;
	/**
	 * @brief Getter
	 * @return Radius of effect
	 */
	float getRadius() const;
	/**
	 * @brief Getter
	 * @return anim name
	 */
	SCP_string getAnim() const;
	/**
	 * @brief Getter
	 * @return Supercap Physics
	 */
	bool getSupercap() const;
	/**
	 * @brief Getter
	 * @return Player Warpout Speed
	 */
	float getPlayerSpeed() const;
	/**
	 * @brief Getter
	 * @return If the model is in depart mode.
	 */
	bool departMode() const;
	/**
	 * @brief Getter
	 * @return If the model is working on a player.
	 */
	bool isPlayer() const;

	// Setters
	void setType(const int index);
	void setStartSound(const SCP_string&);
	void setEndSound(const SCP_string&);
	void setEngageTime(const double);
	void setSpeed(const double);
	void setTime(const double);
	void setExponent(const double);
	void setRadius(const double);
	void setAnim(const SCP_string&);
	void setSupercap(const bool);
	void setPlayerSpeed(const double);
};

} // namespace dialogs