#include "stdafx.h"
#include "FRED.h"
#include "volumetricsdlg.h"

#include "nebula/volumetrics.h"

static constexpr char* Model_file_ext = "Model Files (*.pof)|*.pof||";
static constexpr std::initializer_list<int> Interactible_fields = {
	IDC_HULL,
	IDC_POS_X,
	IDC_SPIN_POS_X,
	IDC_POS_Y,
	IDC_SPIN_POS_Y,
	IDC_POS_Z,
	IDC_SPIN_POS_Z,
	IDC_COLOR_R,
	IDC_SPIN_COLOR_R,
	IDC_COLOR_G,
	IDC_SPIN_COLOR_G,
	IDC_COLOR_B,
	IDC_SPIN_COLOR_B
};

volumetrics_dlg::volumetrics_dlg(CWnd* pParent /*=nullptr*/) : CDialog(volumetrics_dlg::IDD, pParent),
	m_enabled(false),
	m_volumetrics_hull(""),
	m_position(ZERO_VECTOR),
	m_color({255, 255, 255})
{}

volumetrics_dlg::~volumetrics_dlg()
{
}

BOOL volumetrics_dlg::OnInitDialog()
{
	if (The_mission.volumetrics) {
		const volumetric_nebula& volumetrics = *The_mission.volumetrics;
		m_enabled = true;
		m_volumetrics_hull = volumetrics.hullPof.c_str();
		m_position = volumetrics.pos;
		m_color = { static_cast<int>(std::get<0>(volumetrics.nebulaColor) * 255.0f),
					static_cast<int>(std::get<1>(volumetrics.nebulaColor) * 255.0f),
					static_cast<int>(std::get<2>(volumetrics.nebulaColor) * 255.0f) };
	}
	UpdateData(FALSE);
	return TRUE;
}

void volumetrics_dlg::OnClose()
{
	UpdateData(TRUE);
	if (!m_enabled) {
		The_mission.volumetrics.reset();
		return;
	}

	volumetric_nebula& volumetrics = The_mission.volumetrics ? *The_mission.volumetrics : The_mission.volumetrics.emplace();

	volumetrics.hullPof = CT2CA(m_volumetrics_hull);
	volumetrics.pos = m_position;
	volumetrics.nebulaColor = {static_cast<float>(m_color[0]) / 255.0f, static_cast<float>(m_color[1]) / 255.0f, static_cast<float>(m_color[2]) / 255.0f};
}

void volumetrics_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ENABLE, m_enabled);
	DDX_Text(pDX, IDC_HULL, m_volumetrics_hull);
	DDX_Text(pDX, IDC_POS_X, m_position.xyz.x);
	DDX_Text(pDX, IDC_POS_Y, m_position.xyz.y);
	DDX_Text(pDX, IDC_POS_Z, m_position.xyz.z);
	DDX_Text(pDX, IDC_COLOR_R, m_color[0]);
	DDV_MinMaxInt(pDX, m_color[0], 0, 255);
	DDX_Text(pDX, IDC_COLOR_G, m_color[1]);
	DDV_MinMaxInt(pDX, m_color[1], 0, 255);
	DDX_Text(pDX, IDC_COLOR_B, m_color[2]);
	DDV_MinMaxInt(pDX, m_color[2], 0, 255);
}

BEGIN_MESSAGE_MAP(volumetrics_dlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SET_HULL, &volumetrics_dlg::OnBnClickedSetHull)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_X, &volumetrics_dlg::OnDeltaposSpinPosX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_Y, &volumetrics_dlg::OnDeltaposSpinPosY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POS_Z, &volumetrics_dlg::OnDeltaposSpinPosZ)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLOR_R, &volumetrics_dlg::OnDeltaposColorRSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLOR_G, &volumetrics_dlg::OnDeltaposColorGSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_COLOR_B, &volumetrics_dlg::OnDeltaposColorBSpin)
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

void volumetrics_dlg::handle_spinner_vec3d(LPNMUPDOWN spinner, vec3d& vec, float decltype(vec3d::xyz)::*dimension)
{
	UpdateData(TRUE);
	vec.xyz.*dimension -= static_cast<float>(spinner->iDelta);
	UpdateData(FALSE);
}

void volumetrics_dlg::handle_spinner_color(LPNMUPDOWN spinner, std::array<int, 3>& color, size_t idx) {
	UpdateData(TRUE);
	color[idx] -= spinner->iDelta;
	CAP(color[idx], 0, 0xFF);
	UpdateData(FALSE);
}

void volumetrics_dlg::OnDeltaposSpinPosX(NMHDR* pNMHDR, LRESULT* pResult)
{
	handle_spinner_vec3d(reinterpret_cast<LPNMUPDOWN>(pNMHDR), m_position, &decltype(vec3d::xyz)::x);
	*pResult = 0;
}

void volumetrics_dlg::OnDeltaposSpinPosY(NMHDR* pNMHDR, LRESULT* pResult)
{
	handle_spinner_vec3d(reinterpret_cast<LPNMUPDOWN>(pNMHDR), m_position, &decltype(vec3d::xyz)::y);
	*pResult = 0;
}

void volumetrics_dlg::OnDeltaposSpinPosZ(NMHDR* pNMHDR, LRESULT* pResult)
{
	handle_spinner_vec3d(reinterpret_cast<LPNMUPDOWN>(pNMHDR), m_position, &decltype(vec3d::xyz)::z);
	*pResult = 0;
}

void volumetrics_dlg::OnDeltaposColorRSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	handle_spinner_color(reinterpret_cast<LPNMUPDOWN>(pNMHDR), m_color, 0);
	*pResult = 0;
}

void volumetrics_dlg::OnDeltaposColorGSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	handle_spinner_color(reinterpret_cast<LPNMUPDOWN>(pNMHDR), m_color, 1);
	*pResult = 0;
}

void volumetrics_dlg::OnDeltaposColorBSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	handle_spinner_color(reinterpret_cast<LPNMUPDOWN>(pNMHDR), m_color, 2);
	*pResult = 0;
}
