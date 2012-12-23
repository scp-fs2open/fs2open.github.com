/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// ShipFlagsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FRED.h"
#include "ShipFlagsDlg.h"
#include "globalincs/linklist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ship_flags_dlg dialog

ship_flags_dlg::ship_flags_dlg(CWnd* pParent /*=NULL*/)
	: CDialog(ship_flags_dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ship_flags_dlg)
	//}}AFX_DATA_INIT
}

void ship_flags_dlg::DoDataExchange(CDataExchange* pDX)
{
	int n;
	CString str;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ship_flags_dlg)
	DDX_Control(pDX, IDC_TOGGLE_SUBSYSTEM_SCANNING, m_toggle_subsystem_scanning);
	DDX_Control(pDX, IDC_REDALERTCARRY, m_red_alert_carry);
	DDX_Control(pDX, IDC_SCANNABLE, m_scannable);
	DDX_Control(pDX, IDC_REINFORCEMENT, m_reinforcement);
	DDX_Control(pDX, IDC_PROTECT_SHIP, m_protect_ship);
	DDX_Control(pDX, IDC_BEAM_PROTECT_SHIP, m_beam_protect_ship);
	DDX_Control(pDX, IDC_FLAK_PROTECT_SHIP, m_flak_protect_ship);
	DDX_Control(pDX, IDC_LASER_PROTECT_SHIP, m_laser_protect_ship);
	DDX_Control(pDX, IDC_MISSILE_PROTECT_SHIP, m_missile_protect_ship);
	DDX_Control(pDX, IDC_NO_DYNAMIC, m_no_dynamic);
	DDX_Control(pDX, IDC_NO_ARRIVAL_MUSIC, m_no_arrival_music);
	DDX_Control(pDX, IDC_KAMIKAZE, m_kamikaze);
	DDX_Control(pDX, IDC_INVULNERABLE, m_invulnerable);
	DDX_Control(pDX, IDC_TARGETABLE_AS_BOMB, m_targetable_as_bomb);
	DDX_Control(pDX, IDC_IMMOBILE, m_immobile);
	DDX_Control(pDX, IDC_IGNORE_COUNT, m_ignore_count);
	DDX_Control(pDX, IDC_HIDDEN_FROM_SENSORS, m_hidden);
	DDX_Control(pDX, IDC_PRIMITIVE_SENSORS, m_primitive_sensors);
	DDX_Control(pDX, IDC_NO_SUBSPACE_DRIVE, m_no_subspace_drive);
	DDX_Control(pDX, IDC_AFFECTED_BY_GRAVITY, m_affected_by_gravity);
	DDX_Control(pDX, IDC_ESCORT, m_escort);
	DDX_Control(pDX, IDC_DESTROY_CHECK, m_destroy);
	DDX_Control(pDX, IDC_CARGO_KNOWN, m_cargo_known);
	DDX_Control(pDX, IDC_SPECIAL_WARPIN, m_special_warpin);	
	DDX_Control(pDX, IDC_DESTROY_SPIN, m_destroy_spin);	
	DDX_Control(pDX, IDC_DISABLE_BUILTIN_SHIP, m_disable_messages);
	DDX_Control(pDX, IDC_NO_DEATH_SCREAM, m_no_death_scream);
	DDX_Control(pDX, IDC_ALWAYS_DEATH_SCREAM, m_always_death_scream);
	DDX_Control(pDX, IDC_GUARDIAN, m_guardian);
	DDX_Control(pDX, IDC_VAPORIZE, m_vaporize);
	DDX_Control(pDX, IDC_STEALTH, m_stealth);
	DDX_Control(pDX, IDC_FRIENDLY_STEALTH_INVISIBLE, m_friendly_stealth_invisible);
	DDX_Control(pDX, IDC_NAV_CARRY, m_nav_carry);
	DDX_Control(pDX, IDC_NAV_NEEDSLINK, m_nav_needslink);
	DDX_Control(pDX, IDC_HIDE_SHIP_NAME, m_hide_ship_name);
	DDX_Control(pDX, IDC_DISABLE_ETS, m_disable_ets);
	DDX_Control(pDX, IDC_CLOAKED, m_cloaked);
	DDX_Control(pDX, IDC_SET_CLASS_DYNAMICALLY, m_set_class_dynamically);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {  // get dialog control values
		GetDlgItem(IDC_DESTROY_VALUE)->GetWindowText(str);
		n = atoi(str);
		if (n < 0)
			n = 0;

		m_destroy_value.init(n);

		GetDlgItem(IDC_KDAMAGE)->GetWindowText(str);
		m_kdamage.init(atoi(str));

		//  get escort priority
		GetDlgItem(IDC_ESCORT_PRIORITY)->GetWindowText(str);
		m_escort_value.init(atoi(str));

		// get respawn priority
		if(The_mission.game_type & MISSION_TYPE_MULTI) {
			GetDlgItem(IDC_RESPAWN_PRIORITY)->GetWindowText(str);
			m_respawn_priority.init(atoi(str));
		}
	}
}

