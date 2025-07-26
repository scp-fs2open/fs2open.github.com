#include "stdafx.h"
#include "FRED.h"
#include "volumetricsdlg.h"

#include "nebula/volumetrics.h"

#define ID_AND_SPIN(name) IDC_##name, IDC_SPIN_##name

static constexpr char* Model_file_ext = "Model Files (*.pof)|*.pof||";
static constexpr std::initializer_list<int> Interactible_fields = {
	IDC_HULL, IDC_SET_HULL,
	ID_AND_SPIN(POS_X),
	ID_AND_SPIN(POS_Y),
	ID_AND_SPIN(POS_Z),
	ID_AND_SPIN(COLOR_R),
	ID_AND_SPIN(COLOR_G),
	ID_AND_SPIN(COLOR_B),
	ID_AND_SPIN(OPACITY),
	ID_AND_SPIN(OPACITY_DISTANCE),
	ID_AND_SPIN(STEPS),
	ID_AND_SPIN(RESOLUTION),
	ID_AND_SPIN(OVERSAMPLING),
	ID_AND_SPIN(SMOOTHING),
	ID_AND_SPIN(HGCOEFF),
	ID_AND_SPIN(SUN_FALLOFF),
	ID_AND_SPIN(STEPS_SUN),
	ID_AND_SPIN(EM_SPREAD),
	ID_AND_SPIN(EM_INTENSITY),
	ID_AND_SPIN(EM_FALLOFF),
	IDC_NOISE_ENABLE
};
static constexpr std::initializer_list<int> Interactible_noise_fields = {
	ID_AND_SPIN(NOISE_COLOR_R),
	ID_AND_SPIN(NOISE_COLOR_G),
	ID_AND_SPIN(NOISE_COLOR_B),
	ID_AND_SPIN(NOISE_SCALE_B),
	ID_AND_SPIN(NOISE_SCALE_S),
	ID_AND_SPIN(NOISE_INTENSITY),
	ID_AND_SPIN(NOISE_RESOLUTION),
	//IDC_NOISE_BASE, IDC_NOISE_SUB //ToDo: Add once buttons are implemented
};

volumetrics_dlg::volumetrics_dlg(CWnd* pParent /*=nullptr*/) : CDialog(volumetrics_dlg::IDD, pParent),
	m_enabled(false),
	m_volumetrics_hull("hull_pof"),
	m_position(ZERO_VECTOR),
	m_color({255, 255, 255}), 
	m_opacity(0.001f),
	m_opacityDistance(5.0f),
	m_steps(15),
	m_resolution(6),
	m_oversampling(2),
	m_smoothing(0.0f),
	m_henyeyGreenstein(0.2f),
	m_sunFalloffFactor(1.0f),
	m_sunSteps(6),
	m_emissiveSpread(0.7f),
	m_emissiveIntensity(1.1f),
	m_emissiveFalloff(1.5f),
	m_noise(false),
	m_noisecolor({0, 0, 0}),
	m_noiseScaleBase(25.0f),
	m_noiseScaleSub(14.0f),
	m_noiseIntensity(1.0f),
	m_noiseResolution(5)
{}

volumetrics_dlg::~volumetrics_dlg()
{
}

