#pragma once
#include "resource.h"

class volumetrics_dlg : public CDialog
{

public:
	volumetrics_dlg(CWnd* pParent = nullptr);   
	virtual ~volumetrics_dlg();

	enum { IDD = IDD_VOLUMETRICS };

	afx_msg void OnClose();

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 

	DECLARE_MESSAGE_MAP()
};