BEGIN_MESSAGE_MAP(ship_flags_dlg, CDialog)
	//{{AFX_MSG_MAP(ship_flags_dlg)
	ON_BN_CLICKED(IDC_CARGO_KNOWN, OnCargoKnown)
	ON_BN_CLICKED(IDC_DESTROY_CHECK, OnDestroyCheck)
	ON_BN_CLICKED(IDC_ESCORT, OnEscort)
	ON_BN_CLICKED(IDC_HIDDEN_FROM_SENSORS, OnHiddenFromSensors)
	ON_BN_CLICKED(IDC_PRIMITIVE_SENSORS, OnPrimitiveSensors)
	ON_BN_CLICKED(IDC_NO_SUBSPACE_DRIVE, OnNoSubspaceDrive)
	ON_BN_CLICKED(IDC_AFFECTED_BY_GRAVITY, OnAffectedByGravity)
	ON_BN_CLICKED(IDC_IGNORE_COUNT, OnIgnoreCount)
	ON_BN_CLICKED(IDC_INVULNERABLE, OnInvulnerable)
	ON_BN_CLICKED(IDC_TARGETABLE_AS_BOMB, OnTargetableAsBomb)
	ON_BN_CLICKED(IDC_IMMOBILE, OnImmobile)
	ON_BN_CLICKED(IDC_KAMIKAZE, OnKamikaze)
	ON_BN_CLICKED(IDC_NO_ARRIVAL_MUSIC, OnNoArrivalMusic)
	ON_BN_CLICKED(IDC_NO_DYNAMIC, OnNoDynamic)
	ON_BN_CLICKED(IDC_PROTECT_SHIP, OnProtectShip)
	ON_BN_CLICKED(IDC_BEAM_PROTECT_SHIP, OnBeamProtectShip)
	ON_BN_CLICKED(IDC_FLAK_PROTECT_SHIP, OnFlakProtectShip)
	ON_BN_CLICKED(IDC_LASER_PROTECT_SHIP, OnLaserProtectShip)
	ON_BN_CLICKED(IDC_MISSILE_PROTECT_SHIP, OnMissileProtectShip)
	ON_BN_CLICKED(IDC_REINFORCEMENT, OnReinforcement)
	ON_BN_CLICKED(IDC_SCANNABLE, OnScannable)
	ON_BN_CLICKED(IDC_REDALERTCARRY, OnRedalertcarry)
	ON_BN_CLICKED(IDC_TOGGLE_SUBSYSTEM_SCANNING, OnToggleSubsystemScanning)
	ON_BN_CLICKED(IDC_DISABLE_BUILTIN_SHIP, OnDisableBuiltinShip)
	ON_BN_CLICKED(IDC_NO_DEATH_SCREAM, OnNoDeathScream)
	ON_BN_CLICKED(IDC_ALWAYS_DEATH_SCREAM, OnAlwaysDeathScream)
	ON_BN_CLICKED(IDC_GUARDIAN, OnGuardian)
	ON_BN_CLICKED(IDC_VAPORIZE, OnVaporize)
	ON_BN_CLICKED(IDC_STEALTH, OnStealth)
	ON_BN_CLICKED(IDC_FRIENDLY_STEALTH_INVISIBLE, OnFriendlyStealthInvisible)
	ON_BN_CLICKED(IDC_NAV_CARRY, OnNavCarry)
	ON_BN_CLICKED(IDC_NAV_NEEDSLINK, OnNavNeedslink)
	ON_BN_CLICKED(IDC_HIDE_SHIP_NAME, OnHideShipName)
	ON_BN_CLICKED(IDC_SET_CLASS_DYNAMICALLY, OnSetClassDynamically)
	ON_BN_CLICKED(IDC_DISABLE_ETS, OnDisableETS)
	ON_BN_CLICKED(IDC_CLOAKED, OnCloaked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ship_flags_dlg message handlers

void ship_flags_dlg::setup(int n)
{
	p_enable = n;
}

BOOL ship_flags_dlg::OnInitDialog() 
{
	int j, first;
	int protect_ship = 0, beam_protect_ship = 0, flak_protect_ship = 0, laser_protect_ship = 0, missile_protect_ship = 0;
	int ignore_count = 0, reinforcement = 0, cargo_known = 0, immobile = 0;
	int destroy_before_mission = 0, no_arrival_music = 0, escort = 0, invulnerable = 0, targetable_as_bomb = 0;
	int hidden_from_sensors = 0, primitive_sensors = 0, no_subspace_drive = 0, affected_by_gravity = 0;
	int toggle_subsystem_scanning = 0, scannable = 0, kamikaze = 0, no_dynamic = 0, red_alert_carry = 0;
	int special_warpin = 0, disable_messages = 0, guardian = 0, vaporize = 0, stealth = 0, friendly_stealth_invisible = 0;
	int no_death_scream = 0, always_death_scream = 0;
	int nav_carry = 0, nav_needslink = 0, hide_ship_name = 0, set_class_dynamically = 0, no_ets = 0, cloaked = 0;

	object *objp;
	ship *shipp;
	bool ship_in_wing = false;

	first = 1;
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags & OF_MARKED) {
				shipp = &Ships[objp->instance];

				if (first) {
					first = 0;
					scannable = (shipp->flags & SF_SCANNABLE) ? 1 : 0;
					red_alert_carry = (shipp->flags & SF_RED_ALERT_STORE_STATUS) ? 1 : 0;
					special_warpin = (objp->flags & OF_SPECIAL_WARPIN) ? 1 : 0;
					protect_ship = (objp->flags & OF_PROTECTED) ? 1 : 0;
					beam_protect_ship = (objp->flags & OF_BEAM_PROTECTED) ? 1 : 0;
					flak_protect_ship = (objp->flags & OF_FLAK_PROTECTED) ? 1 : 0;
					laser_protect_ship = (objp->flags & OF_LASER_PROTECTED) ? 1 : 0;
					missile_protect_ship = (objp->flags & OF_MISSILE_PROTECTED) ? 1 : 0;
					invulnerable = (objp->flags & OF_INVULNERABLE) ? 1 : 0;
					targetable_as_bomb = (objp->flags & OF_TARGETABLE_AS_BOMB) ? 1 : 0;
					immobile = (objp->flags & OF_IMMOBILE) ? 1 : 0;
					hidden_from_sensors = (shipp->flags & SF_HIDDEN_FROM_SENSORS) ? 1 : 0;
					primitive_sensors = (shipp->flags2 & SF2_PRIMITIVE_SENSORS) ? 1 : 0;
					no_subspace_drive = (shipp->flags2 & SF2_NO_SUBSPACE_DRIVE) ? 1 : 0;
					affected_by_gravity = (shipp->flags2 & SF2_AFFECTED_BY_GRAVITY) ? 1 : 0;
					toggle_subsystem_scanning = (shipp->flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING) ? 1 : 0;
					ignore_count = (shipp->flags & SF_IGNORE_COUNT) ? 1 : 0;
					no_arrival_music = (shipp->flags & SF_NO_ARRIVAL_MUSIC) ? 1 : 0;
					cargo_known = (shipp->flags & SF_CARGO_REVEALED) ? 1 : 0;
					no_dynamic = (Ai_info[shipp->ai_index].ai_flags & AIF_NO_DYNAMIC) ? 1 : 0;
					disable_messages = (shipp->flags2 & SF2_NO_BUILTIN_MESSAGES) ? 1 : 0;
					set_class_dynamically = (shipp->flags2 & SF2_SET_CLASS_DYNAMICALLY) ? 1 : 0;
					no_death_scream = (shipp->flags2 & SF2_NO_DEATH_SCREAM) ? 1 : 0;
					always_death_scream = (shipp->flags2 & SF2_ALWAYS_DEATH_SCREAM) ? 1 : 0;
					guardian = (shipp->ship_guardian_threshold) ? 1 : 0;
					vaporize = (shipp->flags & SF_VAPORIZE) ? 1 : 0;
					stealth = (shipp->flags2 & SF2_STEALTH) ? 1 : 0;
					friendly_stealth_invisible = (shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS) ? 1 : 0;
					nav_carry = (shipp->flags2 & SF2_NAVPOINT_CARRY) ? 1 : 0; 
					nav_needslink = (shipp->flags2 & SF2_NAVPOINT_NEEDSLINK) ? 1 : 0;
					hide_ship_name = (shipp->flags2 & SF2_HIDE_SHIP_NAME) ? 1 : 0;
					no_ets = (shipp->flags2 & SF2_NO_ETS) ? 1 : 0;
					cloaked = (shipp->flags2 & SF2_CLOAKED) ? 1 : 0;

					destroy_before_mission = (shipp->flags & SF_KILL_BEFORE_MISSION) ? 1 : 0;
					m_destroy_value.init(shipp->final_death_time);

					kamikaze = (Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE) ? 1 : 0;
					m_kdamage.init( Ai_info[shipp->ai_index].kamikaze_damage );

					escort = (shipp->flags & SF_ESCORT) ? 1 : 0;
					m_escort_value.init(shipp->escort_priority);
					
					if(The_mission.game_type & MISSION_TYPE_MULTI) {
						m_respawn_priority.init(shipp->respawn_priority);
					}

					for (j=0; j<Num_reinforcements; j++) {
						if (!stricmp(Reinforcements[j].name, shipp->ship_name)) {
							break;
						}
					}

					reinforcement = (j < Num_reinforcements) ? 1 : 0;

					// check if ship in wing
					ship_in_wing = (shipp->wingnum != -1);;
				} else {

					scannable = tristate_set( shipp->flags & SF_SCANNABLE, scannable );
					red_alert_carry = tristate_set( shipp->flags & SF_RED_ALERT_STORE_STATUS, red_alert_carry );
					special_warpin = tristate_set( objp->flags & OF_SPECIAL_WARPIN, special_warpin );
					protect_ship = tristate_set(objp->flags & OF_PROTECTED, protect_ship);
					beam_protect_ship = tristate_set(objp->flags & OF_BEAM_PROTECTED, beam_protect_ship);
					flak_protect_ship = tristate_set(objp->flags & OF_FLAK_PROTECTED, flak_protect_ship);
					laser_protect_ship = tristate_set(objp->flags & OF_LASER_PROTECTED, laser_protect_ship);
					missile_protect_ship = tristate_set(objp->flags & OF_MISSILE_PROTECTED, missile_protect_ship);
					invulnerable = tristate_set(objp->flags & OF_INVULNERABLE, invulnerable);
					targetable_as_bomb = tristate_set(objp->flags & OF_TARGETABLE_AS_BOMB, targetable_as_bomb);
					immobile = tristate_set(objp->flags & OF_IMMOBILE, immobile);
					hidden_from_sensors = tristate_set(shipp->flags & SF_HIDDEN_FROM_SENSORS, hidden_from_sensors);
					primitive_sensors = tristate_set(shipp->flags2 & SF2_PRIMITIVE_SENSORS, primitive_sensors);
					no_subspace_drive = tristate_set(shipp->flags2 & SF2_NO_SUBSPACE_DRIVE, no_subspace_drive);
					affected_by_gravity = tristate_set(shipp->flags2 & SF2_AFFECTED_BY_GRAVITY, affected_by_gravity);
					toggle_subsystem_scanning = tristate_set(shipp->flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING, toggle_subsystem_scanning);
					ignore_count = tristate_set(shipp->flags & SF_IGNORE_COUNT, ignore_count);
					no_arrival_music = tristate_set(shipp->flags & SF_NO_ARRIVAL_MUSIC, no_arrival_music);
					cargo_known = tristate_set(shipp->flags & SF_CARGO_REVEALED, cargo_known);
					no_dynamic = tristate_set( Ai_info[shipp->ai_index].ai_flags & AIF_NO_DYNAMIC, no_dynamic );
					disable_messages = tristate_set(shipp->flags2 & SF2_NO_BUILTIN_MESSAGES, disable_messages);
					set_class_dynamically = tristate_set(shipp->flags2 & SF2_SET_CLASS_DYNAMICALLY, set_class_dynamically);
					no_death_scream = tristate_set(shipp->flags2 & SF2_NO_DEATH_SCREAM, no_death_scream);
					always_death_scream = tristate_set(shipp->flags2 & SF2_ALWAYS_DEATH_SCREAM, always_death_scream);
					guardian = tristate_set(shipp->ship_guardian_threshold, guardian);
					vaporize = tristate_set(shipp->flags & SF_VAPORIZE, vaporize);
					stealth = tristate_set(shipp->flags2 & SF2_STEALTH, stealth);
					friendly_stealth_invisible = tristate_set(shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS, friendly_stealth_invisible);
					nav_carry = tristate_set(shipp->flags2 & SF2_NAVPOINT_CARRY, nav_carry);
					nav_needslink = tristate_set(shipp->flags2 & SF2_NAVPOINT_NEEDSLINK, nav_needslink);
					hide_ship_name = tristate_set(shipp->flags2 & SF2_HIDE_SHIP_NAME, hide_ship_name);
					no_ets = tristate_set(shipp->flags2 & SF2_NO_ETS, no_ets);
					cloaked = tristate_set(shipp->flags2 & SF2_CLOAKED, cloaked);

					// check the final death time and set the internal variable according to whether or not
					// the final_death_time is set.  Also, the value in the edit box must be set if all the
					// values are the same, and cleared if the values are not the same.
					destroy_before_mission = tristate_set(shipp->flags & SF_KILL_BEFORE_MISSION, destroy_before_mission);
					m_destroy_value.set(shipp->final_death_time);

					kamikaze = tristate_set( Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE, kamikaze );
					m_kdamage.set( Ai_info[shipp->ai_index].kamikaze_damage );

					escort = tristate_set(shipp->flags & SF_ESCORT, escort);
					m_escort_value.init(shipp->escort_priority);

					if(The_mission.game_type & MISSION_TYPE_MULTI) {
						m_respawn_priority.init(shipp->escort_priority);
					}

					for (j=0; j<Num_reinforcements; j++) {
						if (!stricmp(Reinforcements[j].name, shipp->ship_name)) {
							break;
						}
					}
					reinforcement = tristate_set(j < Num_reinforcements, reinforcement);

					// check if ship in wing
					ship_in_wing = (shipp->wingnum != -1);;
				}

			}
		}

		objp = GET_NEXT(objp);
	}

	CDialog::OnInitDialog();
	
	m_protect_ship.SetCheck(protect_ship);
	m_beam_protect_ship.SetCheck(beam_protect_ship);
	m_flak_protect_ship.SetCheck(flak_protect_ship);
	m_laser_protect_ship.SetCheck(laser_protect_ship);
	m_missile_protect_ship.SetCheck(missile_protect_ship);
	m_ignore_count.SetCheck(ignore_count);
	m_reinforcement.SetCheck(reinforcement);
	m_cargo_known.SetCheck(cargo_known);
	m_destroy.SetCheck(destroy_before_mission);
	m_no_arrival_music.SetCheck(no_arrival_music);
	m_escort.SetCheck(escort);
	m_invulnerable.SetCheck(invulnerable);
	m_targetable_as_bomb.SetCheck(targetable_as_bomb);
	m_immobile.SetCheck(immobile);
	m_hidden.SetCheck(hidden_from_sensors);
	m_primitive_sensors.SetCheck(primitive_sensors);
	m_no_subspace_drive.SetCheck(no_subspace_drive);
	m_affected_by_gravity.SetCheck(affected_by_gravity);
	m_toggle_subsystem_scanning.SetCheck(toggle_subsystem_scanning);
	m_scannable.SetCheck(scannable);
	m_kamikaze.SetCheck(kamikaze);
	m_no_dynamic.SetCheck(no_dynamic);
	m_red_alert_carry.SetCheck(red_alert_carry);
	m_special_warpin.SetCheck(special_warpin);
	m_disable_messages.SetCheck(disable_messages);
	m_set_class_dynamically.SetCheck(set_class_dynamically);
	m_no_death_scream.SetCheck(no_death_scream);
	m_always_death_scream.SetCheck(always_death_scream);
	m_guardian.SetCheck(guardian);
	m_vaporize.SetCheck(vaporize);
	m_stealth.SetCheck(stealth);
	m_friendly_stealth_invisible.SetCheck(friendly_stealth_invisible);
	m_nav_carry.SetCheck(nav_carry);
	m_nav_needslink.SetCheck(nav_needslink);
	m_hide_ship_name.SetCheck(hide_ship_name);
	m_disable_ets.SetCheck(no_ets);
	m_cloaked.SetCheck(cloaked);
		
	m_kdamage.setup(IDC_KDAMAGE, this);
	m_destroy_value.setup(IDC_DESTROY_VALUE, this);
	m_escort_value.setup(IDC_ESCORT_PRIORITY, this);

	if(The_mission.game_type & MISSION_TYPE_MULTI) {
		m_respawn_priority.setup(IDC_RESPAWN_PRIORITY, this);
	}
	m_destroy_spin.SetRange(0, UD_MAXVAL);

	m_destroy_value.display();
	m_kdamage.display();
	m_escort_value.display();

	if(The_mission.game_type & MISSION_TYPE_MULTI) {
		m_respawn_priority.display();
	} else {
		GetDlgItem(IDC_RESPAWN_PRIORITY)->EnableWindow(FALSE);
	}

	// flags that enable/disable according to whether this isn't a player
	GetDlgItem(IDC_REINFORCEMENT)->EnableWindow(p_enable && !ship_in_wing);
	GetDlgItem(IDC_CARGO_KNOWN)->EnableWindow(p_enable);
	GetDlgItem(IDC_DESTROY_CHECK)->EnableWindow(p_enable);
	GetDlgItem(IDC_DESTROY_VALUE)->EnableWindow(p_enable);
	GetDlgItem(IDC_DESTROY_SPIN)->EnableWindow(p_enable);
	GetDlgItem(IDC_SCANNABLE)->EnableWindow(p_enable);

	// disable the spinner and edit window if the corrsponding check box
	// is not checked!
	if (m_destroy.GetCheck() != 1) {
		GetDlgItem(IDC_DESTROY_VALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESTROY_SPIN)->EnableWindow(FALSE);
	}

	// disable destroy option for ship in wing
	if (ship_in_wing) {
		GetDlgItem(IDC_DESTROY_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESTROY_VALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESTROY_SPIN)->EnableWindow(FALSE);
	}

	// maybe disable escort priority window
	if (m_escort.GetCheck() == 1)
		GetDlgItem(IDC_ESCORT_PRIORITY)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_ESCORT_PRIORITY)->EnableWindow(FALSE);

	// maybe disable kamikaze damage window
	if (m_kamikaze.GetCheck() == 1)
		GetDlgItem(IDC_KDAMAGE)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_KDAMAGE)->EnableWindow(FALSE);

	return TRUE;
}

