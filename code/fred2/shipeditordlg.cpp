/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "MainFrm.h"
#include "render/3d.h"
#include "physics/physics.h"
#include "editor.h"
#include "ai/ailocal.h"
#include "ai/aigoals.h"
#include "parse/parselo.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "InitialStatus.h"
#include "WeaponEditorDlg.h"
#include "ship/ship.h"
#include "TextViewDlg.h"
#include "playerman/player.h"				// used for the max_keyed_target stuff
#include "IgnoreOrdersDlg.h"
#include "mission/missionparse.h"
#include "model/model.h"
#include "starfield/starfield.h"
#include "jumpnode/jumpnode.h"
#include "ShipFlagsDlg.h"
#include "mission/missionmessage.h"
#include "ShipSpecialDamage.h"
#include "ShipTexturesDlg.h"
#include "ShipSpecialHitpoints.h"
#include "altshipclassdlg.h"
#include "species_defs/species_defs.h"
#include "iff_defs/iff_defs.h"
#include "restrictpaths.h"

#define ID_SHIP_MENU 9000

#define NO_PERSONA_INDEX	999

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void numeric_edit_control::setup(int id, CWnd *wnd)
{
	control_id = id;
	dlg = wnd;
}

void numeric_edit_control::init(int n)
{
	value = n;
	unique = 1;
}

void numeric_edit_control::set(int n)
{
	if (n != value){
		unique = 0;
	}
}

void numeric_edit_control::display()
{
	CString str;

	if (unique){
		str.Format("%d", value);
	}

	dlg->GetDlgItem(control_id)->SetWindowText(str);
}

void numeric_edit_control::save(int *n)
{
	CString str;

	if (control_id) {
		dlg->GetDlgItem(control_id)->GetWindowText(str);
		if (!str.IsEmpty()){
			MODIFY(*n, atoi(str));
		}
	}
}

void numeric_edit_control::fix(int n)
{
	if (unique) {
		CString str;
		CWnd *w;

		value = n;
		str.Format("%d", n);
		w = dlg->GetDlgItem(control_id);
		dlg->GetDlgItem(control_id)->SetWindowText(str);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShipEditorDlg dialog

CShipEditorDlg::CShipEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShipEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShipEditorDlg)
	m_ship_name = _T("");
	m_cargo1 = _T("");
	m_ship_class = -1;
	m_team = -1;
	m_arrival_location = -1;
	m_departure_location = -1;
	m_ai_class = -1;
	m_hotkey = -1;
	m_update_arrival = FALSE;
	m_update_departure = FALSE;
	m_arrival_target = -1;
	m_departure_target = -1;
	m_persona = -1;	
	//}}AFX_DATA_INIT

	m_pSEView = NULL;
	initialized = editing = multi_edit = 0;
	select_sexp_node = -1;
	bypass_errors = 0;
}

//	Modeless constructor, MK
CShipEditorDlg::CShipEditorDlg(CView* pView)
{
	m_pSEView = pView;
	initialized = editing = 0;
	select_sexp_node = -1;
}

void CShipEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	int n;
	CString str;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShipEditorDlg)
	DDX_Control(pDX, IDC_NO_DEPARTURE_WARP, m_no_departure_warp);
	DDX_Control(pDX, IDC_NO_ARRIVAL_WARP, m_no_arrival_warp);
	DDX_Control(pDX, IDC_PLAYER_SHIP, m_player_ship);
	DDX_Control(pDX, IDC_DEPARTURE_DELAY_SPIN, m_departure_delay_spin);
	DDX_Control(pDX, IDC_ARRIVAL_DELAY_SPIN, m_arrival_delay_spin);
	DDX_Control(pDX, IDC_DEPARTURE_TREE, m_departure_tree);
	DDX_Control(pDX, IDC_ARRIVAL_TREE, m_arrival_tree);
	DDX_Text(pDX, IDC_SHIP_NAME, m_ship_name);
	DDX_CBString(pDX, IDC_SHIP_CARGO1, m_cargo1);
	DDX_CBIndex(pDX, IDC_SHIP_CLASS, m_ship_class);
	DDX_CBIndex(pDX, IDC_SHIP_TEAM, m_team);
	DDX_CBIndex(pDX, IDC_ARRIVAL_LOCATION, m_arrival_location);
	DDX_CBIndex(pDX, IDC_DEPARTURE_LOCATION, m_departure_location);
	DDX_CBIndex(pDX, IDC_AI_CLASS, m_ai_class);
	DDX_CBIndex(pDX, IDC_HOTKEY, m_hotkey);
	DDX_Check(pDX, IDC_UPDATE_ARRIVAL, m_update_arrival);
	DDX_Check(pDX, IDC_UPDATE_DEPARTURE, m_update_departure);
	DDX_CBIndex(pDX, IDC_ARRIVAL_TARGET, m_arrival_target);
	DDX_CBIndex(pDX, IDC_DEPARTURE_TARGET, m_departure_target);
	DDX_CBIndex(pDX, IDC_SHIP_PERSONA, m_persona);	
	//}}AFX_DATA_MAP
	DDV_MaxChars(pDX, m_ship_name, NAME_LENGTH - 1);
	DDV_MaxChars(pDX, m_cargo1, NAME_LENGTH - 1);

	if (pDX->m_bSaveAndValidate) {  // get dialog control values
		GetDlgItem(IDC_ARRIVAL_DELAY)->GetWindowText(str);
		n = atoi(str);
		if (n < 0){
			n = 0;
		}

		m_arrival_delay.init(n);

		GetDlgItem(IDC_ARRIVAL_DISTANCE)->GetWindowText(str);
		m_arrival_dist.init(atoi(str));

		GetDlgItem(IDC_DEPARTURE_DELAY)->GetWindowText(str);
		n = atoi(str);
		if (n < 0)
			n = 0;
		m_departure_delay.init(n);

		GetDlgItem(IDC_SCORE)->GetWindowText(str);
		m_score.init(atoi(str));

		GetDlgItem(IDC_ASSIST_SCORE)->GetWindowText(str);
		m_assist_score.init(atoi(str));
	}
}

BEGIN_MESSAGE_MAP(CShipEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CShipEditorDlg)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_ARRIVAL_TREE, OnRclickArrivalTree)
	ON_NOTIFY(NM_RCLICK, IDC_DEPARTURE_TREE, OnRclickDepartureTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_ARRIVAL_TREE, OnBeginlabeleditArrivalTree)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_DEPARTURE_TREE, OnBeginlabeleditDepartureTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_ARRIVAL_TREE, OnEndlabeleditArrivalTree)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_DEPARTURE_TREE, OnEndlabeleditDepartureTree)
	ON_BN_CLICKED(IDC_GOALS, OnGoals)
	ON_CBN_SELCHANGE(IDC_SHIP_CLASS, OnSelchangeShipClass)
	ON_BN_CLICKED(IDC_INITIAL_STATUS, OnInitialStatus)
	ON_BN_CLICKED(IDC_WEAPONS, OnWeapons)
	ON_BN_CLICKED(IDC_SHIP_RESET, OnShipReset)
	ON_BN_CLICKED(IDC_DELETE_SHIP, OnDeleteShip)
	ON_BN_CLICKED(IDC_SHIP_TBL, OnShipTbl)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_NOTIFY(TVN_SELCHANGED, IDC_ARRIVAL_TREE, OnSelchangedArrivalTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_DEPARTURE_TREE, OnSelchangedDepartureTree)
	ON_BN_CLICKED(IDC_HIDE_CUES, OnHideCues)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
	ON_CBN_SELCHANGE(IDC_ARRIVAL_LOCATION, OnSelchangeArrivalLocation)
	ON_BN_CLICKED(IDC_PLAYER_SHIP, OnPlayerShip)
	ON_BN_CLICKED(IDC_NO_ARRIVAL_WARP, OnNoArrivalWarp)
	ON_BN_CLICKED(IDC_NO_DEPARTURE_WARP, OnNoDepartureWarp)
	ON_CBN_SELCHANGE(IDC_DEPARTURE_LOCATION, OnSelchangeDepartureLocation)
	ON_CBN_SELCHANGE(IDC_HOTKEY, OnSelchangeHotkey)
	ON_BN_CLICKED(IDC_FLAGS, OnFlags)
	ON_BN_CLICKED(IDC_IGNORE_ORDERS, OnIgnoreOrders)
	ON_BN_CLICKED(IDC_SPECIAL_EXP, OnSpecialExp)
	ON_BN_CLICKED(IDC_TEXTURES, OnTextures)
	ON_BN_CLICKED(IDC_SPECIAL_HITPOINTS, OnSpecialHitpoints)
	ON_BN_CLICKED(IDC_ALT_SHIP_CLASS, OnAltShipClass)
	ON_BN_CLICKED(IDC_RESTRICT_ARRIVAL, OnRestrictArrival)
	ON_BN_CLICKED(IDC_RESTRICT_DEPARTURE, OnRestrictDeparture)
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_SET_AS_PLAYER_SHIP, OnSetAsPlayerShip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShipEditorDlg message handlers

