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
	CToolTipCtrl m_toolTip;

	BOOL m_enabled;
	CString m_volumetrics_hull;
	vec3d m_position;
	std::array<int, 3> m_color;
	float m_opacity;
	float m_opacityDistance;
	int m_steps;
	int m_resolution;
	int m_oversampling;
	float m_smoothing;
	float m_henyeyGreenstein;
	float m_sunFalloffFactor;
	int m_sunSteps;
	float m_emissiveSpread;
	float m_emissiveIntensity;
	float m_emissiveFalloff;
	BOOL m_noise;
	std::array<int, 3> m_noisecolor;
	float m_noiseScaleBase;
	float m_noiseScaleSub;
	float m_noiseIntensity;
	int m_noiseResolution;

	virtual void DoDataExchange(CDataExchange* pDX) override; 
	virtual BOOL OnInitDialog() override; 
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	virtual void OnCancel() override {}

	void handle_spinner(LPNMUPDOWN spinner, vec3d& vec, float decltype(vec3d::xyz)::*dimension);
	void handle_spinner(LPNMUPDOWN spinner, std::array<int, 3>& color, size_t idx);
	void handle_spinner(LPNMUPDOWN spinner, int& data, int min, int max);
	void handle_spinner(LPNMUPDOWN spinner, float& data, float min, float max, float factor = 1.0f);

	void handle_spinner_exp(LPNMUPDOWN spinner, float& data, float min, float max, float factor = 2.0f);
	void handle_spinner_factor(LPNMUPDOWN spinner, float& data, float min, float max, float factor = 1.0f);

	DECLARE_MESSAGE_MAP()
  public:
	afx_msg void OnBnClickedEnable();
	afx_msg void OnBnClickedSetHull();
	afx_msg void OnDeltaposSpinPosX(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPosY(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinPosZ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinColorR(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinColorG(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinColorB(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinOpacity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinOpacityDistance(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinSteps(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinResolution(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinResolutionOversampling(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinSmoothing(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinHGCoeff(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinSunFalloff(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinStepsSun(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinEMSpread(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinEMIntensity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinEMFalloff(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseColorR(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseColorG(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseColorB(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseScaleB(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseScaleS(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseIntensity(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinNoiseResolution(NMHDR* pNMHDR, LRESULT* pResult);
};
