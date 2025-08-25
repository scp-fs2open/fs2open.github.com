#include "FredApplication.h"
#include "BackgroundEditorDialogModel.h"

//#include "mission/missionparse.h"

// TODO move this to common for both FREDs. Do not pass review if this is not done
const static float delta = .00001f;

namespace fso::fred::dialogs {
BackgroundEditorDialogModel::BackgroundEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	// may not need these because I don't think anything else can modify data that this dialog works with
	connect(_editor, &Editor::currentObjectChanged, this, &BackgroundEditorDialogModel::onEditorSelectionChanged);
	connect(_editor, &Editor::missionChanged, this, &BackgroundEditorDialogModel::onEditorMissionChanged);

	auto& bg = getActiveBackground();
	auto& list = bg.bitmaps;
	if (!list.empty()) {
		_selectedBitmapIndex = 0;
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

void BackgroundEditorDialogModel::onEditorSelectionChanged(int)
{
	// reload?
}

void BackgroundEditorDialogModel::onEditorMissionChanged()
{
	// reload?
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

void BackgroundEditorDialogModel::refreshBackgroundPreview()
{
	stars_load_background(Cur_background); // rebuild instances from Backgrounds[]
	if (_viewport) {
		_viewport->needsUpdate();          // schedule a repaint
	}
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
		return -1;

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
		return -1;

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
		return -1;

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
		return -1.0f;

	return bm->scale_x;
}

void BackgroundEditorDialogModel::setBitmapScaleX(float v)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(v, getScaleLimit().first, getScaleLimit().second);
	modify(bm->scale_x, v);

	refreshBackgroundPreview();
}

float BackgroundEditorDialogModel::getBitmapScaleY() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return -1.0f;

	return bm->scale_y;
}

void BackgroundEditorDialogModel::setBitmapScaleY(float v)
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return;

	CLAMP(v, getScaleLimit().first, getScaleLimit().second);
	modify(bm->scale_y, v);

	refreshBackgroundPreview();
}

int BackgroundEditorDialogModel::getBitmapDivX() const
{
	auto* bm = getActiveBitmap();
	if (!bm)
		return -1;

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
		return -1;

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

} // namespace fso::fred::dialogs