BOOL CShipEditorDlg::Create()
{
	int i, index;
	BOOL r;
	CComboBox *ptr;

	r = CDialog::Create(IDD, Fred_main_wnd);

	ptr = (CComboBox *) GetDlgItem(IDC_ARRIVAL_LOCATION);
	ptr->ResetContent();
	for (i=0; i<MAX_ARRIVAL_NAMES; i++){
		ptr->AddString(Arrival_location_names[i]);
	}

	ptr = (CComboBox *) GetDlgItem(IDC_DEPARTURE_LOCATION);
	ptr->ResetContent();
	for (i=0; i<MAX_DEPARTURE_NAMES; i++){
		ptr->AddString(Departure_location_names[i]);
	}

	ptr = (CComboBox *) GetDlgItem(IDC_SHIP_CLASS);
	ptr->ResetContent();
	for (i=0; i<Num_ship_classes; i++){
		ptr->AddString(Ship_info[i].name);
	}

	ptr = (CComboBox *) GetDlgItem(IDC_AI_CLASS);
	ptr->ResetContent();
	for (i=0; i<Num_ai_classes; i++){
		ptr->AddString(Ai_class_names[i]);
	}

	// alternate ship name combobox		
	ptr = (CComboBox *)GetDlgItem(IDC_SHIP_ALT);
	ptr->ResetContent();
	ptr->AddString("<none>");
	ptr->SetCurSel(0);

	// deal with the persona dialog
	ptr = (CComboBox *)GetDlgItem(IDC_SHIP_PERSONA);
	ptr->ResetContent();
	index = ptr->AddString("<None>");
	if ( index >= 0 ){
		ptr->SetItemData(index, NO_PERSONA_INDEX);
	}	

	for ( i = 0; i < Num_personas; i++ ) {
		if ( Personas[i].flags & PERSONA_FLAG_WINGMAN ) {
			int index;

			// don't bother putting any vasudan personas on the list -- done automatically by code
//			if ( Personas[i].flags & PERSONA_FLAG_VASUDAN ){
//				continue;
//			}

			CString persona_name = Personas[i].name;
			if ( Personas[i].flags & PERSONA_FLAG_VASUDAN ){
				persona_name += " -Vas";
			}

			index = ptr->AddString(persona_name);
			if ( index >= 0 ){
				ptr->SetItemData(index, i);
			}
		}
	}

	m_score.setup(IDC_SCORE, this);
	m_assist_score.setup(IDC_ASSIST_SCORE, this);
	m_arrival_dist.setup(IDC_ARRIVAL_DISTANCE, this);
	m_arrival_delay.setup(IDC_ARRIVAL_DELAY, this);
	m_departure_delay.setup(IDC_DEPARTURE_DELAY, this);

	m_hotkey = 0;
	m_arrival_tree.link_modified(&modified);  // provide way to indicate trees are modified in dialog
	m_arrival_tree.setup((CEdit *) GetDlgItem(IDC_HELP_BOX));
	m_departure_tree.link_modified(&modified);
	m_departure_tree.setup();
	m_arrival_delay_spin.SetRange(0, 999);
	m_departure_delay_spin.SetRange(0, 999);
	initialize_data(1);	

	return r;
}

//	This gets called when you click on the "X" button.  Note that OnClose
//	does not destroy the window.  It only hides it.
void CShipEditorDlg::OnClose()
{
	if (verify() && (!bypass_errors)) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		bypass_errors = 0;
		return;
	}

	if (update_data()) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		bypass_errors = 0;
		return;
	}

	SetWindowPos(Fred_main_wnd, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
	Fred_main_wnd->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

BOOL CShipEditorDlg::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL r;

	r = CDialog::Create(IDD, pParentWnd);
	return r;
}

int CShipEditorDlg::tristate_set(int val, int cur_state)
{
	if (val) {
		if (!cur_state){
			return 2;
		}

	} else {
		if (cur_state){
			return 2;
		}
	}

	return cur_state;
}

