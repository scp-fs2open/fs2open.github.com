#include "ShipTextureReplacementDialogModel.h"

#include "mission/object.h"
#include "model/model.h"

#include <algorithm>

namespace {
const SCP_vector<SCP_string>& get_replaceable_texture_types()
{
	static const SCP_vector<SCP_string> types = []() {
		SCP_vector<SCP_string> out;
		out.reserve(MODEL_TEXTURE_SUFFIXES.size());
		for (const auto& suffix : MODEL_TEXTURE_SUFFIXES) {
			out.emplace_back(suffix.second.substr(1)); // strip leading '-'
		}
		return out;
	}();
	return types;
}

bool is_known_subtexture_type(const SCP_string& type)
{
	return std::any_of(get_replaceable_texture_types().begin(),
		get_replaceable_texture_types().end(),
		[&type](const SCP_string& knownType) { return lcase_equal(type, knownType); });
}

void remove_ship_entries(const char* ship_name)
{
	Fred_texture_replacements.erase(
		std::remove_if(Fred_texture_replacements.begin(), Fred_texture_replacements.end(),
			[ship_name](const texture_replace& tr) {
				return !tr.from_table && !stricmp(tr.ship_name, ship_name);
			}),
		Fred_texture_replacements.end());
}
}

namespace fso::fred::dialogs {

ShipTextureReplacementDialogModel::ShipTextureReplacementDialogModel(QObject* parent, EditorViewport* viewport, bool multi)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(multi);
}

void ShipTextureReplacementDialogModel::initializeData(bool multi)
{
	_multi = multi;
	char texture_file[MAX_FILENAME_LEN];
	char* p = nullptr;
	int duplicate;
	polymodel* pm = model_get(Ship_info[Ships[_editor->cur_ship].ship_info_index].model_num);
	_defaultTextures.clear();
	_defaultTextures.resize(pm->n_textures);
	_currentTextures.clear();
	_currentTextures.resize(pm->n_textures);
	_subTypesAvailable.clear();
	_subTypesAvailable.resize(pm->n_textures);
	_replaceMap.clear();
	_replaceMap.resize(pm->n_textures);
	_inheritMap.clear();
	_inheritMap.resize(pm->n_textures);

	// look for textures to populate the list
	for (int i = 0; i < pm->n_textures; i++) {
		// get texture file name
		bm_get_filename(pm->maps[i].textures[0].GetOriginalTexture(), texture_file);

		// skip blank textures
		if (!strlen(texture_file))
			continue;

		// get rid of file extension
		p = strchr(texture_file, '.');
		if (p)
		{
			*p = 0;
		}

		// check for duplicate textures in list
		duplicate = -1;
		for (size_t k = 0; k < _defaultTextures.size(); k++)
		{
			if (!stricmp(_defaultTextures[k].c_str(), texture_file))
			{
				duplicate = static_cast<int>(k);
				break;
			}
		}

		if (duplicate >= 0)
			continue;

		// make old texture lowercase
		strlwr(texture_file);

		// add it to the field
		_defaultTextures[i] = texture_file;
		_currentTextures[i].insert(std::pair<SCP_string, SCP_string>("main", ""));
		//Get all Available SubTypes
		initSubTypes(pm, i);
	}

	if (!_multi) {
		for (auto& Fred_texture_replacement : Fred_texture_replacements)
		{
			if (!stricmp(Ships[_editor->cur_ship].ship_name, Fred_texture_replacement.ship_name) && !(Fred_texture_replacement.from_table))
			{
				// old_texture is stored as the bare base name by this dialog (no type suffix).
				// However, entries loaded from old mission files may have a type suffix
				// (e.g. "fenris-body-misc"), so fall back to stripping if no direct match.
				SCP_string pureName = Fred_texture_replacement.old_texture;

				// Find the matching default texture slot.
				// Try direct match first; fall back to stripping the last '-' segment
				// for old mission-file entries that stored old_texture with a type suffix.
				size_t matchIdx = _defaultTextures.size();
				for (size_t i = 0; i < _defaultTextures.size(); i++) {
					if (lcase_equal(_defaultTextures[i], pureName)) {
						matchIdx = i;
						break;
					}
				}
				if (matchIdx == _defaultTextures.size()) {
					auto stripPos = pureName.find_last_of('-');
					if (stripPos != SCP_string::npos) {
						SCP_string stripped = pureName.substr(0, stripPos);
						for (size_t i = 0; i < _defaultTextures.size(); i++) {
							if (lcase_equal(_defaultTextures[i], stripped)) {
								matchIdx = i;
								break;
							}
						}
					}
				}

				if (matchIdx < _defaultTextures.size())
				{
					size_t i = matchIdx;
					{
						SCP_string newText = Fred_texture_replacement.new_texture;
						SCP_string type;
						{
							auto npos = newText.find_last_of('-');
							if (npos != SCP_string::npos) {
								SCP_string possibleType = newText.substr(npos + 1);
								// Only treat the suffix as a type if it's a known sub-texture type.
								// Texture names themselves can contain hyphens (e.g. "fighter01-01a"),
								// so we must not blindly strip the last segment.
								if (is_known_subtexture_type(possibleType)) {
									type = possibleType;
									newText = newText.substr(0, npos);
								}
							}
						}
						if (!type.empty()) {
							if (is_known_subtexture_type(type)) {
								_currentTextures[i][type] = newText;
								_replaceMap[i][type] = true;
								_inheritMap[i][type] = !_currentTextures[i]["main"].empty() &&
									lcase_equal(newText, _currentTextures[i]["main"]);
							}
						}
						else {
							_currentTextures[i]["main"] = newText;
						}
					}
				}
			}
		}
	}
	modelChanged();
	_modified = false;
}