void ship_flags_dlg::OnOK()
{
	object *objp;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags & OF_MARKED)
				update_ship(objp->instance);
		}

		objp = GET_NEXT(objp);
	}

	CDialog::OnOK();
}

void ship_flags_dlg::update_ship(int shipnum)
{
	ship *shipp = &Ships[shipnum];
	object *objp = &Objects[shipp->objnum];

	if (m_reinforcement.GetCheck() != 2)
	{
		//Check if we're trying to add more and we've got too many.
		if( (Num_reinforcements >= MAX_REINFORCEMENTS) && (m_reinforcement.GetCheck() == 1))
		{
			char error_message[256];
			sprintf(error_message, "Too many reinforcements; could not add ship '%s' to reinforcement list!", shipp->ship_name); 
			MessageBox(error_message);
		}
		//Otherwise, just update as normal.
		else
		{
			set_reinforcement(shipp->ship_name, m_reinforcement.GetCheck());	
		}
	}

	switch (m_cargo_known.GetCheck()) {
		case 1:
			if ( !(shipp->flags & SF_CARGO_REVEALED) )
				set_modified();

			shipp->flags |= SF_CARGO_REVEALED;
			break;

		case 0:
			if ( shipp->flags & SF_CARGO_REVEALED )
				set_modified();

			shipp->flags &= ~SF_CARGO_REVEALED;
			break;
	}

	// update the flags for IGNORE_COUNT and PROTECT_SHIP
	switch (m_protect_ship.GetCheck()) {
		case 1:
			if (!(objp->flags & OF_PROTECTED) )
				set_modified();

			objp->flags |= OF_PROTECTED;
			break;

		case 0:
			if ( objp->flags & OF_PROTECTED )
				set_modified();

			objp->flags &= ~OF_PROTECTED;
			break;
	}

	switch (m_beam_protect_ship.GetCheck()) {
		case 1:
			if (!(objp->flags & OF_BEAM_PROTECTED) )
				set_modified();

			objp->flags |= OF_BEAM_PROTECTED;
			break;

		case 0:
			if ( objp->flags & OF_BEAM_PROTECTED )
				set_modified();

			objp->flags &= ~OF_BEAM_PROTECTED;
			break;
	}

	switch (m_flak_protect_ship.GetCheck()) {
		case 1:
			if (!(objp->flags & OF_FLAK_PROTECTED) )
				set_modified();

			objp->flags |= OF_FLAK_PROTECTED;
			break;

		case 0:
			if ( objp->flags & OF_FLAK_PROTECTED )
				set_modified();

			objp->flags &= ~OF_FLAK_PROTECTED;
			break;
	}

	switch (m_laser_protect_ship.GetCheck()) {
		case 1:
			if (!(objp->flags & OF_LASER_PROTECTED) )
				set_modified();

			objp->flags |= OF_LASER_PROTECTED;
			break;

		case 0:
			if ( objp->flags & OF_LASER_PROTECTED )
				set_modified();

			objp->flags &= ~OF_LASER_PROTECTED;
			break;
	}

	switch (m_missile_protect_ship.GetCheck()) {
		case 1:
			if (!(objp->flags & OF_MISSILE_PROTECTED) )
				set_modified();

			objp->flags |= OF_MISSILE_PROTECTED;
			break;

		case 0:
			if ( objp->flags & OF_MISSILE_PROTECTED )
				set_modified();

			objp->flags &= ~OF_MISSILE_PROTECTED;
			break;
	}

	switch (m_invulnerable.GetCheck()) {
		case 1:
			if ( !(objp->flags & OF_INVULNERABLE) )
				set_modified();

			objp->flags |= OF_INVULNERABLE;
			break;

		case 0:
			if ( objp->flags & OF_INVULNERABLE )
				set_modified();

			objp->flags &= ~OF_INVULNERABLE;
			break;
	}

	switch (m_targetable_as_bomb.GetCheck()) {
		case 1:
			if ( !(objp->flags & OF_TARGETABLE_AS_BOMB) )
				set_modified();

			objp->flags |= OF_TARGETABLE_AS_BOMB;
			break;

		case 0:
			if ( objp->flags & OF_TARGETABLE_AS_BOMB )
				set_modified();

			objp->flags &= ~OF_TARGETABLE_AS_BOMB;
			break;
	}

	switch (m_immobile.GetCheck()) {
		case 1:
			if ( !(objp->flags & OF_IMMOBILE) )
				set_modified();

			objp->flags |= OF_IMMOBILE;
			break;

		case 0:
			if ( objp->flags & OF_IMMOBILE )
				set_modified();

			objp->flags &= ~OF_IMMOBILE;
			break;
	}

	switch (m_hidden.GetCheck()) {
		case 1:
			if ( !(shipp->flags & SF_HIDDEN_FROM_SENSORS) )
				set_modified();

			shipp->flags |= SF_HIDDEN_FROM_SENSORS;
			break;

		case 0:
			if ( shipp->flags & SF_HIDDEN_FROM_SENSORS )
				set_modified();

			shipp->flags &= ~SF_HIDDEN_FROM_SENSORS;
			break;
	}

	switch (m_primitive_sensors.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_PRIMITIVE_SENSORS) )
				set_modified();

			shipp->flags2 |= SF2_PRIMITIVE_SENSORS;
			break;

		case 0:
			if ( shipp->flags2 & SF2_PRIMITIVE_SENSORS )
				set_modified();

			shipp->flags2 &= ~SF2_PRIMITIVE_SENSORS;
			break;
	}

	switch (m_no_subspace_drive.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_NO_SUBSPACE_DRIVE) )
				set_modified();

			shipp->flags2 |= SF2_NO_SUBSPACE_DRIVE;
			break;

		case 0:
			if ( shipp->flags2 & SF2_NO_SUBSPACE_DRIVE )
				set_modified();

			shipp->flags2 &= ~SF2_NO_SUBSPACE_DRIVE;
			break;
	}

	switch (m_affected_by_gravity.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_AFFECTED_BY_GRAVITY) )
				set_modified();

			shipp->flags2 |= SF2_AFFECTED_BY_GRAVITY;
			break;

		case 0:
			if ( shipp->flags2 & SF2_AFFECTED_BY_GRAVITY )
				set_modified();

			shipp->flags2 &= ~SF2_AFFECTED_BY_GRAVITY;
			break;
	}

	switch (m_toggle_subsystem_scanning.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING) )
				set_modified();

			shipp->flags2 |= SF2_TOGGLE_SUBSYSTEM_SCANNING;
			break;

		case 0:
			if ( shipp->flags2 & SF2_TOGGLE_SUBSYSTEM_SCANNING )
				set_modified();

			shipp->flags2 &= ~SF2_TOGGLE_SUBSYSTEM_SCANNING;
			break;
	}

	switch (m_ignore_count.GetCheck()) {
		case 1:
			if ( !(shipp->flags & SF_IGNORE_COUNT) )
				set_modified();

			shipp->flags |= SF_IGNORE_COUNT;
			break;

		case 0:
			if (shipp->flags & SF_IGNORE_COUNT)
				set_modified();

			shipp->flags &= ~SF_IGNORE_COUNT;
			break;
	}

	switch (m_escort.GetCheck()) {
		case 1:
			if (!(shipp->flags & SF_ESCORT))
				set_modified();

			shipp->flags |= SF_ESCORT;
			m_escort_value.save(&shipp->escort_priority);
			break;

		case 0:
			if (shipp->flags & SF_ESCORT)
				set_modified();

			shipp->flags &= ~SF_ESCORT;
			break;
	}

	// deal with updating the "destroy before the mission" stuff
	switch (m_destroy.GetCheck()) {
		case 0:  // this means no check in checkbox
			if ( shipp->flags & SF_KILL_BEFORE_MISSION )
				set_modified();

			shipp->flags &= ~SF_KILL_BEFORE_MISSION;
			break;

		case 1:  // this means checkbox is checked
			if ( !(shipp->flags & SF_KILL_BEFORE_MISSION) )
				set_modified();

			shipp->flags |= SF_KILL_BEFORE_MISSION;
			m_destroy_value.save(&shipp->final_death_time);
			break;
	}  // a mixed state is 2, and since it's not handled, it doesn't change

	switch (m_no_arrival_music.GetCheck()) {
		case 0:
			if (shipp->flags & SF_NO_ARRIVAL_MUSIC)
				set_modified();

			shipp->flags &= ~SF_NO_ARRIVAL_MUSIC;
			break;

		case 1:
			if (!(shipp->flags & SF_NO_ARRIVAL_MUSIC))
				set_modified();

			shipp->flags |= SF_NO_ARRIVAL_MUSIC;
			break;
	}

	switch (m_scannable.GetCheck()) {
		case 1:
			if ( !(shipp->flags & SF_SCANNABLE) )
				set_modified();

			shipp->flags |= SF_SCANNABLE;
			break;

		case 0:
			if ( shipp->flags & SF_SCANNABLE )
				set_modified();

			shipp->flags &= ~SF_SCANNABLE;
			break;
	}

	switch (m_red_alert_carry.GetCheck()) {
		case 1:
			if ( !(shipp->flags & SF_RED_ALERT_STORE_STATUS) )
				set_modified();

			shipp->flags |= SF_RED_ALERT_STORE_STATUS;
			break;

		case 0:
			if ( shipp->flags & SF_RED_ALERT_STORE_STATUS )
				set_modified();

			shipp->flags &= ~SF_RED_ALERT_STORE_STATUS;
			break;
	}

	switch (m_special_warpin.GetCheck()) {
		case 1:
			if ( !(objp->flags & OF_SPECIAL_WARPIN) )
				set_modified();

			objp->flags |= OF_SPECIAL_WARPIN;
			break;

		case 0:
			if ( (objp->flags & OF_SPECIAL_WARPIN) )
				set_modified();

			objp->flags &= (~OF_SPECIAL_WARPIN);
			break;
	}

	switch (m_no_dynamic.GetCheck()) {
		case 1:
			if ( !(Ai_info[shipp->ai_index].ai_flags & AIF_NO_DYNAMIC) )
				set_modified();

			Ai_info[shipp->ai_index].ai_flags |= AIF_NO_DYNAMIC;
			break;

		case 0:
			if ( Ai_info[shipp->ai_index].ai_flags & AIF_NO_DYNAMIC )
				set_modified();

			Ai_info[shipp->ai_index].ai_flags &= ~AIF_NO_DYNAMIC;
			break;
	}

	switch (m_kamikaze.GetCheck()) {
		case 1: {
			int damage;

			if ( !(Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE) )
				set_modified();

			Ai_info[shipp->ai_index].ai_flags |= AIF_KAMIKAZE;
			m_kdamage.save(&damage);
			Ai_info[shipp->ai_index].kamikaze_damage = damage;
			break;
		}

		case 0:
			if ( Ai_info[shipp->ai_index].ai_flags & AIF_KAMIKAZE )
				set_modified();

			Ai_info[shipp->ai_index].ai_flags &= ~AIF_KAMIKAZE;
			Ai_info[shipp->ai_index].kamikaze_damage = 0;
			break;
	}

	switch (m_disable_messages.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_NO_BUILTIN_MESSAGES) )
				set_modified();

			shipp->flags2 |= SF2_NO_BUILTIN_MESSAGES;
			break;

		case 0:
			if ( shipp->flags2 & SF2_NO_BUILTIN_MESSAGES )
				set_modified();

			shipp->flags2 &= ~SF2_NO_BUILTIN_MESSAGES;
			break;
	}

	switch (m_set_class_dynamically.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_SET_CLASS_DYNAMICALLY) )
				set_modified();

			shipp->flags2 |= SF2_SET_CLASS_DYNAMICALLY;
			break;

		case 0:
			if ( shipp->flags2 & SF2_SET_CLASS_DYNAMICALLY )
				set_modified();

			shipp->flags2 &= ~SF2_SET_CLASS_DYNAMICALLY;
			break;
	}

	switch (m_no_death_scream.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_NO_DEATH_SCREAM) )
				set_modified();

			shipp->flags2 |= SF2_NO_DEATH_SCREAM;
			break;

		case 0:
			if ( shipp->flags2 & SF2_NO_DEATH_SCREAM )
				set_modified();

			shipp->flags2 &= ~SF2_NO_DEATH_SCREAM;
			break;
	}

	switch (m_always_death_scream.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_ALWAYS_DEATH_SCREAM) )
				set_modified();

			shipp->flags2 |= SF2_ALWAYS_DEATH_SCREAM;
			break;

		case 0:
			if ( shipp->flags2 & SF2_ALWAYS_DEATH_SCREAM )
				set_modified();

			shipp->flags2 &= ~SF2_ALWAYS_DEATH_SCREAM;
			break;
	}

	switch (m_nav_carry.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_NAVPOINT_CARRY) )
				set_modified();

			shipp->flags2 |= SF2_NAVPOINT_CARRY;
			break;

		case 0:
			if ( shipp->flags2 & SF2_NAVPOINT_CARRY )
				set_modified();

			shipp->flags2 &= ~SF2_NAVPOINT_CARRY;
			break;
	}

	switch (m_nav_needslink.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_NAVPOINT_NEEDSLINK) )
				set_modified();

			shipp->flags2 |= SF2_NAVPOINT_NEEDSLINK;
			break;

		case 0:
			if ( shipp->flags2 & SF2_NAVPOINT_NEEDSLINK )
				set_modified();

			shipp->flags2 &= ~SF2_NAVPOINT_NEEDSLINK;
			break;
	}

	switch (m_hide_ship_name.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_HIDE_SHIP_NAME) )
				set_modified();

			shipp->flags2 |= SF2_HIDE_SHIP_NAME;
			break;

		case 0:
			if ( shipp->flags2 & SF2_HIDE_SHIP_NAME )
				set_modified();

			shipp->flags2 &= ~SF2_HIDE_SHIP_NAME;
			break;
	}

	switch (m_disable_ets.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_NO_ETS) )
				set_modified();

			shipp->flags2 |= SF2_NO_ETS;
			break;

		case 0:
			if ( shipp->flags2 & SF2_NO_ETS )
				set_modified();

			shipp->flags2 &= ~SF2_NO_ETS;
			break;
	}

	switch (m_cloaked.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_CLOAKED) )
				set_modified();

			shipp->flags2 |= SF2_CLOAKED;
			break;

		case 0:
			if ( shipp->flags2 & SF2_CLOAKED )
				set_modified();

			shipp->flags2 &= ~SF2_CLOAKED;
			break;
	}

	switch (m_guardian.GetCheck()) {
		case 1:
			if ( !(shipp->ship_guardian_threshold) )
				set_modified();

			shipp->ship_guardian_threshold = SHIP_GUARDIAN_THRESHOLD_DEFAULT;
			break;

		case 0:
			if ( shipp->ship_guardian_threshold )
				set_modified();

			shipp->ship_guardian_threshold = 0;
			break;
	}

	switch (m_vaporize.GetCheck()) {
		case 1:
			if ( !(shipp->flags & SF_VAPORIZE) )
				set_modified();

			shipp->flags |= SF_VAPORIZE;
			break;

		case 0:
			if ( shipp->flags & SF_VAPORIZE )
				set_modified();

			shipp->flags &= ~SF_VAPORIZE;
			break;
	}

	switch (m_stealth.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_STEALTH) )
				set_modified();

			shipp->flags2 |= SF2_STEALTH;
			break;

		case 0:
			if ( shipp->flags2 & SF2_STEALTH )
				set_modified();

			shipp->flags2 &= ~SF2_STEALTH;
			break;
	}

	switch (m_friendly_stealth_invisible.GetCheck()) {
		case 1:
			if ( !(shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS) )
				set_modified();

			shipp->flags2 |= SF2_FRIENDLY_STEALTH_INVIS;
			break;

		case 0:
			if ( shipp->flags2 & SF2_FRIENDLY_STEALTH_INVIS )
				set_modified();

			shipp->flags2 &= ~SF2_FRIENDLY_STEALTH_INVIS;
			break;
	}

	shipp->respawn_priority = 0;
	if(The_mission.game_type & MISSION_TYPE_MULTI) {
		m_respawn_priority.save(&shipp->respawn_priority);
	}
}