// called to initialize the dialog box to reflect what ships we currently have marked.  Any
// time what we have marked changes, this should get called again.
//
// Notes: player_count is the number of player starts marked, when we are in a non-multiplayer
// mission (NMM).  In a multiplayer mission (MM), player_count will always be zero.
// ship_count in NMM is the number of ships (i.e. not player starts) that are marked.  In MM,
// ship_count is the number of ships and player starts.  Total_count is the sum of ship_count
// and player_count in all cases.  The reason player_count isn't used in MM, and ship_count
// is used instead to track player starts is because in MM, player starts can be edited as
// freely as AI ships, and are very likely to be AI ships sometimes.  Thus, treating them like
// AI ships instead of player starts simplifies processing.
//
void CShipEditorDlg::initialize_data(int full_update)
{
	int i, type, ship_count, player_count, total_count, wing = -1, pvalid_count;
	int a_cue, d_cue, cue_init = 0, cargo = 0, base_ship, base_player, pship = -1;
	int no_arrival_warp = 0, no_departure_warp = 0, escort_count, ship_orders, current_orders;
	int pship_count;  // a total count of the player ships not marked
	object *objp;
	CWnd *w = NULL;
	CString str;
	CComboBox *box;
	CSingleLock sync(&CS_update);

	nprintf(("Fred routing", "Ship dialog load\n"));
	if (!GetSafeHwnd() || bypass_all)
		return;

	sync.Lock();  // don't initialize if we are still updating.  Wait until update is done.

	box = (CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET);
	management_add_ships_to_combo( box, SHIPS_2_COMBO_SPECIAL | SHIPS_2_COMBO_ALL_SHIPS );

	box = (CComboBox *)GetDlgItem(IDC_DEPARTURE_TARGET);
	management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );

	if (The_mission.game_type & MISSION_TYPE_MULTI){
		mission_type = 0;  // multi player mission
	} else {
		mission_type = 1;  // non-multiplayer mission (implies single player mission I guess)
	}

	// figure out what all we are editing.
	ship_count = player_count = escort_count = pship_count = pvalid_count = 0;
	base_ship = base_player = -1;
	enable = p_enable = 1;
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_SHIP) && (Ships[objp->instance].flags & SF_ESCORT)){
			escort_count++;  // get a total count of escort ships
		}

		if (objp->type == OBJ_START){
			pship_count++;  // count all player starts in mission
		}

		if (objp->flags & OF_MARKED) {
			type = objp->type;
			if ((type == OBJ_START) && !mission_type){  // in multiplayer missions, starts act like ships
				type = OBJ_SHIP;
			}

			i = -1;
			if (type == OBJ_START) {
				player_count++;
				// if player_count is 1, base_player will be the one and only player
				i = base_player = objp->instance;

			} else if (type == OBJ_SHIP) {
				ship_count++;
				// if ship_count is 1, base_ship will be the one and only ship
				i = base_ship = objp->instance;
			}

			if (i >= 0){
				if (Ship_info[Ships[i].ship_info_index].flags & SIF_PLAYER_SHIP){
					pvalid_count++;
				}
			}
		}

		objp = GET_NEXT(objp);
	}

	total_count = ship_count + player_count;  // get total number of objects being edited.
	if (total_count > 1){
		multi_edit = 1;
	} else {
		multi_edit = 0;
	}

	a_cue = d_cue = -1;
	m_arrival_location = -1;
	m_arrival_dist.blank();
	m_arrival_target = -1;
	m_arrival_delay.blank();
	m_departure_location = -1;
	m_departure_target = -1;
	m_departure_delay.blank();

	player_ship = single_ship = -1;
	m_arrival_tree.select_sexp_node = m_departure_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;
	ship_orders = 0;				// assume they are all the same type
	if (ship_count) {
		box = (CComboBox *) GetDlgItem(IDC_SHIP_CARGO1);
		box->ResetContent();
		for (i=0; i<Num_cargo; i++){
			box->AddString(Cargo_names[i]);
		}
		
		if (!multi_edit) {
			Assert((ship_count == 1) && (base_ship >= 0));
			m_ship_name = Ships[base_ship].ship_name;			
		} else {
			m_ship_name = _T("");
		}

		m_update_arrival = m_update_departure = 1;
		base_player = 0;
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
				if (objp->flags & OF_MARKED) {
					// do processing for both ships and players
					i = get_ship_from_obj(objp);
					if (base_player >= 0) {
						m_ship_class = Ships[i].ship_info_index;
						m_team = Ships[i].team;
						pship = (objp->type == OBJ_START) ? 1 : 0;
						base_player = -1;

					} else {
						if (Ships[i].ship_info_index != m_ship_class)
							m_ship_class = -1;
						if (Ships[i].team != m_team)
							m_team = -1;

						pship = tristate_set(Objects[Ships[i].objnum].type == OBJ_START, pship);
					}

					// 'and' in the ship type of this ship to our running bitfield
					current_orders = ship_get_default_orders_accepted( &Ship_info[Ships[i].ship_info_index] );
					if (!ship_orders){
						ship_orders = current_orders;
					} else if (ship_orders != current_orders){
						ship_orders = -1;
					}

					if (Ships[i].flags & SF_ESCORT){
						escort_count--;  // remove marked escorts from count
					}

					if (Objects[Ships[i].objnum].type == OBJ_START){
						pship_count--;  // removed marked starts from count
					}

					// do processing only for ships (plus players if in a multiplayer mission
					if ((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !mission_type)) {
						// process this if ship not in a wing
						if (Ships[i].wingnum < 0) {
							if (!cue_init) {
								cue_init = 1;
								a_cue = Ships[i].arrival_cue;
								d_cue = Ships[i].departure_cue;
								m_arrival_location = Ships[i].arrival_location;
								m_arrival_dist.init(Ships[i].arrival_distance);
								m_arrival_target = Ships[i].arrival_anchor;
								m_arrival_delay.init(Ships[i].arrival_delay);
								m_departure_location = Ships[i].departure_location;
								m_departure_delay.init(Ships[i].departure_delay);
								m_departure_target = Ships[i].departure_anchor;

							} else {
								cue_init++;
								if (Ships[i].arrival_location != m_arrival_location){
									m_arrival_location = -1;
								}

								if (Ships[i].departure_location != m_departure_location){
									m_departure_location = -1;
								}

								m_arrival_dist.set(Ships[i].arrival_distance);
								m_arrival_delay.set(Ships[i].arrival_delay);
								m_departure_delay.set(Ships[i].departure_delay);

								if (Ships[i].arrival_anchor != m_arrival_target){
									m_arrival_target = -1;
								}

								if (!cmp_sexp_chains(a_cue, Ships[i].arrival_cue)) {
									a_cue = -1;
									m_update_arrival = 0;
								}

								if (!cmp_sexp_chains(d_cue, Ships[i].departure_cue)) {
									d_cue = -1;
									m_update_departure = 0;
								}

								if ( Ships[i].departure_anchor != m_departure_target ){
									m_departure_target = -1;
								}
							}
						}

						// process the first ship in group, else process the rest
						if (base_ship >= 0) {
							m_ai_class = Ships[i].weapons.ai_class;
							cargo = Ships[i].cargo1;
							m_cargo1 = Cargo_names[cargo];
							m_hotkey = Ships[i].hotkey + 1;
							m_score.init(Ships[i].score);
							m_assist_score.init((int)(Ships[i].assist_score_pct*100));

							m_persona = Ships[i].persona_index + 1;

							// we use final_death_time member of ship structure for holding the amount of time before a mission
							// to destroy this ship
							wing = Ships[i].wingnum;
							if (wing < 0) {
								GetDlgItem(IDC_WING) -> SetWindowText("None");

							} else {
								GetDlgItem(IDC_WING) -> SetWindowText(Wings[wing].name);
								if (!query_whole_wing_marked(wing))
									m_update_arrival = m_update_departure = 0;
							}

							// set routine local varaiables for ship/object flags
							no_arrival_warp = (Ships[i].flags & SF_NO_ARRIVAL_WARP) ? 1 : 0;
							no_departure_warp = (Ships[i].flags & SF_NO_DEPARTURE_WARP) ? 1 : 0;

							base_ship = -1;
							if (!multi_edit)
								single_ship = i;

						} else {
							if (Ships[i].weapons.ai_class != m_ai_class){
								m_ai_class = -1;
							}

							if (Ships[i].cargo1 != cargo){
								m_cargo1 = _T("");
							}

							m_score.set(Ships[i].score);
							m_assist_score.set((int)(Ships[i].assist_score_pct*100));

							if (Ships[i].hotkey != m_hotkey - 1){
								m_hotkey = -1;
							}

							if ( Ships[i].persona_index != (m_persona-1) ){
								m_persona = -1;
							}
							
							if (Ships[i].wingnum != wing){
								GetDlgItem(IDC_WING) -> SetWindowText("");
							}

							no_arrival_warp = tristate_set(Ships[i].flags & SF_NO_ARRIVAL_WARP, no_arrival_warp);
							no_departure_warp = tristate_set(Ships[i].flags & SF_NO_DEPARTURE_WARP, no_departure_warp);
						}
					}
				}
			}

			objp = GET_NEXT(objp);
		}

		if (multi_edit) {
			m_arrival_tree.clear_tree("");
			m_departure_tree.clear_tree("");
		}

		if (cue_init) {
			m_arrival_tree.load_tree(a_cue);
			m_departure_tree.load_tree(d_cue, "false");

		} else {
			m_arrival_tree.clear_tree();
			m_arrival_tree.DeleteAllItems();
			m_departure_tree.clear_tree();
			m_departure_tree.DeleteAllItems();
		}

		m_player_ship.SetCheck(pship);
		m_no_arrival_warp.SetCheck(no_arrival_warp);
		m_no_departure_warp.SetCheck(no_departure_warp);

		if (!multi_edit) {
			i = m_arrival_tree.select_sexp_node;
			if (i != -1) {
				w = GetDlgItem(IDC_ARRIVAL_TREE);
				m_arrival_tree.hilite_item(i);

			} else {
				i = m_departure_tree.select_sexp_node;
				if (i != -1) {
					w = GetDlgItem(IDC_DEPARTURE_TREE);
					m_departure_tree.hilite_item(i);
				}
			}
		}

	} else {  // no ships selected, 0 or more player ships selected
		if (player_count > 1) {  // multiple player ships selected
			Assert(base_player >= 0);
			m_ship_name = _T("");
			m_player_ship.SetCheck(TRUE);
			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if ((objp->type == OBJ_START) && (objp->flags & OF_MARKED)) {
					i = objp->instance;
					if (base_player >= 0) {
						m_ship_class = Ships[i].ship_info_index;
						m_team = Ships[i].team;
						base_player = -1;

					} else {
						if (Ships[i].ship_info_index != m_ship_class)
							m_ship_class = -1;
						if (Ships[i].team != m_team)
							m_team = -1;
					}
				}

				objp = GET_NEXT(objp);
			}

		// only 1 player selected..
		} else if (query_valid_object() && (Objects[cur_object_index].type == OBJ_START)) {
			Assert((player_count == 1) && !multi_edit);
			player_ship = Objects[cur_object_index].instance;
			m_ship_name = Ships[player_ship].ship_name;
			m_ship_class = Ships[player_ship].ship_info_index;
			m_team = Ships[player_ship].team;
			m_player_ship.SetCheck(TRUE);

		} else {  // no ships or players selected..
			m_ship_name = _T("");
			m_ship_class = -1;
			m_team = -1;
			m_persona = -1;
			m_player_ship.SetCheck(FALSE);
		}

		m_ai_class = -1;
		m_cargo1 = _T("");
		m_hotkey = 0;
		m_score.blank();  // cause control to be blank
		m_assist_score.blank(); 
		m_arrival_location = -1;
		m_departure_location = -1;
		m_arrival_delay.blank();
		m_departure_delay.blank();
		m_arrival_dist.blank();
		m_arrival_target = -1;
		m_departure_target = -1;
		m_arrival_tree.clear_tree();
		m_arrival_tree.DeleteAllItems();
		m_departure_tree.clear_tree();
		m_departure_tree.DeleteAllItems();
		m_no_arrival_warp.SetCheck(0);
		m_no_departure_warp.SetCheck(0);
		enable = p_enable = 0;
		GetDlgItem(IDC_WING)->SetWindowText(_T("None"));
	}

	box = (CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET);
	// must put the appropriate ships into the list depending on arrival location
	if ( m_arrival_location != ARRIVE_FROM_DOCK_BAY ){
		management_add_ships_to_combo( box, SHIPS_2_COMBO_SPECIAL | SHIPS_2_COMBO_ALL_SHIPS );
	} else {
		management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
	}

	// set the internal variable appropriatly
	// Goober5000 - gah, this is ridiculous!  Prior to this point in the function (and only in this function),
	// m_arrival_target seems to refer to the arrival anchor.  The rest of the time, it refers to the index
	// of the drop-down list.
	if (m_arrival_target >= 0)
	{
		if (m_arrival_target & SPECIAL_ARRIVAL_ANCHOR_FLAG)
		{
			// figure out what the box represents this as
			char tmp[NAME_LENGTH + 15];
			stuff_special_arrival_anchor_name(tmp, m_arrival_target, 0);

			// find it in the box
			m_arrival_target = box->FindStringExact(-1, tmp);
		}
		else
		{
			// find it in the box
			m_arrival_target = box->FindStringExact(-1, Ships[m_arrival_target].ship_name);
		}
	}

	box = (CComboBox *)GetDlgItem(IDC_DEPARTURE_TARGET);
	// must put the appropriate ships into the list depending on departure location
	if ( m_departure_location == DEPART_AT_DOCK_BAY ){
		management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
	} else {
		box->ResetContent();
	}

	if ( m_departure_target >= 0 ){
		m_departure_target = box->FindStringExact( -1, Ships[m_departure_target].ship_name );
	}

	initialized = 1;
	if (player_count) {
		box = (CComboBox *) GetDlgItem(IDC_SHIP_TEAM);
		if (!mission_type){  // multiplayer mission
			box->EnableWindow(TRUE);
		}

		else {
			box->EnableWindow(FALSE);
			m_team = -1;
		}

		box->ResetContent();
		for (i=0; i<MAX_TVT_TEAMS; i++)
			box->AddString(Iff_info[i].iff_name);
	} else {
		box = (CComboBox *) GetDlgItem(IDC_SHIP_TEAM);
		box->EnableWindow(enable);
		box->ResetContent();
		for (i=0; i<Num_iffs; i++){
			box->AddString(Iff_info[i].iff_name);
		}
	}	

	m_score.display();
	m_assist_score.display();
	m_arrival_dist.display();
	m_arrival_delay.display();
	m_departure_delay.display();

	if (full_update)
		UpdateData(FALSE);

	if (!cue_init) {
		GetDlgItem(IDC_ARRIVAL_LOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_DELAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_DELAY_SPIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_TREE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEPARTURE_LOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEPARTURE_DELAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEPARTURE_DELAY_SPIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEPARTURE_TREE)->EnableWindow(FALSE);
		GetDlgItem(IDC_NO_ARRIVAL_WARP)->EnableWindow(FALSE);
		GetDlgItem(IDC_NO_DEPARTURE_WARP)->EnableWindow(FALSE);

		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(FALSE);

	} else {
		GetDlgItem(IDC_ARRIVAL_LOCATION)->EnableWindow(enable);
		if (m_arrival_location) {
			GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(enable);
			GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(enable);
		} else {
			GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(FALSE);
		}
		if (m_arrival_location == ARRIVE_FROM_DOCK_BAY) {
			GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(enable);
		} else {
			GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(FALSE);
		}

		GetDlgItem(IDC_DEPARTURE_LOCATION)->EnableWindow(enable);
		if ( m_departure_location ) {
			GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(enable);
		} else {
			GetDlgItem(IDC_DEPARTURE_TARGET)->EnableWindow(FALSE);
		}
		if (m_departure_location == DEPART_AT_DOCK_BAY) {
			GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(enable);
		} else {
			GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(FALSE);
		}

		GetDlgItem(IDC_ARRIVAL_DELAY)->EnableWindow(enable);
		GetDlgItem(IDC_ARRIVAL_DELAY_SPIN)->EnableWindow(enable);
		GetDlgItem(IDC_ARRIVAL_TREE)->EnableWindow(enable);
		GetDlgItem(IDC_DEPARTURE_LOCATION)->EnableWindow(enable);
		GetDlgItem(IDC_DEPARTURE_DELAY)->EnableWindow(enable);
		GetDlgItem(IDC_DEPARTURE_DELAY_SPIN)->EnableWindow(enable);
		GetDlgItem(IDC_DEPARTURE_TREE)->EnableWindow(enable);
		GetDlgItem(IDC_NO_ARRIVAL_WARP)->EnableWindow(enable);
		GetDlgItem(IDC_NO_DEPARTURE_WARP)->EnableWindow(enable);
	}

	if (total_count) {
		GetDlgItem(IDC_SHIP_NAME)->EnableWindow(!multi_edit);
		GetDlgItem(IDC_SHIP_CLASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_SHIP_ALT)->EnableWindow(TRUE);
		GetDlgItem(IDC_INITIAL_STATUS)->EnableWindow(TRUE);
		GetDlgItem(IDC_WEAPONS)->EnableWindow(m_ship_class >= 0);
		GetDlgItem(IDC_FLAGS)->EnableWindow(TRUE);
		GetDlgItem(IDC_TEXTURES)->EnableWindow(TRUE);
		GetDlgItem(IDC_ALT_SHIP_CLASS)->EnableWindow(TRUE);	
		GetDlgItem(IDC_SPECIAL_EXP)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPECIAL_HITPOINTS)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_SHIP_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_SHIP_CLASS)->EnableWindow(FALSE);
		GetDlgItem(IDC_SHIP_ALT)->EnableWindow(FALSE);
		GetDlgItem(IDC_INITIAL_STATUS)->EnableWindow(FALSE);
		GetDlgItem(IDC_WEAPONS)->EnableWindow(FALSE);
		GetDlgItem(IDC_FLAGS)->EnableWindow(FALSE);
		GetDlgItem(IDC_TEXTURES)->EnableWindow(FALSE);
		GetDlgItem(IDC_ALT_SHIP_CLASS)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIAL_EXP)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIAL_HITPOINTS)->EnableWindow(FALSE);
	}

	// disable textures for multiple ships
	if (multi_edit)
	{
		GetDlgItem(IDC_TEXTURES)->EnableWindow(FALSE);
	}

	GetDlgItem(IDC_AI_CLASS)->EnableWindow(enable);
	GetDlgItem(IDC_SHIP_CARGO1)->EnableWindow(enable);
	GetDlgItem(IDC_HOTKEY)->EnableWindow(enable);
	if ((m_ship_class >= 0) && !(Ship_info[m_ship_class].flags & SIF_CARGO) && !(Ship_info[m_ship_class].flags & SIF_NO_SHIP_TYPE))
		GetDlgItem(IDC_GOALS)->EnableWindow(enable);
	else if (multi_edit)
		GetDlgItem(IDC_GOALS)->EnableWindow(enable);
	else
		GetDlgItem(IDC_GOALS)->EnableWindow(FALSE);

	// !pship_count used because if allowed to clear, we would have no player starts
	// mission_type 0 = multi, 1 = single
	if (mission_type || !pship_count || (pship_count + total_count > MAX_PLAYERS) || (pvalid_count < total_count))
		m_player_ship.EnableWindow(FALSE);
	else
		m_player_ship.EnableWindow(TRUE);

	// show the "set player" button only if single player
	if (mission_type)
		GetDlgItem(IDC_SET_AS_PLAYER_SHIP)->ShowWindow(TRUE);
	else
		GetDlgItem(IDC_SET_AS_PLAYER_SHIP)->ShowWindow(FALSE);

	// enable the "set player" button only if single player, single edit, and ship is in player wing
	{
		int marked_ship = (player_ship >= 0) ? player_ship : single_ship;

		if (mission_type && total_count && !multi_edit && wing_is_player_wing(Ships[marked_ship].wingnum))
			GetDlgItem(IDC_SET_AS_PLAYER_SHIP)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_SET_AS_PLAYER_SHIP)->EnableWindow(FALSE);
	}

	GetDlgItem(IDC_DELETE_SHIP)->EnableWindow(enable);
	GetDlgItem(IDC_SHIP_RESET)->EnableWindow(enable);
	GetDlgItem(IDC_SCORE)->EnableWindow(enable);
	GetDlgItem(IDC_ASSIST_SCORE)->EnableWindow(enable);