BOOL volumetrics_dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_toolTip.Create(this);
	static constexpr char* Tooltip_distance = _T("This is how far something has to be in the nebula to be obscured to the maximum opacity.");
	static constexpr char* Tooltip_steps = _T("If you see banding on ships in the volumetrics, increase this.");
	static constexpr char* Tooltip_oversampling = _T("Increasing this improves the nebula's edge's smoothness especially for large nebula at low resolutions.");
	static constexpr char* Tooltip_smoothing = _T("Smoothing controls how soft edges of the hull POF will be in the nebula, defined as a fraction of the nebula size.");
	static constexpr char* Tooltip_henyey = _T("Values greater than 0 cause a cloud-like light shine-through, values smaller than 0 cause a highly reflective nebula.");
	static constexpr char* Tooltip_sun_falloff = _T("Values greater than 1 means the nebula's depths are brighter than they ought to be, values smaller than 0 means they're darker.");
	static constexpr char* Tooltip_steps_sun = _T("If you see banding in the volumetrics' light and shadow, increase this.");
	static constexpr char* Tooltip_em_spread = _T("How much light sources will scatter in the nebula.");
	static constexpr char* Tooltip_em_falloff = _T("Values greater than 1 will cause light sources in the nebula to spread more evenly, values smaller than 1 will cause them to be more intense in their center.");
	m_toolTip.AddTool(GetDlgItem(IDC_OPACITY_DISTANCE), Tooltip_distance);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_OPACITY_DISTANCE), Tooltip_distance);
	m_toolTip.AddTool(GetDlgItem(IDC_STEPS), Tooltip_steps);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_STEPS), Tooltip_steps);
	m_toolTip.AddTool(GetDlgItem(IDC_OVERSAMPLING), Tooltip_oversampling);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_OVERSAMPLING), Tooltip_oversampling);
	m_toolTip.AddTool(GetDlgItem(IDC_SMOOTHING), Tooltip_smoothing);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_SMOOTHING), Tooltip_smoothing);
	m_toolTip.AddTool(GetDlgItem(IDC_HGCOEFF), Tooltip_henyey);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_HGCOEFF), Tooltip_henyey);
	m_toolTip.AddTool(GetDlgItem(IDC_SUN_FALLOFF), Tooltip_sun_falloff);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_SUN_FALLOFF), Tooltip_sun_falloff);
	m_toolTip.AddTool(GetDlgItem(IDC_STEPS_SUN), Tooltip_steps_sun);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_STEPS_SUN), Tooltip_steps_sun);
	m_toolTip.AddTool(GetDlgItem(IDC_EM_SPREAD), Tooltip_em_spread);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_EM_SPREAD), Tooltip_em_spread);
	m_toolTip.AddTool(GetDlgItem(IDC_EM_FALLOFF), Tooltip_em_falloff);
	m_toolTip.AddTool(GetDlgItem(IDC_SPIN_EM_FALLOFF), Tooltip_em_falloff);
	m_toolTip.Activate(TRUE);

	if (The_mission.volumetrics) {
		const volumetric_nebula& volumetrics = *The_mission.volumetrics;
		m_enabled = true;
		m_volumetrics_hull = volumetrics.hullPof.c_str();
		m_position = volumetrics.pos;
		m_color = { static_cast<int>(std::get<0>(volumetrics.nebulaColor) * 255.0f),
					static_cast<int>(std::get<1>(volumetrics.nebulaColor) * 255.0f),
					static_cast<int>(std::get<2>(volumetrics.nebulaColor) * 255.0f) };
		m_opacity = volumetrics.alphaLim;
		m_opacityDistance = volumetrics.opacityDistance;
		m_steps = volumetrics.steps;
		m_resolution = volumetrics.resolution;
		m_oversampling = volumetrics.oversampling;
		m_smoothing = volumetrics.smoothing;
		m_henyeyGreenstein = volumetrics.henyeyGreensteinCoeff;
		m_sunFalloffFactor = volumetrics.globalLightDistanceFactor;
		m_sunSteps = volumetrics.globalLightSteps;
		m_emissiveSpread = volumetrics.emissiveSpread;
		m_emissiveIntensity = volumetrics.emissiveIntensity;
		m_emissiveFalloff = volumetrics.emissiveFalloff;
		if (volumetrics.noiseActive) {
			m_noisecolor = { static_cast<int>(std::get<0>(volumetrics.noiseColor) * 255.0f),
							 static_cast<int>(std::get<1>(volumetrics.noiseColor) * 255.0f),
							 static_cast<int>(std::get<2>(volumetrics.noiseColor) * 255.0f) };
			m_noiseScaleBase = std::get<0>(volumetrics.noiseScale);
			m_noiseScaleSub = std::get<1>(volumetrics.noiseScale);
			m_noiseIntensity = volumetrics.noiseColorIntensity;
			m_noiseResolution = volumetrics.noiseResolution;
		}
	}

	UpdateData(FALSE);
	OnBnClickedEnable();
	return TRUE;
}

