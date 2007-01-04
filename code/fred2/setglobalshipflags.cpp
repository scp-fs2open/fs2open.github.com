// SetGlobalShipFlags.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "SetGlobalShipFlags.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SetGlobalShipFlags dialog


SetGlobalShipFlags::SetGlobalShipFlags(CWnd* pParent /*=NULL*/)
	: CDialog(SetGlobalShipFlags::IDD, pParent)
{
	//{{AFX_DATA_INIT(SetGlobalShipFlags)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SetGlobalShipFlags::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SetGlobalShipFlags)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SetGlobalShipFlags, CDialog)
	//{{AFX_MSG_MAP(SetGlobalShipFlags)
	ON_BN_CLICKED(IDC_NO_SHIELDS, OnNoShields)
	ON_BN_CLICKED(IDC_NO_SUBSPACE_DRIVE, OnNoSubspaceDrive)
	ON_BN_CLICKED(IDC_PRIMITIVE_SENSORS, OnPrimitiveSensors)
	ON_BN_CLICKED(IDC_AFFECTED_BY_GRAVITY, OnAffectedByGravity)
	ON_BN_CLICKED(IDC_RESET_SCORES, OnResetScores)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SetGlobalShipFlags message handlers

void SetGlobalShipFlags::OnNoShields() 
{
	int i;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		if (Ships[i].objnum < 0)
			continue;

		Objects[Ships[i].objnum].flags |= OF_NO_SHIELDS;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnNoSubspaceDrive() 
{
	int i;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		if (Ships[i].objnum < 0)
			continue;

		// only for fighters and bombers
		if (Ship_info[Ships[i].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))
			Ships[i].flags2 |= SF2_NO_SUBSPACE_DRIVE;
		else
			Ships[i].flags2 &= ~SF2_NO_SUBSPACE_DRIVE;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnPrimitiveSensors() 
{
	int i;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		if (Ships[i].objnum < 0)
			continue;

		// only for fighters and bombers
		if (Ship_info[Ships[i].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))
			Ships[i].flags2 |= SF2_PRIMITIVE_SENSORS;
		else
			Ships[i].flags2 &= ~SF2_PRIMITIVE_SENSORS;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnAffectedByGravity() 
{
	int i;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		if (Ships[i].objnum < 0)
			continue;

		// only for fighters and bombers
		if (Ship_info[Ships[i].ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))
			Ships[i].flags2 |= SF2_AFFECTED_BY_GRAVITY;
		else
			Ships[i].flags2 &= ~SF2_AFFECTED_BY_GRAVITY;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnResetScores()
{
	int i, ship_class;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		if (Ships[i].objnum < 0)
			continue;

		ship_class = Ships[i].ship_info_index;
		if (ship_class < 0)
			continue;

		Ships[i].score = Ship_info[ship_class].score;
	}

	MessageBox("Task complete.");
}