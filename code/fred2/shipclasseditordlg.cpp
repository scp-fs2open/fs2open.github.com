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
#include "ShipClassEditorDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShipClassEditorDlg dialog


CShipClassEditorDlg::CShipClassEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShipClassEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShipClassEditorDlg)
	m_ShipClassAfterburner = FALSE;
	m_ShipClassAIClass = -1;
	m_ShipClassArmor = _T("");
	m_ShipClassCloak = FALSE;
	m_ShipClassDebrisModel = -1;
	m_ShipClassModel = -1;
	m_ShipClassEngine = _T("");
	m_ShipClassExplosion1 = _T("");
	m_ShipClassExplosion2 = _T("");
	m_ShipClassIFF = _T("");
	m_ShipClassManufacturer = _T("");
	m_ShipClassMaxBank = 0;
	m_ShipClassMaxPitch = 0;
	m_ShipClassMaxRoll = 0;
	m_ShipClassMaxSpeed = 0;
	m_ShipClassName = _T("");
	m_ShipClassPowerPlant = _T("");
	m_ShipClassScore = 0;
	m_ShipClassShields = 0;
	m_ShipClassWarpdrive = FALSE;
	m_ShipClassTurretWeapon1 = _T("");
	m_ShipClassTurretWeapon2 = _T("");
	m_ShipClassWeaponSpecial = _T("");
	m_ShipClassWeapon1 = _T("");
	m_ShipClassWeapon2 = _T("");
	//}}AFX_DATA_INIT
}


void CShipClassEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShipClassEditorDlg)
	DDX_Control(pDX, IDC_SOUNDS, m_SoundsEditor);
	DDX_Control(pDX, IDC_SCLASS_WINDOW, m_ShipClassWindow);
	DDX_Control(pDX, IDC_SCLASS_NEW, m_ShipClassNew);
	DDX_Control(pDX, IDC_SCLASS_DELETE, m_ShipClassDelete);
	DDX_Control(pDX, IDC_MODELS, m_ModelsEditor);
	DDX_Control(pDX, IDC_GOALS, m_GoalsEditor);
	DDX_Check(pDX, IDC_SCLASS_AFTERBURNER, m_ShipClassAfterburner);
	DDX_CBIndex(pDX, IDC_SCLASS_AI_CLASS, m_ShipClassAIClass);
	DDX_CBString(pDX, IDC_SCLASS_ARMOR, m_ShipClassArmor);
	DDX_Check(pDX, IDC_SCLASS_CLOAK, m_ShipClassCloak);
	DDX_CBIndex(pDX, IDC_SCLASS_DEBRIS_MODEL, m_ShipClassDebrisModel);
	DDX_CBIndex(pDX, IDC_SCLASS_3D_OBJECT, m_ShipClassModel);
	DDX_Text(pDX, IDC_SCLASS_ENGINES, m_ShipClassEngine);
	DDX_CBString(pDX, IDC_SCLASS_EXPLOSION1, m_ShipClassExplosion1);
	DDX_CBString(pDX, IDC_SCLASS_EXPLOSION2, m_ShipClassExplosion2);
	DDX_CBString(pDX, IDC_SCLASS_IFF, m_ShipClassIFF);
	DDX_Text(pDX, IDC_SCLASS_MANUFACTURER, m_ShipClassManufacturer);
	DDX_Text(pDX, IDC_SCLASS_MAX_BANK, m_ShipClassMaxBank);
	DDX_Text(pDX, IDC_SCLASS_MAX_PITCH, m_ShipClassMaxPitch);
	DDX_Text(pDX, IDC_SCLASS_MAX_ROLL, m_ShipClassMaxRoll);
	DDX_Text(pDX, IDC_SCLASS_MAX_SPEED, m_ShipClassMaxSpeed);
	DDX_CBString(pDX, IDC_SCLASS_NAME, m_ShipClassName);
	DDX_Text(pDX, IDC_SCLASS_POWER_PLANT, m_ShipClassPowerPlant);
	DDX_Text(pDX, IDC_SCLASS_SCORE, m_ShipClassScore);
	DDX_Text(pDX, IDC_SCLASS_SHIELDS, m_ShipClassShields);
	DDX_Check(pDX, IDC_SCLASS_WARPDRIVE, m_ShipClassWarpdrive);
	DDX_CBString(pDX, IDC_SCLASS_TURRET_WEAPON1, m_ShipClassTurretWeapon1);
	DDX_CBString(pDX, IDC_SCLASS_TURRET_WEAPON2, m_ShipClassTurretWeapon2);
	DDX_CBString(pDX, IDC_SCLASS_WEAPON_SPECIAL, m_ShipClassWeaponSpecial);
	DDX_CBString(pDX, IDC_SCLASS_WEAPON1, m_ShipClassWeapon1);
	DDX_CBString(pDX, IDC_SCLASS_WEAPON2, m_ShipClassWeapon2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShipClassEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CShipClassEditorDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShipClassEditorDlg message handlers
