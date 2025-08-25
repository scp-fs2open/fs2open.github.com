#include "FredApplication.h"
#include "BackgroundEditorDialogModel.h"

#include "graphics/light.h"
#include "math/bitarray.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "starfield/nebula.h"

// TODO move this to common for both FREDs. Do not pass review if this is not done
const static float delta = .00001f;
const static float default_nebula_range = 3000.0f;

namespace fso::fred::dialogs {
BackgroundEditorDialogModel::BackgroundEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	auto& bg = getActiveBackground();
	auto& bm_list = bg.bitmaps;
	if (!bm_list.empty()) {
		_selectedBitmapIndex = 0;
	}

	auto& sun_list = bg.suns;
	if (!sun_list.empty()) {
		_selectedSunIndex = 0;
	}
}

bool BackgroundEditorDialogModel::apply()
{
	// override dumb values with reasonable ones
	// this is what original FRED does but it was a text edit field using atoi
	// ours is a limited spinbox so this probably isn't necessary anymore??
	// Does this mean range can never be 0?????????
	if (Neb2_awacs <= 0.00000001f) {
		Neb2_awacs = 3000.0f;
	}
	return true;
}

void BackgroundEditorDialogModel::reject()
{
	// do nothing
}

void BackgroundEditorDialogModel::refreshBackgroundPreview()
{
	stars_load_background(Cur_background); // rebuild instances from Backgrounds[]
	_editor->missionChanged();
}

background_t& BackgroundEditorDialogModel::getActiveBackground() const
{
	if (!SCP_vector_inbounds(Backgrounds, Cur_background)) {
		// Fall back to first background if Cur_background isn’t set
		Cur_background = 0;
	}
	return Backgrounds[Cur_background];
}

starfield_list_entry* BackgroundEditorDialogModel::getActiveBitmap() const
{
	auto& bg = getActiveBackground();
	auto& list = bg.bitmaps;
	if (!SCP_vector_inbounds(list, _selectedBitmapIndex)) {
		return nullptr;
	}
	return &list[_selectedBitmapIndex];
}

starfield_list_entry* BackgroundEditorDialogModel::getActiveSun() const
{
	auto& bg = getActiveBackground();
	auto& list = bg.suns;
	if (!SCP_vector_inbounds(list, _selectedSunIndex)) {
		return nullptr;
	}
	return &list[_selectedSunIndex];
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getAvailableBitmapNames() const
{
	SCP_vector<SCP_string> out;
	const int count = stars_get_num_entries(/*is_a_sun=*/false, /*bitmap_count=*/true);
	out.reserve(count);
	for (int i = 0; i < count; ++i) {
		if (const char* name = stars_get_name_FRED(i, /*is_a_sun=*/false)) {
			out.emplace_back(name);
		}
	}
	return out;
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getMissionBitmapNames() const
{
	SCP_vector<SCP_string> out;
	const auto& vec = getActiveBackground().bitmaps;
	out.reserve(vec.size());
	for (const auto& sle : vec) {
		out.emplace_back(sle.filename);
	}
	return out;
}

void BackgroundEditorDialogModel::setSelectedBitmapIndex(int index)
{
	const auto& bg = getActiveBackground();
	const auto& list = bg.bitmaps;
	if (!SCP_vector_inbounds(list, index)) {
		_selectedBitmapIndex = -1;
		return;
	}
	_selectedBitmapIndex = index;
}

int BackgroundEditorDialogModel::getSelectedBitmapIndex() const
{
	return _selectedBitmapIndex;
}

void BackgroundEditorDialogModel::addMissionBitmapByName(const SCP_string& name)
{
	if (name.empty())
		return;

	// Must exist in tables
	if (stars_find_bitmap(name.c_str()) < 0)
		return;

	starfield_list_entry sle{};
	std::strncpy(sle.filename, name.c_str(), MAX_FILENAME_LEN - 1);
	sle.ang.p = 0.0f;
	sle.ang.b = 0.0f;
	sle.ang.h = 0.0f;
	sle.scale_x = 1.0f;
	sle.scale_y = 1.0f;
	sle.div_x = 1;
	sle.div_y = 1;

	auto& list = getActiveBackground().bitmaps;
	list.push_back(sle);

	_selectedBitmapIndex = static_cast<int>(list.size()) - 1;

	set_modified();
	refreshBackgroundPreview();
}

void BackgroundEditorDialogModel::removeMissionBitmap()
{
	auto& list = getActiveBackground().bitmaps;

	// Make sure we have an active bitmap
	if (getActiveBitmap() == nullptr) {
		return;
	}

	list.erase(list.begin() + _selectedBitmapIndex);
	
	// choose a sensible new selection
	if (list.empty()) {
		_selectedBitmapIndex = -1;
	} else {
		_selectedBitmapIndex = std::min(_selectedBitmapIndex, static_cast<int>(list.size()) - 1);
	}

	set_modified();
	refreshBackgroundPreview();
}

SCP_string BackgroundEditorDialogModel::getBitmapName() const
{
	auto bm = getActiveBitmap();
	if (bm == nullptr) {
		return "";
	}

	return bm->filename;
}

void BackgroundEditorDialogModel::setBitmapName(const SCP_string& name)
{
	if (name.empty())
		return;

	// Must exist in tables
	if (stars_find_bitmap(name.c_str()) < 0)
		return;

	auto bm = getActiveBitmap();
	if (bm != nullptr) {
		strcpy_s(bm->filename, name.c_str());
		set_modified();
		refreshBackgroundPreview();
	}
}

int BackgroundEditorDialogModel::getBitmapPitch() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return fl2ir(fl_degrees(bm->ang.p) + delta);
}

void BackgroundEditorDialogModel::setBitmapPitch(int deg)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	modify(bm->ang.p, fl_radians(deg));

	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getBitmapBank() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return fl2ir(fl_degrees(bm->ang.b) + delta);
}