//#ifndef NDEBUG
	GetDlgItem(IDC_SHIP_TBL)->EnableWindow(m_ship_class >= 0);
//#else
//	GetDlgItem(IDC_SHIP_TBL)->EnableWindow(0);
//	GetDlgItem(IDC_SHIP_TBL)->ShowWindow(SW_HIDE);
//#endif

	if (cue_init > 1) {  // more than one ship (players don't have cues to edit)
		GetDlgItem(IDC_UPDATE_ARRIVAL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_UPDATE_DEPARTURE)->ShowWindow(SW_SHOW);

	} else {
		GetDlgItem(IDC_UPDATE_ARRIVAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPDATE_DEPARTURE)->ShowWindow(SW_HIDE);
	}

	if (multi_edit || (total_count > 1)) {
		// we will allow the ignore orders dialog to be multi edit if all selected
		// ships are the same type.  the ship_type (local) variable holds the ship types
		// for all ships.  Determine how may bits set and enable/diable window
		// as appropriate
		if ( /*(m_team == -1) ||*/ (ship_orders == -1) ){
			GetDlgItem(IDC_IGNORE_ORDERS)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_IGNORE_ORDERS)->EnableWindow(TRUE);
		}
	} else
		// always enabled when one ship is selected
		GetDlgItem(IDC_IGNORE_ORDERS)->EnableWindow(enable);

	// always enabled if >= 1 ship selected
	GetDlgItem(IDC_SHIP_PERSONA)->EnableWindow(enable);	

	if (multi_edit){
		SetWindowText("Edit Marked Ships");
	} else if (player_count) {
		SetWindowText("Edit Player Ship");
	} else {
		SetWindowText("Edit Ship");
	}

	// setup alternate name and callsign stuff
	if(player_ship >= 0){				
		ship_alt_name_init(player_ship);
		ship_callsign_init(player_ship);
	} else {				
		ship_alt_name_init(single_ship);
		ship_callsign_init(single_ship);
	}

	modified = 0;
	if (w){
		w->SetFocus();
	}
}