void ShipTextureReplacementDialogModel::initSubTypes(polymodel* model, int mapNum)
{
	for (const auto& type : get_replaceable_texture_types()) {
		_subTypesAvailable[mapNum].insert({type, false});
		_currentTextures[mapNum].insert({type, ""});
		_replaceMap[mapNum].insert({type, false});
		_inheritMap[mapNum].insert({type, true});
	}

	char subMap[MAX_FILENAME_LEN];
	for (int j = 1; j < TM_NUM_TYPES; j++) {
		bm_get_filename(model->maps[mapNum].textures[j].GetOriginalTexture(), subMap);
		char* p = strchr(subMap, '.');
		if (p)
		{
			*p = 0;
		}
		SCP_string subMapClean = subMap;
		SCP_tolower(subMapClean);
		SCP_string type;
		auto npos = subMapClean.find_last_of('-');
		if (npos != SCP_string::npos) {
			type = subMapClean.substr(npos + 1);
		}
		else {
			continue;
		}
		if (!type.empty()) {
			if (type == MODEL_TEXTURE_SUFFIX_TRANS.substr(1)) {
				// transparency map, not a replaceable subtype
			} else {
				if (is_known_subtexture_type(type)) {
					_subTypesAvailable[mapNum][type] = true;
				} else {
					error_display(1, "Invalid Map type %s. Check your model's texture names or get a programmer", type.c_str());
				}
			}
		}
	}
}