void volumetrics_dlg::OnClose()
{
	UpdateData(TRUE);
	if (!m_enabled) {
		The_mission.volumetrics.reset();

		CDialog::OnOK();
		CDialog::OnClose();
		return;
	}

	volumetric_nebula& volumetrics = The_mission.volumetrics ? *The_mission.volumetrics : The_mission.volumetrics.emplace();

	volumetrics.hullPof = CT2CA(m_volumetrics_hull);
	volumetrics.pos = m_position;
	volumetrics.nebulaColor = {static_cast<float>(m_color[0]) / 255.0f, static_cast<float>(m_color[1]) / 255.0f, static_cast<float>(m_color[2]) / 255.0f};
	volumetrics.alphaLim = m_opacity;
	volumetrics.opacityDistance = m_opacityDistance;
	volumetrics.steps = m_steps;
	volumetrics.resolution = m_resolution;
	volumetrics.oversampling = m_oversampling;
	volumetrics.smoothing = m_smoothing;
	volumetrics.henyeyGreensteinCoeff = m_henyeyGreenstein;
	volumetrics.globalLightDistanceFactor = m_sunFalloffFactor;
	volumetrics.globalLightSteps= m_sunSteps;
	volumetrics.emissiveSpread = m_emissiveSpread;
	volumetrics.emissiveIntensity = m_emissiveIntensity;
	volumetrics.emissiveFalloff = m_emissiveFalloff;

	volumetrics.noiseActive = m_noise;
	if (m_noise) {
		volumetrics.noiseColor = {static_cast<float>(m_noisecolor[0]) / 255.0f, static_cast<float>(m_noisecolor[1]) / 255.0f, static_cast<float>(m_noisecolor[2]) / 255.0f};
		volumetrics.noiseScale = {m_noiseScaleBase, m_noiseScaleSub};
		volumetrics.noiseColorIntensity = m_noiseIntensity;
		volumetrics.noiseResolution = m_noiseResolution;
	}

	CDialog::OnOK();
	CDialog::OnClose();
}