// update ship structure(s) with dialog data.  The data is first checked for errors.  If
// no errors occur, returns 0.  If an error occurs, returns -1.  If the update is bypassed,
// returns 1.  Bypass is necessary to avoid an infinite loop, and it doesn't actually
// update the data.  Bypass only occurs if bypass mode is active and we still get an error.
// Once the error no longer occurs, bypass mode is cleared and data is updated.
int CShipEditorDlg::update_data(int redraw)
{
	char *str, old_name[255];
	object *ptr;
	int i, z, wing;
	CSingleLock sync(&CS_cur_object_index), sync2(&CS_update);

	nprintf(("Fred routing", "Ship dialog save\n"));
	if (!GetSafeHwnd() || !initialized || bypass_all)
		return 0;

	sync.Lock();  // don't allow cur_object_index to change while we are using it
	sync2.Lock();  // don't allow reinitialization until we are done updating
	UpdateData(TRUE);
	UpdateData(TRUE);
	Wing_editor_dialog.update_data_safe();
	if (multi_edit) {  // editing multiple ships (ships and/or players)
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) && (ptr->flags & OF_MARKED))
				update_ship(get_ship_from_obj(ptr));

			ptr = GET_NEXT(ptr);
		}

	} else if (player_ship >= 0) {  // editing a single player
		update_ship(player_ship);

	} else if (single_ship >= 0) {  // editing a single ship
		m_ship_name.TrimLeft(); 
		m_ship_name.TrimRight(); 
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (cur_object_index != OBJ_INDEX(ptr))) {
				str = Ships[ptr->instance].ship_name;
				if (!stricmp(m_ship_name, str)) {
					if (bypass_errors)
						return 1;

					bypass_errors = 1;
					z = MessageBox("This ship name is already being used by another ship\n"
						"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

					if (z == IDCANCEL)
						return -1;

					m_ship_name = _T(Ships[single_ship].ship_name);
					UpdateData(FALSE);
				}
			}

			ptr = GET_NEXT(ptr);
		}

		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count && !stricmp(Wings[i].name, m_ship_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This ship name is already being used by a wing\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_ship_name = _T(Ships[single_ship].ship_name);
				UpdateData(FALSE);
			}
		}

		for (i=0; i<Num_iffs; i++) {
			if (!stricmp(m_ship_name, Iff_info[i].iff_name)) {
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This ship name is already being used by a team.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_ship_name = _T(Ships[single_ship].ship_name);
				UpdateData(FALSE);
			}
		}

		for ( i=0; i < (int)Ai_tp_list.size(); i++) {
			if (!stricmp(m_ship_name, Ai_tp_list[i].name)) 
			{
				if (bypass_errors)
					return 1;

				bypass_errors = 1;
				z = MessageBox("This ship name is already being used by a target priority group.\n"
					"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

				if (z == IDCANCEL)
					return -1;

				m_ship_name = _T(Ships[single_ship].ship_name);
				UpdateData(FALSE);
			}
		}

		if (find_matching_waypoint_list(const_cast<char*>((const char *) m_ship_name)) != NULL)
		{
			if (bypass_errors)
				return 0;

			bypass_errors = 1;
			z = MessageBox("This ship name is already being used by a waypoint path\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_ship_name = _T(Ships[single_ship].ship_name);
			UpdateData(FALSE);
		}

		if(jumpnode_get_by_name(m_ship_name) != NULL)
		{
			if (bypass_errors)
				return 1;

			bypass_errors = 1;

			z = MessageBox("This ship name is already being used by a jump node\n"
			"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
			return -1;

			m_ship_name = _T(Ships[single_ship].ship_name);
			UpdateData(FALSE);
		}
		
		if (!stricmp(m_ship_name.Left(1), "<")) {
			if (bypass_errors)
				return 1;

			bypass_errors = 1;
			z = MessageBox("Ship names not allowed to begin with <\n"
				"Press OK to restore old name", "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);

			if (z == IDCANCEL)
				return -1;

			m_ship_name = _T(Ships[single_ship].ship_name);
			UpdateData(FALSE);
		}

		wing = Ships[single_ship].wingnum;
		if (wing >= 0) {
			Assert((wing < MAX_WINGS) && Wings[wing].wave_count);
			for (i=0; i<Wings[wing].wave_count; i++)
				if (wing_objects[wing][i] == Ships[single_ship].objnum)
					break;

			Assert(i < Wings[wing].wave_count);
			sprintf(old_name, "%s %d", Wings[wing].name, i + 1);
			if (strcmp(old_name, m_ship_name)) {
				if (bypass_errors)
					return 0;

				if (MessageBox("This ship is part of a wing, and its name cannot be changed",
					NULL, MB_OKCANCEL) == IDCANCEL)
						return -1;

				m_ship_name = _T(old_name);
				UpdateData(FALSE);
			}
		}

		z = update_ship(single_ship);
		if (z)
			return z;

		strcpy_s(old_name, Ships[single_ship].ship_name);
		string_copy(Ships[single_ship].ship_name, m_ship_name, NAME_LENGTH, 1);
		str = Ships[single_ship].ship_name;
		if (strcmp(old_name, str)) {
			update_sexp_references(old_name, str);
			ai_update_goal_references(REF_TYPE_SHIP, old_name, str);
			update_texture_replacements(old_name, str);
			for (i=0; i<Num_reinforcements; i++)
				if (!strcmp(old_name, Reinforcements[i].name)) {
					Assert(strlen(str) < NAME_LENGTH);
					strcpy_s(Reinforcements[i].name, str);
				}

			Update_window = 1;
		}
	}

	if (Player_start_shipnum < 0 || Objects[Ships[Player_start_shipnum].objnum].type != OBJ_START) {  // need a new single player start.
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_START) {
				Player_start_shipnum = ptr->instance;
				break;
			}

			ptr = GET_NEXT(ptr);
		}
	}

	if (modified)
		set_modified();

	Wing_editor_dialog.initialize_data_safe(1);
	bypass_errors = 0;
	modified = 0;

	if (redraw)
		update_map_window();

	return 0;
}