bool ShipTextureReplacementDialogModel::apply()
{
	if (query_modified()) {
		_mainChanged = false;

		// Remove all existing non-table entries for affected ships before writing new ones.
		// This ensures that each save operation is a simple append rather than a remove-and-add,
		// which was causing each type's save to clobber the previous type's entry.
		if (!_multi) {
			remove_ship_entries(Ships[_editor->cur_ship].ship_name);
		} else {
			object* objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
					(objp->flags[Object::Object_Flags::Marked])) {
					remove_ship_entries(Ships[get_ship_from_obj(objp)].ship_name);
				}
				objp = GET_NEXT(objp);
			}
		}

		for (size_t i = 0; i < getSize(); i++) {
			if ((!_currentTextures[i]["main"].empty()) && (_currentTextures[i]["main"] != _defaultTextures[i])) {
				_mainChanged = true;
				SCP_string name = _currentTextures[i]["main"];
				if (testTexture(name)) {
					if (!_multi) {
						texture_replace tr;
						strcpy_s(tr.old_texture, _defaultTextures[i].c_str());
						strcpy_s(tr.new_texture, name.c_str());
						strcpy_s(tr.ship_name, Ships[_editor->cur_ship].ship_name);
						tr.new_texture_id = -1;
						tr.from_table = false;
						Fred_texture_replacements.push_back(tr);
						_editor->missionChanged();
					}
					else {
						object* objp = GET_FIRST(&obj_used_list);
						while (objp != END_OF_LIST(&obj_used_list)) {
							if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
								(objp->flags[Object::Object_Flags::Marked])) {
								Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));
								auto shipp = &Ships[get_ship_from_obj(objp)];
								texture_replace tr;
								strcpy_s(tr.old_texture, _defaultTextures[i].c_str());
								strcpy_s(tr.new_texture, name.c_str());
								strcpy_s(tr.ship_name, shipp->ship_name);
								tr.new_texture_id = -1;
								tr.from_table = false;
								Fred_texture_replacements.push_back(tr);
								_editor->missionChanged();
							}
							objp = GET_NEXT(objp);
						}
					}
				}
				else {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Missing Texture", "FRED was unable to find Main Texture %s \n Aborting at this point",
						{ DialogButton::Ok });
					if (button == DialogButton::Ok) {
						return false;
					}
				}
			}
			for (const auto& type : get_replaceable_texture_types()) {
				saveSubMap(i, type);
			}
			_editor->missionChanged();
		}
		_editor->missionChanged();

		// Update each affected ship's pmi so the viewport reflects the new textures immediately.
		// We cannot use ship::apply_replacement_textures here because all our Fred_texture_replacements
		// entries share old_texture = base name, so FindTexture always resolves to TM_BASE_TYPE and
		// sub-type entries would overwrite the base slot. Instead, write slots directly using the
		// TM_* indices from MODEL_TEXTURE_SUFFIXES.
		auto load_tex = [](const SCP_string& name) -> int {
			int id = bm_load(name);
			if (id < 0) {
				int nf, fps;
				id = bm_load_animation(name.c_str(), &nf, &fps, nullptr, nullptr, false, true);
			}
			return id;
		};

		auto apply_to_ship = [&](int ship_index) {
			auto& shipp = Ships[ship_index];
			auto* pmi = model_get_instance(shipp.model_instance_num);
			auto* pm = model_get(Ship_info[shipp.ship_info_index].model_num);

			pmi->texture_replace = std::make_shared<model_texture_replace>();

			for (size_t i = 0; i < getSize(); i++) {
				if (_defaultTextures[i].empty())
					continue;

				// Find which texture group owns this base texture.
				int groupIdx = -1;
				for (int j = 0; j < pm->n_textures; j++) {
					if (pm->maps[j].FindTexture(_defaultTextures[i].c_str()) >= 0) {
						groupIdx = j;
						break;
					}
				}
				if (groupIdx < 0)
					continue;

				// Main (base) slot.
				const SCP_string& mainName = _currentTextures[i].at("main");
				if (!mainName.empty() && !lcase_equal(mainName, _defaultTextures[i])) {
					int id = load_tex(mainName);
					if (id >= 0)
						(*pmi->texture_replace)[groupIdx * TM_NUM_TYPES + TM_BASE_TYPE] = id;
				}

				// Sub-type slots.
				for (const auto& [tmType, suffix] : MODEL_TEXTURE_SUFFIXES) {
					const SCP_string typeKey = suffix.substr(1);
					if (!_replaceMap[i].at(typeKey))
						continue;

					SCP_string fullName;
					if (_inheritMap[i].at(typeKey)) {
						if (mainName.empty() || lcase_equal(mainName, _defaultTextures[i]))
							continue;
						fullName = (mainName != "invisible") ? mainName + suffix : mainName;
					} else {
						const SCP_string& typeName = _currentTextures[i].at(typeKey);
						if (typeName.empty())
							continue;
						fullName = (typeName != "invisible") ? typeName + suffix : typeName;
					}

					int id = load_tex(fullName);
					if (id >= 0)
						(*pmi->texture_replace)[groupIdx * TM_NUM_TYPES + tmType] = id;
				}
			}
		};

		if (!_multi) {
			apply_to_ship(_editor->cur_ship);
		} else {
			object* objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
					objp->flags[Object::Object_Flags::Marked])
					apply_to_ship(get_ship_from_obj(objp));
				objp = GET_NEXT(objp);
			}
		}

		return true;
	}
	else {
		return true;
	}
}