int ship_flags_dlg::tristate_set(int val, int cur_state)
{
	if (val) {
		if (!cur_state) {
			return 2;
		}
	} else {
		if (cur_state) {
			return 2;
		}
	}

	return cur_state;
}

// a stub for now, but might be useful later.  Easier than ripping out the calls to this
// everywhere at least.
void ship_flags_dlg::set_modified()
{
}

void ship_flags_dlg::OnCargoKnown() 
{
	if (m_cargo_known.GetCheck() == 1) {
		m_cargo_known.SetCheck(0);
	} else {
		m_cargo_known.SetCheck(1);
	}
}

void ship_flags_dlg::OnDestroyCheck() 
{
	if (m_destroy.GetCheck() == 1) {
		m_destroy.SetCheck(0);
		GetDlgItem(IDC_DESTROY_VALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESTROY_SPIN)->EnableWindow(FALSE);

	} else {
		m_destroy.SetCheck(1);
		GetDlgItem(IDC_DESTROY_VALUE)->EnableWindow(TRUE);
		GetDlgItem(IDC_DESTROY_SPIN)->EnableWindow(TRUE);
	}
}

void ship_flags_dlg::OnEscort() 
{
	if (m_escort.GetCheck() == 1) {
		m_escort.SetCheck(0);
		GetDlgItem(IDC_ESCORT_PRIORITY)->EnableWindow(FALSE);
	} else {
		m_escort.SetCheck(1);
		GetDlgItem(IDC_ESCORT_PRIORITY)->EnableWindow(TRUE);
	}
}