int CShipEditorDlg::update_ship(int ship)
{
	int z, d;
	CString str;
	CComboBox *box;
	int persona;

	// THIS DIALOG IS THE SOME OF THE WORST CODE I HAVE EVER SEEN IN MY ENTIRE LIFE. 
	// IT TOOK A RIDICULOUSLY LONG AMOUNT OF TIME TO ADD 2 FUNCTIONS. OMG
	ship_alt_name_close(ship);
	ship_callsign_close(ship);

	if ((Ships[ship].ship_info_index != m_ship_class) && (m_ship_class != -1)) {
		change_ship_type(ship, m_ship_class);
		set_modified();
	}

	if (m_team != -1)
		MODIFY(Ships[ship].team, m_team);

	if (Objects[Ships[ship].objnum].type != OBJ_SHIP){
		if (mission_type || (Objects[Ships[ship].objnum].type != OBJ_START)){
			return 0;
		}
	}

	if (m_ai_class != -1){
		MODIFY(Ships[ship].weapons.ai_class, m_ai_class);
	}
	if (strlen(m_cargo1)) {
		z = string_lookup(m_cargo1, Cargo_names, Num_cargo);
		if (z == -1) {
			if (Num_cargo < MAX_CARGO) {
				z = Num_cargo++;
				strcpy(Cargo_names[z], m_cargo1);
			}
			else {
				str.Format("Maximum number of cargo names (%d) reached.\nIgnoring new name.\n", MAX_CARGO);
				MessageBox(str, "Error", MB_ICONEXCLAMATION);
				z = 0;
				m_cargo1 = Cargo_names[z];
			}
		}

		MODIFY(Ships[ship].cargo1, (char)z);
	}

	m_score.save(&Ships[ship].score);
	int temp_assist = -1;
	m_assist_score.save(&temp_assist); 
	if (temp_assist != -1) {
		Ships[ship].assist_score_pct = ((float)temp_assist)/100;
		// value must be a percentage
		if (Ships[ship].assist_score_pct < 0) {
			Ships[ship].assist_score_pct = 0;
			MessageBox("Assist Percentage too low. Set to 0. No score will be granted for an assist");
		} 
		else if (Ships[ship].assist_score_pct > 1) {
			Ships[ship].assist_score_pct = 1;
			MessageBox("Assist Percentage too high. Set to 1. Assists will score as many points as a kill");	
		}
	}

	if (m_arrival_location != -1)
		MODIFY(Ships[ship].arrival_location, m_arrival_location);
	if (m_departure_location != -1)
		MODIFY(Ships[ship].departure_location, m_departure_location);

	if (m_persona != -1)
	{
		// do the persona update
		// m_persona holds the index into the list.  Get the item data associated with this index and then
		// assign to the ship taking care that we check for the NO_PERSONA_INDEX id
		box = (CComboBox *)GetDlgItem(IDC_SHIP_PERSONA);
		persona = box->GetItemData(m_persona);
		if ( persona == NO_PERSONA_INDEX )
			persona = -1;

		MODIFY(Ships[ship].persona_index, persona);
	}

	if (Ships[ship].wingnum < 0) {
		if (!multi_edit || m_update_arrival) {  // should we update the arrival cue?
			if (Ships[ship].arrival_cue >= 0)
				free_sexp2(Ships[ship].arrival_cue);

			Ships[ship].arrival_cue = m_arrival_tree.save_tree();
		}

		if (!multi_edit || m_update_departure) {
			if (Ships[ship].departure_cue >= 0)
				free_sexp2(Ships[ship].departure_cue);

			Ships[ship].departure_cue = m_departure_tree.save_tree();
		}

		m_arrival_dist.save(&Ships[ship].arrival_distance);
		m_arrival_delay.save(&Ships[ship].arrival_delay);
		m_departure_delay.save(&Ships[ship].departure_delay);
		if (m_arrival_target >= 0) {
			z = ((CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET))->GetItemData(m_arrival_target);
			MODIFY(Ships[ship].arrival_anchor, z);

			// if the arrival is not hyperspace or docking bay -- force arrival distance to be
			// greater than 2*radius of target.
			if (((m_arrival_location != ARRIVE_FROM_DOCK_BAY) && (m_arrival_location != ARRIVE_AT_LOCATION)) && (z >= 0) && !(z & SPECIAL_ARRIVAL_ANCHOR_FLAG)) {
			d = int(min(500, 2.0f * Objects[Ships[ship].objnum].radius));
				if ((Ships[ship].arrival_distance < d) && (Ships[ship].arrival_distance > -d)) {
					str.Format("Ship must arrive at least %d meters away from target.\n"
						"Value has been reset to this.  Use with caution!\r\n"
						"Recommended distance is %d meters.\r\n", d, (int)(2.0f * Objects[Ships[ship].objnum].radius) );

					MessageBox(str);
					if (Ships[ship].arrival_distance < 0)
						Ships[ship].arrival_distance = -d;
					else
						Ships[ship].arrival_distance = d;

					m_arrival_dist.fix(Ships[ship].arrival_distance);
				}
			}
		}
		if (m_departure_target >= 0) {
			z = ((CComboBox *) GetDlgItem(IDC_DEPARTURE_TARGET))->GetItemData(m_departure_target);
			MODIFY(Ships[ship].departure_anchor, z );
		}
	}

	if (m_hotkey != -1)
		MODIFY(Ships[ship].hotkey, m_hotkey - 1);

	switch( m_no_arrival_warp.GetCheck() ) {
		case 0:
			if (Ships[ship].flags & SF_NO_ARRIVAL_WARP)
				set_modified();

			Ships[ship].flags &= ~SF_NO_ARRIVAL_WARP;
			break;

		case 1:
			if (!(Ships[ship].flags & SF_NO_ARRIVAL_WARP))
				set_modified();

			Ships[ship].flags |= SF_NO_ARRIVAL_WARP;
			break;
	}

	switch( m_no_departure_warp.GetCheck() ) {
		case 0:
			if (Ships[ship].flags & SF_NO_DEPARTURE_WARP)
				set_modified();

			Ships[ship].flags &= ~SF_NO_DEPARTURE_WARP;
			break;

		case 1:
			if (!(Ships[ship].flags & SF_NO_DEPARTURE_WARP))
				set_modified();

			Ships[ship].flags |= SF_NO_DEPARTURE_WARP;
			break;
	}

	switch (m_player_ship.GetCheck()) {
		case 1:
			if (Objects[Ships[ship].objnum].type != OBJ_START) {
				Player_starts++;
				set_modified();
			}

			Objects[Ships[ship].objnum].type = OBJ_START;
			break;

		case 0:
			if (Objects[Ships[ship].objnum].type == OBJ_START) {
				Player_starts--;
				set_modified();
			}

			Objects[Ships[ship].objnum].type = OBJ_SHIP;
			break;
	}	

	Update_ship = 1;
	return 0;
}

void CShipEditorDlg::OnOK()
{
	HWND h;
	CWnd *w;

	w = GetFocus();
	if (w) {
		h = w->m_hWnd;
		GetDlgItem(IDC_ARRIVAL_TREE)->SetFocus();
		::SetFocus(h);
	}
}

void CShipEditorDlg::OnInitMenu(CMenu *pMenu)
{
	CMenu *m;

	m = pMenu->GetSubMenu(0);
	clear_menu(m);
	generate_ship_popup_menu(m, ID_SHIP_MENU, MF_ENABLED, SHIP_FILTER_PLAYERS);
	if (cur_ship != -1)
		m->CheckMenuItem(ID_SHIP_MENU + cur_ship, MF_BYCOMMAND | MF_CHECKED);

	CWnd::OnInitMenu(pMenu);
}

BOOL CShipEditorDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id, ship;

	id = LOWORD(wParam);
	if (id >= ID_SHIP_MENU && id < ID_SHIP_MENU + MAX_SHIPS) {
		if (!update_data()) {
			ship = id - ID_SHIP_MENU;
			unmark_all();
			set_cur_object_index(Ships[ship].objnum);
			return 1;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CShipEditorDlg::OnRclickArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_arrival_tree.right_clicked();	
	*pResult = 0;
}

void CShipEditorDlg::OnRclickDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_departure_tree.right_clicked();
	*pResult = 0;
}

void CShipEditorDlg::OnBeginlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_arrival_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = editing = 1;

	} else
		*pResult = 1;
}

void CShipEditorDlg::OnBeginlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (m_departure_tree.edit_label(pTVDispInfo->item.hItem) == 1)	{
		*pResult = 0;
		modified = editing = 1;

	} else
		*pResult = 1;
}

void CShipEditorDlg::OnEndlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_arrival_tree.end_label_edit(pTVDispInfo->item);
	editing = 0;
}

void CShipEditorDlg::OnEndlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	*pResult = m_departure_tree.end_label_edit(pTVDispInfo->item);
	editing = 0;
}

int CShipEditorDlg::verify()
{
	nprintf(("Fred routing", "Ship dialog verify\n"));
	if (!GetSafeHwnd() || !modified)
		return 0;

	if (bypass_errors)
		return 1;

	return 0;
}

void CShipEditorDlg::OnGoals()
{
	ShipGoalsDlg dlg_goals;

	Assert(query_valid_object());
//	if (multi_edit)
//		dlg_goals.initialize_multi();
//
//	else {
//		Assert(single_ship != -1);
//		dlg_goals.self_ship = single_ship;
//		dlg_goals.initialize(Ai_info[Ships[single_ship].ai_index].goals);
//	}

	if (!multi_edit) {
		Assert(single_ship != -1);
		dlg_goals.self_ship = single_ship;
	}

	dlg_goals.DoModal();
	if (!multi_edit && !query_initial_orders_empty(Ai_info[Ships[single_ship].ai_index].goals))
		if ((Ships[single_ship].wingnum >= 0) && (query_initial_orders_conflict(Ships[single_ship].wingnum)))
			MessageBox("This ship's wing also has initial orders", "Possible conflict");
}

void CShipEditorDlg::OnSelchangeShipClass() 
{
	object *ptr;

	UpdateData(TRUE);
	UpdateData(TRUE);
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags & OF_MARKED))
			if (Ships[ptr->instance].ship_info_index != m_ship_class) {
				change_ship_type(ptr->instance, m_ship_class);
				set_modified();
			}

		ptr = GET_NEXT(ptr);
	}

	update_map_window();
}

void CShipEditorDlg::OnInitialStatus() 
{
	initial_status dlg;

	dlg.m_multi_edit = multi_edit;
	dlg.DoModal();
}

void CShipEditorDlg::OnWeapons() 
{
	int i, ship = -1;
	WeaponEditorDlg dlg;
	object *objp;
	CComboBox *box;

	dlg.m_multi_edit = multi_edit;
	dlg.DoModal();

	if (multi_edit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (objp->flags & OF_MARKED)
				if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) {
					i = objp->instance;
					if (ship) {
						if (Ships[i].weapons.ai_class != Ships[ship].weapons.ai_class)
							m_ai_class = -1;

					} else {
						ship = i;
						m_ai_class = Ships[i].weapons.ai_class;
					}
				}

			objp = GET_NEXT(objp);
		}

	} else {
		ship = single_ship;
		if (ship < 0)
			ship = player_ship;

		Assert(ship >= 0);
		m_ai_class = Ships[ship].weapons.ai_class;
	}

	box = (CComboBox *) GetDlgItem(IDC_AI_CLASS);
	box->SetCurSel(m_ai_class);
}