void volumetrics_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ENABLE, m_enabled);
	DDX_Text(pDX, IDC_HULL, m_volumetrics_hull);
	if (m_volumetrics_hull.IsEmpty())
		pDX->Fail();
	DDX_Text(pDX, IDC_POS_X, m_position.xyz.x);
	DDX_Text(pDX, IDC_POS_Y, m_position.xyz.y);
	DDX_Text(pDX, IDC_POS_Z, m_position.xyz.z);
	DDX_Text(pDX, IDC_COLOR_R, m_color[0]);
	DDV_MinMaxInt(pDX, m_color[0], 0, 255);
	DDX_Text(pDX, IDC_COLOR_G, m_color[1]);
	DDV_MinMaxInt(pDX, m_color[1], 0, 255);
	DDX_Text(pDX, IDC_COLOR_B, m_color[2]);
	DDV_MinMaxInt(pDX, m_color[2], 0, 255);
	DDX_Text(pDX, IDC_OPACITY, m_opacity);
	DDV_MinMaxFloat(pDX, m_opacity, 0.0001f, 1.0f);
	DDX_Text(pDX, IDC_OPACITY_DISTANCE, m_opacityDistance);
	DDV_MinMaxFloat(pDX, m_opacityDistance, 0.1f, FLT_MAX);
	DDX_Text(pDX, IDC_STEPS, m_steps);
	DDV_MinMaxInt(pDX, m_steps, 1, 100);
	DDX_Text(pDX, IDC_RESOLUTION, m_resolution);
	DDV_MinMaxInt(pDX, m_resolution, 6, 8);
	DDX_Text(pDX, IDC_OVERSAMPLING, m_oversampling);
	DDV_MinMaxInt(pDX, m_oversampling, 1, 3);
	DDX_Text(pDX, IDC_SMOOTHING, m_smoothing);
	DDV_MinMaxFloat(pDX, m_smoothing, 0.0f, 0.5f);
	DDX_Text(pDX, IDC_HGCOEFF, m_henyeyGreenstein);
	DDV_MinMaxFloat(pDX, m_henyeyGreenstein, -1.0f, 1.0f);
	DDX_Text(pDX, IDC_SUN_FALLOFF, m_sunFalloffFactor);
	DDV_MinMaxFloat(pDX, m_sunFalloffFactor, 0.001f, 100.0f);
	DDX_Text(pDX, IDC_STEPS_SUN, m_sunSteps);
	DDV_MinMaxInt(pDX, m_sunSteps, 2, 16);
	DDX_Text(pDX, IDC_EM_SPREAD, m_emissiveSpread);
	DDV_MinMaxFloat(pDX, m_emissiveSpread, 0.0f, 5.0f);
	DDX_Text(pDX, IDC_EM_INTENSITY, m_emissiveIntensity);
	DDV_MinMaxFloat(pDX, m_emissiveIntensity, 0.0f, 100.0f);
	DDX_Text(pDX, IDC_EM_FALLOFF, m_emissiveFalloff);
	DDV_MinMaxFloat(pDX, m_emissiveFalloff, 0.01f, 10.0f);
	DDX_Check(pDX, IDC_NOISE_ENABLE, m_noise);
	DDX_Text(pDX, IDC_NOISE_COLOR_R, m_noisecolor[0]);
	DDV_MinMaxInt(pDX, m_noisecolor[0], 0, 255);
	DDX_Text(pDX, IDC_NOISE_COLOR_G, m_noisecolor[1]);
	DDV_MinMaxInt(pDX, m_noisecolor[1], 0, 255);
	DDX_Text(pDX, IDC_NOISE_COLOR_B, m_noisecolor[2]);
	DDV_MinMaxInt(pDX, m_noisecolor[2], 0, 255);
	DDX_Text(pDX, IDC_NOISE_SCALE_B, m_noiseScaleBase);
	DDV_MinMaxFloat(pDX, m_noiseScaleBase, 0.01f, 1000.0f);
	DDX_Text(pDX, IDC_NOISE_SCALE_S, m_noiseScaleSub);
	DDV_MinMaxFloat(pDX, m_noiseScaleSub, 0.01f, 1000.0f);
	DDX_Text(pDX, IDC_NOISE_INTENSITY, m_noiseIntensity);
	DDV_MinMaxFloat(pDX, m_noiseIntensity, 0.1f, 100.0f);
	DDX_Text(pDX, IDC_NOISE_RESOLUTION, m_noiseResolution);
	DDV_MinMaxInt(pDX, m_noiseResolution, 4, 8);
}

