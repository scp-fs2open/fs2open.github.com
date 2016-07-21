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
#include "OperatorArgTypeSelect.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OperatorArgTypeSelect dialog


OperatorArgTypeSelect::OperatorArgTypeSelect(CWnd* pParent /*=NULL*/)
	: CDialog(OperatorArgTypeSelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(OperatorArgTypeSelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void OperatorArgTypeSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OperatorArgTypeSelect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OperatorArgTypeSelect, CDialog)
	//{{AFX_MSG_MAP(OperatorArgTypeSelect)
	ON_BN_CLICKED(ID_BOOLEAN, OnBoolean)
	ON_BN_CLICKED(ID_NUMBERS, OnNumbers)
	ON_BN_CLICKED(ID_SHIPS, OnShips)
	ON_BN_CLICKED(ID_WINGS, OnWings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OperatorArgTypeSelect message handlers

void OperatorArgTypeSelect::OnBoolean() 
{
	EndDialog(ID_BOOLEAN);
}

void OperatorArgTypeSelect::OnNumbers() 
{
	EndDialog(ID_NUMBERS);
}

void OperatorArgTypeSelect::OnShips() 
{
	EndDialog(ID_SHIPS);
}

void OperatorArgTypeSelect::OnWings() 
{
	EndDialog(ID_WINGS);
}