void CShipEditorDlg::OnShipReset() 
{
	int i, j, index, ship;
	object *objp;
	ship_info *sip;
	ship_subsys *ptr;
	ship_weapon *wp;
	model_subsystem *sp;

	m_cargo1 = "Nothing";
	m_ai_class = AI_DEFAULT_CLASS;
	if (m_ship_class) {
		m_team = Species_info[Ship_info[m_ship_class].species].default_iff;
	}

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !mission_type)) && (objp->flags & OF_MARKED)) {
			ship = objp->instance;

			// reset ship goals
			for (i=0; i<MAX_AI_GOALS; i++){
				Ai_info[Ships[ship].ai_index].goals[i].ai_mode = AI_GOAL_NONE;
			}

			objp->phys_info.speed = 0.0f;
			objp->shield_quadrant[0] = 100.0f;
			objp->hull_strength = 100.0f;

			sip = &Ship_info[Ships[ship].ship_info_index];
			for (i=0; i<sip->num_primary_banks; i++)
				Ships[ship].weapons.primary_bank_weapons[i] = sip->primary_bank_weapons[i];

			for (i=0; i<sip->num_secondary_banks; i++) {
				Ships[ship].weapons.secondary_bank_weapons[i] = sip->secondary_bank_weapons[i];
				Ships[ship].weapons.secondary_bank_capacity[i] = sip->secondary_bank_ammo_capacity[i];
			}

			index = 0;
			ptr = GET_FIRST(&Ships[ship].subsys_list);
			while (ptr != END_OF_LIST(&Ships[ship].subsys_list)) {
				ptr->current_hits = 0.0f;
				if (ptr->system_info->type == SUBSYSTEM_TURRET) {
					wp = &ptr->weapons;
					sp = &Ship_info[Ships[ship].ship_info_index].subsystems[index];

					j = 0;
					for (i=0; i<MAX_SHIP_PRIMARY_BANKS; i++){
						if (sp->primary_banks[i] != -1){
							wp->primary_bank_weapons[j++] = sp->primary_banks[i];
						}
					}

					wp->num_primary_banks = j;
					j = 0;
					for (i=0; i<MAX_SHIP_SECONDARY_BANKS; i++){
						if (sp->secondary_banks[i] != -1) {
							wp->secondary_bank_weapons[j] = sp->secondary_banks[i];
							wp->secondary_bank_capacity[j++] = sp->secondary_bank_capacity[i];
						}
					}

					wp->num_secondary_banks = j;
					for (i=0; i<MAX_SHIP_SECONDARY_BANKS; i++){
						wp->secondary_bank_ammo[i] = 100;
					}
				}

				index++;
				ptr = GET_NEXT(ptr);
			}
		}

		objp = GET_NEXT(objp);
	}

	UpdateData(FALSE);
	if (multi_edit){
		MessageBox("Ships reset to ship class defaults");
	} else {
		MessageBox("Ship reset to ship class defaults");
	}
}

void CShipEditorDlg::OnDeleteShip() 
{
	delete_marked();
	unmark_all();
}

void CShipEditorDlg::OnShipTbl()
{
	text_view_dlg dlg;

	dlg.set(m_ship_class);
	dlg.DoModal();
}

int CShipEditorDlg::make_ship_list(int *arr)
{
	int n = 0;
	object *ptr;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)){
			arr[n++] = OBJ_INDEX(ptr);
		}

		ptr = GET_NEXT(ptr);
	}

	return n;
}

void CShipEditorDlg::OnPrev() 
{
	int i, n, arr[MAX_SHIPS];

	if (!update_data()) {
		n = make_ship_list(arr);
		if (!n){
			return;
		}

		if (cur_ship < 0){
			i = n - 1;
		}

		else {
			for (i=0; i<n; i++){
				if (Ships[cur_ship].objnum == arr[i]){
					break;
				}
			}

			Assert(i < n);
			i--;
			if (i < 0){
				i = n - 1;
			}
		}

		unmark_all();
		set_cur_object_index(arr[i]);
		Ship_editor_dialog.initialize_data(1);
		Update_ship = 0;
	}

	return;
}

void CShipEditorDlg::OnNext() 
{
	int i, n, arr[MAX_SHIPS];

	if (!update_data()) {
		n = make_ship_list(arr);
		if (!n)
			return;

		if (cur_ship < 0)
			i = 0;

		else {
			for (i=0; i<n; i++)
				if (Ships[cur_ship].objnum == arr[i])
					break;

			Assert(i < n);
			i++;
			if (i == n)
				i = 0;
		}

		unmark_all();
		set_cur_object_index(arr[i]);
		Ship_editor_dialog.initialize_data(1);
		Update_ship = 0;
	}

	return;
}

void CShipEditorDlg::OnSelchangedArrivalTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM h;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	h = pNMTreeView->itemNew.hItem;
	if (h){
		m_arrival_tree.update_help(h);
	}

	*pResult = 0;
}

void CShipEditorDlg::OnSelchangedDepartureTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM h;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	h = pNMTreeView->itemNew.hItem;
	if (h){
		m_departure_tree.update_help(h);
	}

	*pResult = 0;
}

void CShipEditorDlg::calc_cue_height()
{
	CRect cue, help;

	GetDlgItem(IDC_CUE_FRAME)->GetWindowRect(cue);
	cue_height = (cue.bottom - cue.top)+20;
	if (Show_sexp_help){
		GetDlgItem(IDC_HELP_BOX)->GetWindowRect(help);
		cue_height += (help.bottom - help.top);
	}

	if (Hide_ship_cues) {
		((CButton *) GetDlgItem(IDC_HIDE_CUES)) -> SetCheck(1);
		OnHideCues();
	}
}

void CShipEditorDlg::show_hide_sexp_help()
{
	CRect rect, help;
	GetDlgItem(IDC_HELP_BOX)->GetWindowRect(help);
	float box_size = (float)(help.bottom - help.top);

	if (Show_sexp_help){
		cue_height += (int)box_size;
	} else {
		cue_height -= (int)box_size;
	}

	if (((CButton *) GetDlgItem(IDC_HIDE_CUES)) -> GetCheck()){
		return;
	}

	GetWindowRect(rect);

	if (Show_sexp_help){
		rect.bottom += (LONG)box_size;
	} else {
		rect.bottom -= (LONG)box_size;
	}

	MoveWindow(rect);
}

void CShipEditorDlg::OnHideCues() 
{
	CRect rect;

	GetWindowRect(rect);
	if (((CButton *) GetDlgItem(IDC_HIDE_CUES)) -> GetCheck()) {
		rect.bottom -= cue_height;
		Hide_ship_cues = 1;

	} else {
		rect.bottom += cue_height;
		Hide_ship_cues = 0;
	}

	MoveWindow(rect);
}

void CShipEditorDlg::OnSelchangeArrivalLocation() 
{
	CComboBox *box;

	UpdateData();
	box = (CComboBox *)GetDlgItem( IDC_ARRIVAL_TARGET );
	if (m_arrival_location) {
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(TRUE);
		if (m_arrival_target < 0) {
			m_arrival_target = 0;
		}

		// determine which items we should put into the arrival target combo box
		if ( m_arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
		} else {
			management_add_ships_to_combo( box, SHIPS_2_COMBO_SPECIAL | SHIPS_2_COMBO_ALL_SHIPS );
		}

	} else {
		m_arrival_target = -1;
		GetDlgItem(IDC_ARRIVAL_DISTANCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ARRIVAL_TARGET)->EnableWindow(FALSE);
	}

	if (m_arrival_location == ARRIVE_FROM_DOCK_BAY)	{
		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_RESTRICT_ARRIVAL)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}

void CShipEditorDlg::OnSelchangeDepartureLocation() 
{
	CComboBox *box;

	UpdateData();
	box = (CComboBox *)GetDlgItem(IDC_DEPARTURE_TARGET);
	if ( m_departure_location ) {
		box->EnableWindow(TRUE);
		if ( m_departure_target < 0 ) {
			m_departure_target = 0;
		}

		// we need to build up the list box content based on the departure type.  When
		// from a docking bay, only show ships in the list which have them.  Show all ships otherwise
		if ( m_departure_location == DEPART_AT_DOCK_BAY ) {
			management_add_ships_to_combo( box, SHIPS_2_COMBO_DOCKING_BAY_ONLY );
		} else {
			// I think that this section is currently illegal
			Int3();
		}

	} else {
		m_departure_target = -1;
		box->EnableWindow(FALSE);
	}

	if (m_departure_location == DEPART_AT_DOCK_BAY)	{
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_RESTRICT_DEPARTURE)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
}


void CShipEditorDlg::OnPlayerShip() 
{
	if (m_player_ship.GetCheck() == 1)
		m_player_ship.SetCheck(0);
	else
		m_player_ship.SetCheck(1);

	update_map_window();
}

void CShipEditorDlg::OnNoArrivalWarp() 
{
	if (m_no_arrival_warp.GetCheck() == 1)
		m_no_arrival_warp.SetCheck(0);
	else
		m_no_arrival_warp.SetCheck(1);
}

void CShipEditorDlg::OnNoDepartureWarp() 
{
	if (m_no_departure_warp.GetCheck() == 1)
		m_no_departure_warp.SetCheck(0);
	else
		m_no_departure_warp.SetCheck(1);
}


// function to possibly warn user when he selects a hotkey which might be used for
// a player wing.
void CShipEditorDlg::OnSelchangeHotkey() 
{
	int set_num;
	char buf[256];

	UpdateData(TRUE);
	set_num = m_hotkey-1;			// use -1 since values associated with hotkey sets are 1 index based

	// the first three sets are generally reserved for player starting wings.
	if ( set_num < MAX_STARTING_WINGS ) {
		sprintf( buf, "This hotkey set should probably be reserved\nfor wing %s", Starting_wing_names[set_num] );
		MessageBox(buf, NULL, MB_OK);
	}
}