BOOL volumetrics_dlg::PreTranslateMessage(MSG* pMsg)
{
	m_toolTip.RelayEvent(pMsg);
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(volumetrics_dlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ENABLE, &volumetrics_dlg::OnBnClickedEnable)
	ON_BN_CLICKED(IDC_NOISE_ENABLE, &volumetrics_dlg::OnBnClickedEnable)
	ON_BN_CLICKED(IDC_SET_HULL, &volumetrics_dlg::OnBnClickedSetHull)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_X, &volumetrics_dlg::OnDeltaposSpinPosX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_Y, &volumetrics_dlg::OnDeltaposSpinPosY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_Z, &volumetrics_dlg::OnDeltaposSpinPosZ)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLOR_R, &volumetrics_dlg::OnDeltaposSpinColorR)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLOR_G, &volumetrics_dlg::OnDeltaposSpinColorG)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLOR_B, &volumetrics_dlg::OnDeltaposSpinColorB)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_OPACITY, &volumetrics_dlg::OnDeltaposSpinOpacity)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_OPACITY_DISTANCE, &volumetrics_dlg::OnDeltaposSpinOpacityDistance)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_STEPS, &volumetrics_dlg::OnDeltaposSpinSteps)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RESOLUTION, &volumetrics_dlg::OnDeltaposSpinResolution)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_OVERSAMPLING, &volumetrics_dlg::OnDeltaposSpinResolutionOversampling)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SMOOTHING, &volumetrics_dlg::OnDeltaposSpinSmoothing)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HGCOEFF, &volumetrics_dlg::OnDeltaposSpinHGCoeff)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SUN_FALLOFF, &volumetrics_dlg::OnDeltaposSpinSunFalloff)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_STEPS_SUN, &volumetrics_dlg::OnDeltaposSpinStepsSun)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_EM_SPREAD, &volumetrics_dlg::OnDeltaposSpinEMSpread)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_EM_INTENSITY, &volumetrics_dlg::OnDeltaposSpinEMIntensity)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_EM_FALLOFF, &volumetrics_dlg::OnDeltaposSpinEMFalloff)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_COLOR_R, &volumetrics_dlg::OnDeltaposSpinNoiseColorR)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_COLOR_R, &volumetrics_dlg::OnDeltaposSpinNoiseColorG)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_COLOR_R, &volumetrics_dlg::OnDeltaposSpinNoiseColorB)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_SCALE_B, &volumetrics_dlg::OnDeltaposSpinNoiseScaleB)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_SCALE_S, &volumetrics_dlg::OnDeltaposSpinNoiseScaleS)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_INTENSITY, &volumetrics_dlg::OnDeltaposSpinNoiseIntensity)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NOISE_RESOLUTION, &volumetrics_dlg::OnDeltaposSpinNoiseResolution)
END_MESSAGE_MAP()

void volumetrics_dlg::OnBnClickedSetHull()
{
	CString filename;
	int z;

	UpdateData(TRUE);

	z = cfile_push_chdir(CF_TYPE_DATA);
	CFileDialog dlg(TRUE, nullptr, filename, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Model_file_ext);

	if (dlg.DoModal() == IDOK) {
		m_volumetrics_hull = dlg.GetFileName();
	}

	UpdateData(FALSE);

	// restore directory
	if (!z)
		cfile_pop_dir();
}

void volumetrics_dlg::OnBnClickedEnable()
{
	UpdateData(TRUE);

	for (const int& id : Interactible_fields) {
		GetDlgItem(id)->EnableWindow(m_enabled);
	}

	for (const int& id : Interactible_noise_fields) {
		GetDlgItem(id)->EnableWindow(m_enabled && m_noise);
	}
}

void volumetrics_dlg::handle_spinner(LPNMUPDOWN spinner, vec3d& vec, float decltype(vec3d::xyz)::*dimension)
{
	UpdateData(TRUE);
	vec.xyz.*dimension -= static_cast<float>(spinner->iDelta);
	UpdateData(FALSE);
}

void volumetrics_dlg::handle_spinner(LPNMUPDOWN spinner, std::array<int, 3>& color, size_t idx)
{
	UpdateData(TRUE);
	color[idx] -= spinner->iDelta;
	CAP(color[idx], 0, 0xFF);
	UpdateData(FALSE);
}

void volumetrics_dlg::handle_spinner(LPNMUPDOWN spinner, int& data, int min, int max)
{
	UpdateData(TRUE);
	data -= spinner->iDelta;
	CAP(data, min, max);
	UpdateData(FALSE);
}

void volumetrics_dlg::handle_spinner(LPNMUPDOWN spinner, float& data, float min, float max, float factor)
{
	UpdateData(TRUE);
	data -= static_cast<float>(spinner->iDelta) * factor;
	CAP(data, min, max);
	UpdateData(FALSE);
}

void volumetrics_dlg::handle_spinner_exp(LPNMUPDOWN spinner, float& data, float min, float max, float factor)
{
	UpdateData(TRUE);
	data *= spinner->iDelta > 0 ? 1.0f / factor : factor;
	CAP(data, min, max);
	UpdateData(FALSE);
}

