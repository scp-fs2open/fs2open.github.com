#include "FredApplication.h"
#include "BackgroundEditorDialogModel.h"

//#include "mission/missionparse.h"

// TODO move this to common for both FREDs. Do not pass review if this is not done
const static float delta = .00001f;

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
	// do the OnClose stuff here
	return true;
}

void BackgroundEditorDialogModel::reject()
{
	// do nothing?
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

} // namespace fso::fred::dialogs