void CShipEditorDlg::OnFlags() 
{
	ship_flags_dlg dlg;

	dlg.setup(p_enable);
	dlg.DoModal();
}

void CShipEditorDlg::OnIgnoreOrders() 
{
	// TODO: Add your control notification handler code here
	ignore_orders_dlg player_order_dlg;

	Assert(query_valid_object());

	if (!multi_edit) {
		if ( single_ship != -1 ){
			player_order_dlg.m_ship = single_ship;
		} else {
			player_order_dlg.m_ship = player_ship;
		}
	} else {
		player_order_dlg.m_ship = -1;
	}

	player_order_dlg.DoModal();
}

void CShipEditorDlg::OnSpecialExp() 
{
	// TODO: Add your control notification handler code here
	ShipSpecialDamage dlg;
	dlg.DoModal();
}

// alternate ship name stuff
void CShipEditorDlg::ship_alt_name_init(int base_ship)
{
	int idx;
	CComboBox *ptr = (CComboBox*)GetDlgItem(IDC_SHIP_ALT);
	if(ptr == NULL){
		Int3();
		return;
	}

	// multi-edit. bah	
	if(multi_edit){		
		GetDlgItem(IDC_SHIP_ALT)->EnableWindow(FALSE);
		return;
	} 
	GetDlgItem(IDC_SHIP_ALT)->EnableWindow(TRUE);	

	// reset the combobox and add all relevant strings
	ptr->ResetContent();
	ptr->AddString("<none>");
	for(idx=0; idx<Mission_alt_type_count; idx++){
		ptr->AddString(Mission_alt_types[idx]);
	}

	// "none"
	if(base_ship < 0){
		ptr->SetCurSel(0);
	}

	// otherwise look his stuff up
	if(strlen(Fred_alt_names[base_ship])){
		ptr->SelectString(0, Fred_alt_names[base_ship]);
	} else {
		ptr->SetCurSel(0);
	}
}

void CShipEditorDlg::ship_alt_name_close(int base_ship)
{
	CString cstr;
	char str[NAME_LENGTH+2] = "";
	char *p;	
	CComboBox *ptr = (CComboBox*)GetDlgItem(IDC_SHIP_ALT);

	if(multi_edit){
		return;
	}
	
	if(ptr == NULL){
		Int3();
		return;
	}

	// see if we have something besides "none" selected
	ptr->GetWindowText(cstr);
	if(cstr == CString("<none>")){
		// zero the entry
		strcpy_s(Fred_alt_names[base_ship], "");
		return;
	}	
	p = cstr.GetBuffer(0);
	if(p == NULL){
		return;
	}
	strcpy_s(str, p);

	// otherwise see if it already exists
	if(mission_parse_lookup_alt(str) >= 0){
		strcpy_s(Fred_alt_names[base_ship], str);
		return;
	}

	// otherwise try and add it
	if(mission_parse_add_alt(str) >= 0){
		strcpy_s(Fred_alt_names[base_ship], str);
		return;
	}

	// bad - couldn't add
	strcpy_s(Fred_alt_names[base_ship], "");
	MessageBox("Couldn't add new alternate type name. Already using too many!");
}

// callsign stuff
void CShipEditorDlg::ship_callsign_init(int base_ship)
{
	int idx;
	CComboBox *ptr = (CComboBox*)GetDlgItem(IDC_SHIP_CALLSIGN);
	if(ptr == NULL){
		Int3();
		return;
	}

	// multi-edit. bah	
	if(multi_edit){		
		GetDlgItem(IDC_SHIP_CALLSIGN)->EnableWindow(FALSE);
		return;
	} 
	GetDlgItem(IDC_SHIP_CALLSIGN)->EnableWindow(TRUE);	

	// reset the combobox and add all relevant strings
	ptr->ResetContent();
	ptr->AddString("<none>");
	for(idx=0; idx<Mission_callsign_count; idx++){
		ptr->AddString(Mission_callsigns[idx]);
	}

	// "none"
	if(base_ship < 0){
		ptr->SetCurSel(0);
	}

	// otherwise look his stuff up
	if(strlen(Fred_callsigns[base_ship])){
		ptr->SelectString(0, Fred_callsigns[base_ship]);
	} else {
		ptr->SetCurSel(0);
	}
}

void CShipEditorDlg::ship_callsign_close(int base_ship)
{
	CString cstr;
	char str[NAME_LENGTH+2] = "";
	char *p;	
	CComboBox *ptr = (CComboBox*)GetDlgItem(IDC_SHIP_CALLSIGN);

	if(multi_edit){
		return;
	}
	
	if(ptr == NULL){
		Int3();
		return;
	}

	// see if we have something besides "none" selected
	ptr->GetWindowText(cstr);
	if(cstr == CString("<none>")){
		// zero the entry
		strcpy_s(Fred_callsigns[base_ship], "");
		return;
	}	
	p = cstr.GetBuffer(0);
	if(p == NULL){
		return;
	}
	strcpy_s(str, p);

	// otherwise see if it already exists
	if(mission_parse_lookup_callsign(str) >= 0){
		strcpy_s(Fred_callsigns[base_ship], str);
		return;
	}

	// otherwise try and add it
	if(mission_parse_add_callsign(str) >= 0){
		strcpy_s(Fred_callsigns[base_ship], str);

		return;
	}

	// bad - couldn't add
	strcpy_s(Fred_callsigns[base_ship], "");
	MessageBox("Couldn't add new callsign. Already using too many!");
}

void CShipEditorDlg::OnTextures() 
{
	CShipTexturesDlg dlg_textures;

	Assert(query_valid_object());

	if (multi_edit)
	{
		MessageBox("Sorry, you can only edit textures for one ship at a time.", "Too many ships selected");
		return;
	}

	// get the ship that's marked
	int marked_ship = (player_ship >= 0) ? player_ship : single_ship;

	dlg_textures.self_ship = marked_ship;
	dlg_textures.DoModal();
}

void CShipEditorDlg::OnSpecialHitpoints() 
{
	ShipSpecialHitpoints dlg;
	dlg.DoModal();
}

void CShipEditorDlg::OnAltShipClass() 
{
	AltShipClassDlg dlg;
	dlg.DoModal();
}

// Goober5000
void CShipEditorDlg::OnSetAsPlayerShip() 
{
	Assert(query_valid_object());

	if (multi_edit)
	{
		MessageBox("Please select only one ship.", "Too many ships selected");
		return;
	}

	// since this is single player, clear all player ships and set only this one
	object *objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list))
	{
		if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))
		{
			if (objp->flags & OF_MARKED)	// there should only be one selected ship
			{
				// set as player ship
				objp->type = OBJ_START;
				objp->flags |= OF_PLAYER_SHIP;
			}
			else
			{
				// set as regular ship
				objp->type = OBJ_SHIP;
				objp->flags &= ~OF_PLAYER_SHIP;
			}
		}
		objp = GET_NEXT(objp);
	}

	// finally set editor dialog
	m_player_ship.SetCheck(1);
	update_map_window();
}

// Goober5000
void CShipEditorDlg::OnRestrictArrival()
{
	int arrive_from_ship;
	CComboBox *box;
	restrict_paths dlg;

	// grab stuff from GUI
	UpdateData(TRUE);

	if (m_arrival_location != ARRIVE_FROM_DOCK_BAY)
	{
		Int3();
		return;
	}

	box = (CComboBox *) GetDlgItem(IDC_ARRIVAL_TARGET);
	if (box->GetCount() == 0)
		return;

	arrive_from_ship = box->GetItemData(m_arrival_target);

	if (!ship_has_dock_bay(arrive_from_ship))
	{
		Int3();
		return;
	}

	// get the ship that's marked
	int marked_ship = (player_ship >= 0) ? player_ship : single_ship;

	dlg.m_arrival = true;
	dlg.m_ship_class = Ships[arrive_from_ship].ship_info_index;
	dlg.m_path_mask = &Ships[marked_ship].arrival_path_mask;

	dlg.DoModal();
}

// Goober5000
void CShipEditorDlg::OnRestrictDeparture()
{
	int depart_to_ship;
	CComboBox *box;
	restrict_paths dlg;

	// grab stuff from GUI
	UpdateData(TRUE);

	if (m_departure_location != DEPART_AT_DOCK_BAY)
	{
		Int3();
		return;
	}

	box = (CComboBox *) GetDlgItem(IDC_DEPARTURE_TARGET);
	if (box->GetCount() == 0)
		return;

	depart_to_ship = box->GetItemData(m_departure_target);

	if (!ship_has_dock_bay(depart_to_ship))
	{
		Int3();
		return;
	}

	// get the ship that's marked
	int marked_ship = (player_ship >= 0) ? player_ship : single_ship;

	dlg.m_arrival = false;
	dlg.m_ship_class = Ships[depart_to_ship].ship_info_index;
	dlg.m_path_mask = &Ships[marked_ship].departure_path_mask;

	dlg.DoModal();
}