void ship_flags_dlg::OnHiddenFromSensors() 
{
	if (m_hidden.GetCheck() == 1) {
		m_hidden.SetCheck(0);
	} else {
		m_hidden.SetCheck(1);
	}
}

void ship_flags_dlg::OnPrimitiveSensors() 
{
	if (m_primitive_sensors.GetCheck() == 1) {
		m_primitive_sensors.SetCheck(0);
	} else {
		m_primitive_sensors.SetCheck(1);
	}
}

void ship_flags_dlg::OnNoSubspaceDrive() 
{
	if (m_no_subspace_drive.GetCheck() == 1) {
		m_no_subspace_drive.SetCheck(0);
	} else {
		m_no_subspace_drive.SetCheck(1);
	}
}

void ship_flags_dlg::OnAffectedByGravity() 
{
	if (m_affected_by_gravity.GetCheck() == 1) {
		m_affected_by_gravity.SetCheck(0);
	} else {
		m_affected_by_gravity.SetCheck(1);
	}
}

void ship_flags_dlg::OnToggleSubsystemScanning() 
{
	if (m_toggle_subsystem_scanning.GetCheck() == 1) {
		m_toggle_subsystem_scanning.SetCheck(0);
	} else {
		m_toggle_subsystem_scanning.SetCheck(1);
	}
}