void BackgroundEditorDialogModel::setBitmapBank(int deg)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	modify(bm->ang.b, fl_radians(deg));

	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getBitmapHeading() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return fl2ir(fl_degrees(bm->ang.h) + delta);
}

void BackgroundEditorDialogModel::setBitmapHeading(int deg)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	modify(bm->ang.h, fl_radians(deg));

	refreshBackgroundPreview();
}

float BackgroundEditorDialogModel::getBitmapScaleX() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return bm->scale_x;
}

void BackgroundEditorDialogModel::setBitmapScaleX(float v)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(v, getBitmapScaleLimit().first, getBitmapScaleLimit().second);
	modify(bm->scale_x, v);

	refreshBackgroundPreview();
}

float BackgroundEditorDialogModel::getBitmapScaleY() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return bm->scale_y;
}

void BackgroundEditorDialogModel::setBitmapScaleY(float v)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(v, getBitmapScaleLimit().first, getBitmapScaleLimit().second);
	modify(bm->scale_y, v);

	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getBitmapDivX() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return bm->div_x;
}

void BackgroundEditorDialogModel::setBitmapDivX(int v)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(v, getDivisionLimit().first, getDivisionLimit().second);
	modify(bm->div_x, v);

	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getBitmapDivY() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return 0;

	return bm->div_y;
}