void ShipTextureReplacementDialogModel::reject() {}

void ShipTextureReplacementDialogModel::saveSubMap(size_t index, const SCP_string& type)
{
	if (!_replaceMap[index][type])
		return;

	SCP_string fullName;
	if (_inheritMap[index][type]) {
		if (!_mainChanged)
			return;
		if (_currentTextures[index]["main"].empty())
			return;
		fullName = (_currentTextures[index]["main"] != "invisible")
			? _currentTextures[index]["main"] + "-" + type
			: _currentTextures[index]["main"];
	} else {
		if (_currentTextures[index][type].empty())
			return;
		fullName = (_currentTextures[index][type] != "invisible")
			? _currentTextures[index][type] + "-" + type
			: _currentTextures[index][type];
	}

	if (!testTexture(fullName)) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Missing Texture",
			"FRED was unable to find %s \n Skipping", { DialogButton::Ok });
		return;
	}

	// Existing entries for this ship were already removed at the start of apply(),
	// so just append the new entry for each affected ship.
	auto push_entry = [&](const char* ship_name) {
		texture_replace tr;
		strcpy_s(tr.old_texture, (_defaultTextures[index] + "-" + type).c_str());
		strcpy_s(tr.new_texture, fullName.c_str());
		strcpy_s(tr.ship_name, ship_name);
		tr.new_texture_id = -1;
		tr.from_table = false;
		Fred_texture_replacements.push_back(tr);
		_editor->missionChanged();
	};

	if (!_multi) {
		push_entry(Ships[_editor->cur_ship].ship_name);
	} else {
		object* objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));
				push_entry(Ships[get_ship_from_obj(objp)].ship_name);
			}
			objp = GET_NEXT(objp);
		}
	}
}

bool ShipTextureReplacementDialogModel::testTexture(const SCP_string& fullName)
{
	int temp_bmp, temp_frames, temp_fps;
	if (fullName == "invisible") {
		return true;
	}
	else {
		// try loading the texture (bmpman should take care of eventually unloading it)
		temp_bmp = bm_load(fullName);
		if (temp_bmp < 0)
		{
			temp_bmp = bm_load_animation(fullName.c_str(), &temp_frames, &temp_fps, nullptr, nullptr, false, true);
		}
		return temp_bmp >= 0;
	}
}

size_t ShipTextureReplacementDialogModel::getSize() const
{
	return _defaultTextures.size();
}

SCP_string ShipTextureReplacementDialogModel::getDefaultName(size_t index) const
{
	Assert(index <= _defaultTextures.size());
	return _defaultTextures[index];
}

void ShipTextureReplacementDialogModel::setMap(size_t index, const SCP_string& type, const SCP_string& newName)
{
	Assert(index < _currentTextures.size());
	auto pos = _currentTextures[index].find(type);
	if (pos == _currentTextures[index].end()) {
		error_display(1, "Tried to set non existant map type %s. Get a programmer", type.c_str());
	}
	else {
		modify(_currentTextures[index][type], newName);
	}
}

SCP_string ShipTextureReplacementDialogModel::getMap(size_t index, const SCP_string& type) const
{
	Assert(index < _currentTextures.size());
	auto pos = _currentTextures[index].find(type);
	if (pos == _currentTextures[index].end()) {
		error_display(1, "Asked for non existant map type %s. Get a programmer", type.c_str());
		return nullptr;
	}
	else {
		return pos->second;
	}
}

SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getSubtypesForMap(size_t index) const
{
	Assert(index < _currentTextures.size());
	return _subTypesAvailable[index];
}

SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getReplace(size_t index) const
{
	Assert(index < _currentTextures.size());
	return _replaceMap[index];
}

SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getInherit(size_t index) const
{
	Assert(index < _currentTextures.size());
	return _inheritMap[index];
}

void ShipTextureReplacementDialogModel::setReplace(size_t index, const SCP_string& type, bool state)
{
	Assert(index < _currentTextures.size());
	modify(_replaceMap[index][type], state);
}

void ShipTextureReplacementDialogModel::setInherit(size_t index, const SCP_string& type, bool state)
{
	Assert(index < _currentTextures.size());
	modify(_inheritMap[index][type], state);
}

} // namespace fso::fred::dialogs
