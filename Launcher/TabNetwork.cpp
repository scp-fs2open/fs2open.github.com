// TabNetwork.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "TabNetwork.h"

#include "win32func.h"
#include "launcher_settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int NUM_NET_CONNECTION_TYPES = 3; 
const int NUM_NET_CONNECTION_SPEED = 6; 

typedef struct
{
	int id;
	char *text;
} RadioList; 

RadioList net_connection_list[NUM_NET_CONNECTION_TYPES] = 
{
	IDC_NET_TYPE_1,	"None",
	IDC_NET_TYPE_2,	"Dialup",
	IDC_NET_TYPE_3,	"LAN"
}; 

RadioList connection_speed_list[NUM_NET_CONNECTION_SPEED] =
{
	IDC_CONSPEED_1, "None", 
	IDC_CONSPEED_2,	"Slow", 
	IDC_CONSPEED_3,	"56K",  
	IDC_CONSPEED_4,	"ISDN", 
	IDC_CONSPEED_5,	"Cable",
	IDC_CONSPEED_6,	"Fast" 
};

/////////////////////////////////////////////////////////////////////////////
// CTabNetwork dialog


CTabNetwork::CTabNetwork(CWnd* pParent /*=NULL*/)
	: CDialog(CTabNetwork::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabNetwork)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTabNetwork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabNetwork)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabNetwork, CDialog)
	//{{AFX_MSG_MAP(CTabNetwork)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabNetwork message handlers

void CTabNetwork::OnApply()
{
	int i; 

	// Check NetworkConnection radio buttons
 	for(i = 0; i < NUM_NET_CONNECTION_TYPES; i++)
		if(((CButton *) GetDlgItem(net_connection_list[i].id))->GetCheck() == 1)
		{
			reg_set_sz(LauncherSettings::get_reg_path(), "NetworkConnection", net_connection_list[i].text);
			continue;
		}

	// Check ConnectionSpeed radio buttons
	for(i = 0; i < NUM_NET_CONNECTION_SPEED; i++)
		if(((CButton *) GetDlgItem(connection_speed_list[i].id))->GetCheck() == 1)
		{
			reg_set_sz(LauncherSettings::get_reg_path(), "ConnectionSpeed", connection_speed_list[i].text);
			continue;
		}

	// Force the port number here
	char  local_port_text[50];
	int   local_port_value;
	GetDlgItem(IDC_LOCAL_PORT)->GetWindowText(local_port_text, 50);

	if(strlen(local_port_text) > 0)
	{
		local_port_value = atoi(local_port_text);
		reg_set_dword(LauncherSettings::get_reg_path(), "ForcePort", local_port_value);
	}

	// custom ip
	char custom_ip[20] = { 0 };
	GetDlgItem(IDC_CUSTOM_IP)->GetWindowText(custom_ip, 20);

	char net_reg_path[MAX_PATH] = { 0 };
	_snprintf(net_reg_path, MAX_PATH - 1, "%s\\%s", LauncherSettings::get_reg_path(), "Network");

	if ( strlen(custom_ip) > 0 )
		reg_set_sz(net_reg_path, "CustomIP", custom_ip);
}

BOOL CTabNetwork::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	((CButton *) GetDlgItem(IDC_NET_TYPE_1))->SetCheck(1);
	((CButton *) GetDlgItem(IDC_CONSPEED_1))->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabNetwork::LoadSettings()
{
	int i; 
	char net_connection[20], net_speed[20];

	reg_get_sz(LauncherSettings::get_reg_path(), "NetworkConnection", net_connection, 20);
	reg_get_sz(LauncherSettings::get_reg_path(), "ConnectionSpeed", net_speed, 20);

	for(i = 0; i < NUM_NET_CONNECTION_TYPES; i++)
	{
		int result = (stricmp(net_connection, net_connection_list[i].text) == 0) ? 1 : 0;
		((CButton *) GetDlgItem(net_connection_list[i].id))->SetCheck(result);
	}

	for(i = 0; i < NUM_NET_CONNECTION_SPEED; i++)
	{
		int result = (stricmp(net_speed, connection_speed_list[i].text) == 0) ? 1 : 0;
		((CButton *) GetDlgItem(connection_speed_list[i].id))->SetCheck(result);
	}

	// Force the port number here
	DWORD local_port_value = 0;
	if(reg_get_dword(LauncherSettings::get_reg_path(), "ForcePort", &local_port_value))
	{
		char local_port_text[50];

		sprintf(local_port_text, "%d", local_port_value);
		GetDlgItem(IDC_LOCAL_PORT)->SetWindowText(local_port_text);
	}

	char net_reg_path[MAX_PATH] = { 0 };
	_snprintf(net_reg_path, MAX_PATH - 1, "%s\\%s", LauncherSettings::get_reg_path(), "Network");

	char custom_ip[20] = { 0 };
	if ( reg_get_sz(net_reg_path, "CustomIP", custom_ip, 20) )
		GetDlgItem(IDC_CUSTOM_IP)->SetWindowText(custom_ip);
}