void BackgroundEditorDialogModel::setBitmapDivY(int v)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(v, getDivisionLimit().first, getDivisionLimit().second);
	modify(bm->div_y, v);

	refreshBackgroundPreview();
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getAvailableSunNames() const
{
	SCP_vector<SCP_string> out;
	const int count = stars_get_num_entries(/*is_a_sun=*/true, /*bitmap_count=*/true);
	out.reserve(count);
	for (int i = 0; i < count; ++i) {
		if (const char* name = stars_get_name_FRED(i, /*is_a_sun=*/true)) { // table order
			out.emplace_back(name);
		}
	}
	return out;
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getMissionSunNames() const
{
	SCP_vector<SCP_string> out;
	const auto& vec = getActiveBackground().suns;
	out.reserve(vec.size());
	for (const auto& sle : vec)
		out.emplace_back(sle.filename);
	return out;
}

void BackgroundEditorDialogModel::setSelectedSunIndex(int index)
{
	const auto& list = getActiveBackground().suns;
	if (!SCP_vector_inbounds(list, index)) {
		_selectedSunIndex = -1;
		return;
	}
	_selectedSunIndex = index;
}

int BackgroundEditorDialogModel::getSelectedSunIndex() const
{
	return _selectedSunIndex;
}

void BackgroundEditorDialogModel::addMissionSunByName(const SCP_string& name)
{
	if (name.empty())
		return;

	if (stars_find_sun(name.c_str()) < 0)
		return; // must exist in sun table

	starfield_list_entry sle{};
	std::strncpy(sle.filename, name.c_str(), MAX_FILENAME_LEN - 1);
	sle.ang.p = sle.ang.b = sle.ang.h = 0.0f;
	sle.scale_x = 1.0f;
	sle.scale_y = 1.0f;
	sle.div_x = 1;
	sle.div_y = 1;

	auto& list = getActiveBackground().suns;
	list.push_back(sle);
	_selectedSunIndex = static_cast<int>(list.size()) - 1;

	set_modified();
	refreshBackgroundPreview();
}

void BackgroundEditorDialogModel::removeMissionSun()
{
	auto& list = getActiveBackground().suns;
	if (getActiveSun() == nullptr)
		return;

	list.erase(list.begin() + _selectedSunIndex);
	if (list.empty())
		_selectedSunIndex = -1;
	else
		_selectedSunIndex = std::min(_selectedSunIndex, static_cast<int>(list.size()) - 1);

	set_modified();
	refreshBackgroundPreview();
}

SCP_string BackgroundEditorDialogModel::getSunName() const
{
	auto* s = getActiveSun();
	if (!s)
		return "";

	return s->filename;
}

void BackgroundEditorDialogModel::setSunName(const SCP_string& name)
{
	if (name.empty())
		return;

	if (stars_find_sun(name.c_str()) < 0)
		return;

	auto* s = getActiveSun();
	if (!s)
		return;

	strcpy_s(s->filename, name.c_str());
	set_modified();
	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getSunPitch() const
{
	auto* s = getActiveSun();
	if (!s)
		return 0;

	return fl2ir(fl_degrees(s->ang.p) + delta);
}

void BackgroundEditorDialogModel::setSunPitch(int deg)
{
	auto* s = getActiveSun();
	if (!s)
		return;

	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	modify(s->ang.p, fl_radians(deg));
	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getSunBank() const
{
	// Bank is not used for suns but this is added for consistency
	/*auto* s = getActiveSun();
	if (!s)
		return 0;

	return fl2ir(fl_degrees(s->ang.b) + delta);*/
}

void BackgroundEditorDialogModel::setSunBank(int deg)
{
	// Bank is not used for suns but this is added for consistency
	/*auto* s = getActiveSun();
	if (!s)
		return;

	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	modify(s->ang.b, fl_radians(deg));
	refreshBackgroundPreview();*/
}

int BackgroundEditorDialogModel::getSunHeading() const
{
	auto* s = getActiveSun();
	if (!s)
		return 0;

	return fl2ir(fl_degrees(s->ang.h) + delta);
}

void BackgroundEditorDialogModel::setSunHeading(int deg)
{
	auto* s = getActiveSun();
	if (!s)
		return;

	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	modify(s->ang.h, fl_radians(deg));
	refreshBackgroundPreview();
}

float BackgroundEditorDialogModel::getSunScale() const
{
	auto* s = getActiveSun();
	if (!s)
		return 0;

	return s->scale_x; // suns store scale in X; Y remains 1.0
}

void BackgroundEditorDialogModel::setSunScale(float v)
{
	auto* s = getActiveSun();
	if (!s)
		return;

	CLAMP(v, getSunScaleLimit().first, getSunScaleLimit().second);
	modify(s->scale_x, v);
	refreshBackgroundPreview();
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getLightningNames() const
{
	SCP_vector<SCP_string> out;
	out.emplace_back("<None>"); // legacy default
	for (const auto& st : Storm_types) {
		out.emplace_back(st.name);
	}
	return out;
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getNebulaPatternNames() const
{
	SCP_vector<SCP_string> out;
	out.emplace_back("<None>"); // matches legacy combo where index 0 = none
	for (const auto& neb : Neb2_bitmap_filenames) {
		out.emplace_back(neb);
	}
	return out;
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getPoofNames() const
{
	SCP_vector<SCP_string> out;
	out.reserve(Poof_info.size());
	for (const auto& p : Poof_info) {
		out.emplace_back(p.name);
	}
	return out;
}

bool BackgroundEditorDialogModel::getFullNebulaEnabled() const
{
	return The_mission.flags[Mission::Mission_Flags::Fullneb];
}

void BackgroundEditorDialogModel::setFullNebulaEnabled(bool enabled)
{
	const bool currentlyEnabled = getFullNebulaEnabled();
	if (enabled == currentlyEnabled) {
		return;
	}

	if (enabled) {
		The_mission.flags.set(Mission::Mission_Flags::Fullneb);

		// Set defaults if needed
		if (Neb2_awacs <= 0.0f) {
			modify(Neb2_awacs, default_nebula_range);
		}
	} else {
		// Disable full nebula
		The_mission.flags.remove(Mission::Mission_Flags::Fullneb);
		modify(Neb2_awacs, -1.0f);
	}

	set_modified();
}

float BackgroundEditorDialogModel::getFullNebulaRange() const
{
	// May be -1 if full nebula is disabled
	return Neb2_awacs;
}

void BackgroundEditorDialogModel::setFullNebulaRange(float range)
{
	modify(Neb2_awacs, range);
}

SCP_string BackgroundEditorDialogModel::getNebulaFullPattern() const
{
	return (Neb2_texture_name[0] != '\0') ? SCP_string(Neb2_texture_name) : SCP_string("<None>");
}

void BackgroundEditorDialogModel::setNebulaFullPattern(const SCP_string& name)
{
	if (lcase_equal(name, "<None>")) {
		strcpy_s(Neb2_texture_name, "");
	} else {
		strcpy_s(Neb2_texture_name, name.c_str());
	}

	set_modified();
}

SCP_string BackgroundEditorDialogModel::getLightning() const
{
	// Return "<none>" when engine stores "none" or empty
	if (Mission_parse_storm_name[0] == '\0')
		return "<None>";
	SCP_string s = Mission_parse_storm_name;
	if (lcase_equal(s, "none"))
		return "<None>";
	return s;
}

void BackgroundEditorDialogModel::setLightning(const SCP_string& name)
{
	// Engine convention is the literal "none" for no storm
	if (lcase_equal(name, "<None>")) {
		strcpy_s(Mission_parse_storm_name, "none");
	} else {
		strcpy_s(Mission_parse_storm_name, name.c_str());
	}
	set_modified();
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getSelectedPoofs() const
{
	SCP_vector<SCP_string> out;
	for (size_t i = 0; i < Poof_info.size(); ++i) {
		if (get_bit(Neb2_poof_flags.get(), i))
			out.emplace_back(Poof_info[i].name);
	}
	return out;
}

void BackgroundEditorDialogModel::setSelectedPoofs(const SCP_vector<SCP_string>& names)
{
	// Clear all, then set matching names
	clear_all_bits(Neb2_poof_flags.get(), Poof_info.size());
	for (const auto& want : names) {
		for (size_t i = 0; i < Poof_info.size(); ++i) {
			if (!_stricmp(Poof_info[i].name, want.c_str())) {
				set_bit(Neb2_poof_flags.get(), i);
				break;
			}
		}
	}

	set_modified();
}

bool BackgroundEditorDialogModel::getShipTrailsToggled() const
{
	return The_mission.flags[Mission::Mission_Flags::Toggle_ship_trails];
}

void BackgroundEditorDialogModel::setShipTrailsToggled(bool on)
{
	The_mission.flags.set(Mission::Mission_Flags::Toggle_ship_trails, on);
	set_modified();
}

float BackgroundEditorDialogModel::getFogNearMultiplier() const
{
	return Neb2_fog_near_mult;
}

void BackgroundEditorDialogModel::setFogNearMultiplier(float v)
{
	modify(Neb2_fog_near_mult, v);
}

float BackgroundEditorDialogModel::getFogFarMultiplier() const
{
	return Neb2_fog_far_mult;
}

void BackgroundEditorDialogModel::setFogFarMultiplier(float v)
{
	modify(Neb2_fog_far_mult, v);
}

bool BackgroundEditorDialogModel::getDisplayBackgroundBitmaps() const
{
	return The_mission.flags[Mission::Mission_Flags::Fullneb_background_bitmaps];
}

void BackgroundEditorDialogModel::setDisplayBackgroundBitmaps(bool on)
{
	The_mission.flags.set(Mission::Mission_Flags::Fullneb_background_bitmaps, on);
	set_modified();
}

bool BackgroundEditorDialogModel::getFogPaletteOverride() const
{
	return The_mission.flags[Mission::Mission_Flags::Neb2_fog_color_override];
}

void BackgroundEditorDialogModel::setFogPaletteOverride(bool on)
{
	The_mission.flags.set(Mission::Mission_Flags::Neb2_fog_color_override, on);
	set_modified();
}

int BackgroundEditorDialogModel::getFogR() const
{
	return Neb2_fog_color[0];
}

void BackgroundEditorDialogModel::setFogR(int r)
{
	CLAMP(r, 0, 255)
	const ubyte v = static_cast<ubyte>(r);
	modify(Neb2_fog_color[0], v);
}

int BackgroundEditorDialogModel::getFogG() const
{
	return Neb2_fog_color[1];
}

void BackgroundEditorDialogModel::setFogG(int g)
{
	CLAMP(g, 0, 255)
	const ubyte v = static_cast<ubyte>(g);
	modify(Neb2_fog_color[1], v);
}

int BackgroundEditorDialogModel::getFogB() const
{
	return Neb2_fog_color[2];
}

void BackgroundEditorDialogModel::setFogB(int b)
{
	CLAMP(b, 0, 255)
	const ubyte v = static_cast<ubyte>(b);
	modify(Neb2_fog_color[2], v);
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getOldNebulaPatternOptions() const
{
	SCP_vector<SCP_string> out;
	out.emplace_back("<None>");
	for (auto& neb : Nebula_filenames) {
		out.emplace_back(neb);
	}
	return out;
}

SCP_vector<SCP_string> BackgroundEditorDialogModel::getOldNebulaColorOptions() const
{
	SCP_vector<SCP_string> out;
	out.reserve(NUM_NEBULA_COLORS);
	for (auto& color : Nebula_colors) {
		out.emplace_back(color);
	}
	return out;
}

SCP_string BackgroundEditorDialogModel::getOldNebulaPattern() const
{
	if (Nebula_index < 0)
		return "<None>";

	if (Nebula_index >= 0 && Nebula_index < NUM_NEBULAS) {
		return Nebula_filenames[Nebula_index];
	}

	return SCP_string{};
}

void BackgroundEditorDialogModel::setOldNebulaPattern(const SCP_string& name)
{
	int newIndex = -1;
	if (!name.empty() && stricmp(name.c_str(), "<None>") != 0) {
		for (int i = 0; i < NUM_NEBULAS; ++i) {
			if (!stricmp(Nebula_filenames[i], name.c_str())) {
				newIndex = i;
				break;
			}
		}
	}

	modify(Nebula_index, newIndex);
}

SCP_string BackgroundEditorDialogModel::getOldNebulaColorName() const
{
	if (Mission_palette >= 0 && Mission_palette < NUM_NEBULA_COLORS) {
		return Nebula_colors[Mission_palette];
	}
	return SCP_string{};
}

void BackgroundEditorDialogModel::setOldNebulaColorName(const SCP_string& name)
{
	if (name.empty())
		return;
	for (int i = 0; i < NUM_NEBULA_COLORS; ++i) {
		if (!stricmp(Nebula_colors[i], name.c_str())) {
			modify(Mission_palette, i);
			return;
		}
	}
	// name not found: ignore
}

int BackgroundEditorDialogModel::getOldNebulaPitch() const
{
	return Nebula_pitch;
}

void BackgroundEditorDialogModel::setOldNebulaPitch(int deg)
{
	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	if (Nebula_pitch != deg) {
		Nebula_pitch = deg;
		modify(Nebula_pitch, deg);
	}
}

int BackgroundEditorDialogModel::getOldNebulaBank() const
{
	return Nebula_bank;
}

void BackgroundEditorDialogModel::setOldNebulaBank(int deg)
{
	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	if (Nebula_bank != deg) {
		Nebula_bank = deg;
		modify(Nebula_bank, deg);
	}
}

int BackgroundEditorDialogModel::getOldNebulaHeading() const
{
	return Nebula_heading;
}

void BackgroundEditorDialogModel::setOldNebulaHeading(int deg)
{
	CLAMP(deg, getOrientLimit().first, getOrientLimit().second);
	if (Nebula_heading != deg) {
		Nebula_heading = deg;
		modify(Nebula_heading, deg);
	}
}

int BackgroundEditorDialogModel::getAmbientR() const
{
	return The_mission.ambient_light_level & 0xff;
}

void BackgroundEditorDialogModel::setAmbientR(int r)
{
	CLAMP(r, 1, 255);
	
	const int g = getAmbientG(), b = getAmbientB();
	const int newCol = (r) | (g << 8) | (b << 16);
	
	modify(The_mission.ambient_light_level, newCol);

	gr_set_ambient_light(r, g, b);
	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getAmbientG() const
{
	return (The_mission.ambient_light_level >> 8) & 0xff;
}

void BackgroundEditorDialogModel::setAmbientG(int g)
{
	CLAMP(g, 1, 255);

	const int r = getAmbientR(), b = getAmbientB();
	const int newCol = (r) | (g << 8) | (b << 16);

	modify(The_mission.ambient_light_level, newCol);

	gr_set_ambient_light(r, g, b);
	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getAmbientB() const
{
	return (The_mission.ambient_light_level >> 16) & 0xff;
}

void BackgroundEditorDialogModel::setAmbientB(int b)
{
	CLAMP(b, 1, 255);

	const int r = getAmbientR(), g = getAmbientG();
	const int newCol = (r) | (g << 8) | (b << 16);

	modify(The_mission.ambient_light_level, newCol);

	gr_set_ambient_light(r, g, b);
	refreshBackgroundPreview();
}

} // namespace fso::fred::dialogs