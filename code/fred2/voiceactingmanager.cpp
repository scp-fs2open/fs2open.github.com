// VoiceActingManager.cpp : implementation file
//

#include "stdafx.h"
#include "fred.h"
#include "VoiceActingManager.h"
#include "VoiceFileManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VoiceActingManager dialog


VoiceActingManager::VoiceActingManager(CWnd* pParent /*=NULL*/)
	: CDialog(VoiceActingManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(VoiceActingManager)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void VoiceActingManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VoiceActingManager)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VoiceActingManager, CDialog)
	//{{AFX_MSG_MAP(VoiceActingManager)
	ON_BN_CLICKED(IDC_GEN_FILENAMES, OnGenFilenames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VoiceActingManager message handlers

void VoiceActingManager::OnGenFilenames() 
{
	VoiceFileManager dlg;
	
	dlg.DoModal();	
}