void ship_flags_dlg::OnIgnoreCount() 
{
	if (m_ignore_count.GetCheck() == 1) {
		m_ignore_count.SetCheck(0);
	} else {
		m_ignore_count.SetCheck(1);
	}
}

void ship_flags_dlg::OnInvulnerable() 
{
	if (m_invulnerable.GetCheck() == 1) {
		m_invulnerable.SetCheck(0);
	} else {
		m_invulnerable.SetCheck(1);
	}
}

void ship_flags_dlg::OnTargetableAsBomb() 
{
	if (m_targetable_as_bomb.GetCheck() == 1) {
		m_targetable_as_bomb.SetCheck(0);
	} else {
		m_targetable_as_bomb.SetCheck(1);
	}
}

void ship_flags_dlg::OnImmobile() 
{
	if (m_immobile.GetCheck() == 1) {
		m_immobile.SetCheck(0);
	} else {
		m_immobile.SetCheck(1);
	}
}

void ship_flags_dlg::OnKamikaze() 
{
	if (m_kamikaze.GetCheck() == 1) {
		GetDlgItem(IDC_KDAMAGE)->EnableWindow(FALSE);
		m_kamikaze.SetCheck(0);

	} else {
		GetDlgItem(IDC_KDAMAGE)->EnableWindow(TRUE);
		m_kamikaze.SetCheck(1);
	}
}