void volumetrics_dlg::handle_spinner_factor(LPNMUPDOWN spinner, float& data, float min, float max, float factor)
{
	UpdateData(TRUE);
	// This is a factor-spinner. Above 1, it is x, below 1 it is 1/x. To enable a smoother transition: If the next de/increment would enter the new mode, already use the new mode now
	factor *= spinner->iDelta;
	data = data > 1.0f + (factor / 2.0f) ? data - factor : 1.0f / (factor + 1.0f / data);
	CAP(data, min, max);
	UpdateData(FALSE);
}

#define SPIN_LINEAR handle_spinner
#define SPIN_EXP handle_spinner_exp
#define SPIN_FACTOR handle_spinner_factor
#define SPINNER_IMPL(type, name, var, ...)                                      \
	void volumetrics_dlg::OnDeltaposSpin##name(NMHDR* pNMHDR, LRESULT* pResult) \
	{                                                                           \
		type(reinterpret_cast<LPNMUPDOWN>(pNMHDR), var, __VA_ARGS__);           \
		*pResult = 0;                                                           \
	}

SPINNER_IMPL(SPIN_LINEAR, PosX, m_position, &decltype(vec3d::xyz)::x)
SPINNER_IMPL(SPIN_LINEAR, PosY, m_position, &decltype(vec3d::xyz)::y)
SPINNER_IMPL(SPIN_LINEAR, PosZ, m_position, &decltype(vec3d::xyz)::z)

SPINNER_IMPL(SPIN_LINEAR, ColorR, m_color, 0)
SPINNER_IMPL(SPIN_LINEAR, ColorG, m_color, 1)
SPINNER_IMPL(SPIN_LINEAR, ColorB, m_color, 2)

//This is the cube root of 10, so by clicking up or down thrice you'll have multiplied the value by 10 for a logarithmic spinner
SPINNER_IMPL(SPIN_EXP, Opacity, m_opacity, 0.0001f, 1.0f, 2.15443469003f)
SPINNER_IMPL(SPIN_LINEAR, OpacityDistance, m_opacityDistance, 0.1f, FLT_MAX)

SPINNER_IMPL(SPIN_LINEAR, Steps, m_steps, 1, 100)
SPINNER_IMPL(SPIN_LINEAR, Resolution, m_resolution, 5, 8)
SPINNER_IMPL(SPIN_LINEAR, ResolutionOversampling, m_oversampling, 1, 3)
SPINNER_IMPL(SPIN_LINEAR, Smoothing, m_smoothing, 0.0f, 0.5f, 0.01f)

SPINNER_IMPL(SPIN_LINEAR, HGCoeff, m_henyeyGreenstein, -1.0f, 1.0f, 0.1f)
SPINNER_IMPL(SPIN_FACTOR, SunFalloff, m_sunFalloffFactor, 0.001f, 100.0f)
SPINNER_IMPL(SPIN_LINEAR, StepsSun, m_sunSteps, 2, 16)

SPINNER_IMPL(SPIN_LINEAR, EMSpread, m_emissiveSpread, 0.0f, 5.0f, 0.1f)
SPINNER_IMPL(SPIN_LINEAR, EMIntensity, m_emissiveIntensity, 0.0f, 100.0f, 0.1f)
SPINNER_IMPL(SPIN_FACTOR, EMFalloff, m_emissiveFalloff, 0.01f, 10.0f, 0.2f)

SPINNER_IMPL(SPIN_LINEAR, NoiseColorR, m_noisecolor, 0)
SPINNER_IMPL(SPIN_LINEAR, NoiseColorG, m_noisecolor, 1)
SPINNER_IMPL(SPIN_LINEAR, NoiseColorB, m_noisecolor, 2)

SPINNER_IMPL(SPIN_LINEAR, NoiseScaleB, m_noiseScaleBase, 0.01f, 1000.0f)
SPINNER_IMPL(SPIN_LINEAR, NoiseScaleS, m_noiseScaleSub, 0.01f, 1000.0f)

SPINNER_IMPL(SPIN_LINEAR, NoiseIntensity, m_noiseIntensity, 0.1f, 100.0f)
SPINNER_IMPL(SPIN_LINEAR, NoiseResolution, m_noiseResolution, 5, 8)
