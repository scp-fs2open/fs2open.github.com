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
	ship *shipp;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		shipp = &Ships[i];
		if (shipp->objnum < 0)
			continue;

		Objects[shipp->objnum].flags |= OF_NO_SHIELDS;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnNoSubspaceDrive() 
{
	int i;
	ship *shipp;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		shipp = &Ships[i];
		if (shipp->objnum < 0)
			continue;

		// only for fighters and bombers
		if (Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))
			shipp->flags2 |= SF2_NO_SUBSPACE_DRIVE;
		else
			shipp->flags2 &= ~SF2_NO_SUBSPACE_DRIVE;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnPrimitiveSensors() 
{
	int i;
	ship *shipp;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		shipp = &Ships[i];
		if (shipp->objnum < 0)
			continue;

		// only for fighters and bombers
		if (Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))
			shipp->flags2 |= SF2_PRIMITIVE_SENSORS;
		else
			shipp->flags2 &= ~SF2_PRIMITIVE_SENSORS;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnAffectedByGravity() 
{
	int i;
	ship *shipp;
	
	for (i=0; i<MAX_SHIPS; i++)
	{
		shipp = &Ships[i];
		if (shipp->objnum < 0)
			continue;

		// only for fighters and bombers
		if (Ship_info[shipp->ship_info_index].flags & (SIF_FIGHTER | SIF_BOMBER))
			shipp->flags2 |= SF2_AFFECTED_BY_GRAVITY;
		else
			shipp->flags2 &= ~SF2_AFFECTED_BY_GRAVITY;
	}

	MessageBox("Task complete.");
}

void SetGlobalShipFlags::OnResetScores()
{
	int i, z;
	bool confirm_each;
	ship *shipp;
	ship_info *sip;
	
	z = MessageBox("Do you want to confirm each ship score?", "Reset Ship Scores", MB_ICONQUESTION | MB_YESNO);
	confirm_each = (z == IDYES);

	for (i=0; i<MAX_SHIPS; i++)
	{
		shipp = &Ships[i];
		if (shipp->objnum < 0)
			continue;

		sip = (shipp->ship_info_index >= 0) ? &Ship_info[shipp->ship_info_index] : NULL;
		if (sip == NULL)
			continue;

		if (shipp->score == sip->score)
			continue;

		if (confirm_each)
		{
			char temp[NAME_LENGTH + NAME_LENGTH + 30];
			sprintf(temp, "Change %s (%s) from %d to %d?", shipp->ship_name, sip->name, shipp->score, sip->score);
			
			z = MessageBox(temp, "Reset Score", MB_ICONQUESTION | MB_YESNO);
			if (z != IDYES)
				continue;
		}

		shipp->score = sip->score;
	}

	MessageBox("Task complete.");
}