void ship_flags_dlg::OnNoArrivalMusic() 
{
	if (m_no_arrival_music.GetCheck() == 1) {
		m_no_arrival_music.SetCheck(0);
	} else {
		m_no_arrival_music.SetCheck(1);
	}
}

void ship_flags_dlg::OnNoDynamic() 
{
	if (m_no_dynamic.GetCheck() == 1) {
		m_no_dynamic.SetCheck(0);
	} else {
		m_no_dynamic.SetCheck(1);
	}
}

void ship_flags_dlg::OnProtectShip() 
{
	if (m_protect_ship.GetCheck() == 1) {
		m_protect_ship.SetCheck(0);
	} else {
		m_protect_ship.SetCheck(1);
	}
}

void ship_flags_dlg::OnBeamProtectShip() 
{
	if (m_beam_protect_ship.GetCheck() == 1) {
		m_beam_protect_ship.SetCheck(0);
	} else {
		m_beam_protect_ship.SetCheck(1);
	}
}

void ship_flags_dlg::OnFlakProtectShip() 
{
	if (m_flak_protect_ship.GetCheck() == 1) {
		m_flak_protect_ship.SetCheck(0);
	} else {
		m_flak_protect_ship.SetCheck(1);
	}
}

void ship_flags_dlg::OnLaserProtectShip() 
{
	if (m_laser_protect_ship.GetCheck() == 1) {
		m_laser_protect_ship.SetCheck(0);
	} else {
		m_laser_protect_ship.SetCheck(1);
	}
}

