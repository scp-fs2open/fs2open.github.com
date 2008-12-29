// TabHelp.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "TabHelp.h"
#include "Win32func.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabHelp dialog


CTabHelp::CTabHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CTabHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabHelp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabHelp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabHelp, CDialog)
	//{{AFX_MSG_MAP(CTabHelp)
	ON_BN_CLICKED(IDC_GAME_README, OnReadme)
	ON_BN_CLICKED(IDC_GOTO_FORUM, OnGotoForum)
	ON_BN_CLICKED(IDC_REPORT_BUG, OnReportBug)
	ON_BN_CLICKED(IDC_FEATURES_ON, OnFeaturesOn)
	ON_BN_CLICKED(IDC_FEATURES_OFF, OnFeaturesOff)
	ON_BN_CLICKED(IDC_CUSTOM, OnCustom)
	ON_BN_CLICKED(IDC_REGISTER_BUTTON, OnRegisterButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabHelp message handlers

void CTabHelp::OnReadme() 
{
	OpenInternetPage("http://www.hard-light.net/wiki/index.php/Main_Page");
}

void CTabHelp::OnGotoForum() 
{
	OpenInternetPage("http://www.hard-light.net/forums/index.php/board,50.0.html");
}

void CTabHelp::OnReportBug() 
{
	OpenInternetPage("http://scp.indiegames.us/mantis/main_page.php");
}

void CTabHelp::OpenInternetPage(char *path)
{
	open_web_page(path);
}

void CTabHelp::OnFeaturesOn() 
{
}

void CTabHelp::OnFeaturesOff() 
{
}

void CTabHelp::OnCustom()
{
}

BOOL CTabHelp::OnInitDialog() 
{
	CDialog::OnInitDialog();

	srand( (unsigned)time( NULL ) );

	GetDlgItem(IDC_LOGO1)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LOGO2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LOGO3)->ShowWindow(SW_HIDE);
	
 	switch(rand() % 3)
	{
	case 0: GetDlgItem(IDC_LOGO1)->ShowWindow(SW_SHOW); break;
	case 1: GetDlgItem(IDC_LOGO2)->ShowWindow(SW_SHOW);	break;
	case 2: GetDlgItem(IDC_LOGO3)->ShowWindow(SW_SHOW);	break;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabHelp::OnRegisterButton() 
{
	OpenInternetPage("http://www.game-warden.com/forum/showthread.php?t=267");
	
}