void ship_flags_dlg::OnMissileProtectShip() 
{
	if (m_missile_protect_ship.GetCheck() == 1) {
		m_missile_protect_ship.SetCheck(0);
	} else {
		m_missile_protect_ship.SetCheck(1);
	}
}

void ship_flags_dlg::OnReinforcement() 
{
	if (m_reinforcement.GetCheck() == 1) {
		m_reinforcement.SetCheck(0);
	} else {
		m_reinforcement.SetCheck(1);
	}
}

void ship_flags_dlg::OnScannable() 
{
	if (m_scannable.GetCheck() == 1) {
		m_scannable.SetCheck(0);
	} else {
		m_scannable.SetCheck(1);
	}
}

void ship_flags_dlg::OnRedalertcarry() 
{
	if (m_red_alert_carry.GetCheck() == 1) {
		m_red_alert_carry.SetCheck(0);
	} else {
		m_red_alert_carry.SetCheck(1);
	}
}

void ship_flags_dlg::OnDisableBuiltinShip() 
{
	if (m_disable_messages.GetCheck() == 1) {
		m_disable_messages.SetCheck(0);
	} else {
		m_disable_messages.SetCheck(1);
	}
}

void ship_flags_dlg::OnSetClassDynamically() 
{
	if (m_set_class_dynamically.GetCheck() == 1) {
		m_set_class_dynamically.SetCheck(0);
	} else {
		m_set_class_dynamically.SetCheck(1);
	}
}
void ship_flags_dlg::OnNoDeathScream()
{
	if (m_no_death_scream.GetCheck() == 1) {
 		m_no_death_scream.SetCheck(0);
	} else {
		m_no_death_scream.SetCheck(1);
	}
}

void ship_flags_dlg::OnAlwaysDeathScream()
{
	if (m_always_death_scream.GetCheck() == 1) {
		m_always_death_scream.SetCheck(0);
	} else {
		m_always_death_scream.SetCheck(1);
	}
}

void ship_flags_dlg::OnGuardian()
{
	if (m_guardian.GetCheck() == 1) {
 		m_guardian.SetCheck(0);
	} else {
		m_guardian.SetCheck(1);
	}
}

void ship_flags_dlg::OnVaporize()
{
	if (m_vaporize.GetCheck() == 1) {
 		m_vaporize.SetCheck(0);
	} else {
		m_vaporize.SetCheck(1);
	}
}

void ship_flags_dlg::OnStealth()
{
	if (m_stealth.GetCheck() == 1) {
 		m_stealth.SetCheck(0);
	} else {
		m_stealth.SetCheck(1);
	}
}

void ship_flags_dlg::OnFriendlyStealthInvisible()
{
	if (m_friendly_stealth_invisible.GetCheck() == 1) {
 		m_friendly_stealth_invisible.SetCheck(0);
	} else {
		m_friendly_stealth_invisible.SetCheck(1);
	}
}

void ship_flags_dlg::OnNavCarry()
{
	if (m_nav_carry.GetCheck() == 1) {
 		m_nav_carry.SetCheck(0);
	} else {
		m_nav_carry.SetCheck(1);
	}
}

void ship_flags_dlg::OnNavNeedslink()
{
	if (m_nav_needslink.GetCheck() == 1) {
 		m_nav_needslink.SetCheck(0);
	} else {
		m_nav_needslink.SetCheck(1);
	}
}

void ship_flags_dlg::OnHideShipName()
{
	if (m_hide_ship_name.GetCheck() == 1) {
 		m_hide_ship_name.SetCheck(0);
	} else {
		m_hide_ship_name.SetCheck(1);
	}
}



void ship_flags_dlg::OnDisableETS()
{
	if (m_disable_ets.GetCheck() == 1) {
		m_disable_ets.SetCheck(0);
	} else {
		m_disable_ets.SetCheck(1);
	}
}

void ship_flags_dlg::OnCloaked()
{
	if (m_cloaked.GetCheck() == 1) {
		m_cloaked.SetCheck(0);
	} else {
		m_cloaked.SetCheck(1);
	}
}
