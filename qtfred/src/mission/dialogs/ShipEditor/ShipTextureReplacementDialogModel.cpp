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
		// Two-pass load: collect mains first so subtype inherit detection (pass 2) compares
		// against a fully-loaded main regardless of entry ordering in Fred_texture_replacements.
		// The type is taken from old_texture's suffix (authoritative... saveSubMap writes
		// "<base>-<type>") rather than from new_texture, which can legitimately contain hyphens.
		struct DeferredSubtype {
			size_t index;
			SCP_string type;
			SCP_string newText;
		};
		SCP_vector<DeferredSubtype> deferred;

		for (const auto& tr : Fred_texture_replacements) {
			if (tr.from_table)
				continue;
			if (stricmp(Ships[_editor->cur_ship].ship_name, tr.ship_name) != 0)
				continue;

			SCP_string oldName = tr.old_texture;

			// Direct match -> main entry (old_texture is the bare base name).
			size_t matchIdx = _defaultTextures.size();
			for (size_t i = 0; i < _defaultTextures.size(); i++) {
				if (lcase_equal(_defaultTextures[i], oldName)) {
					matchIdx = i;
					break;
				}
			}
			if (matchIdx < _defaultTextures.size()) {
				_currentTextures[matchIdx]["main"] = tr.new_texture;
				continue;
			}

			// Fall back to stripping the suffix -> subtype entry, also handles old mission
			// files that stored "<base>-<type>" as old_texture.
			auto stripPos = oldName.find_last_of('-');
			if (stripPos == SCP_string::npos)
				continue;
			SCP_string suffix = oldName.substr(stripPos + 1);
			if (!is_known_subtexture_type(suffix))
				continue;
			SCP_string stripped = oldName.substr(0, stripPos);
			for (size_t i = 0; i < _defaultTextures.size(); i++) {
				if (lcase_equal(_defaultTextures[i], stripped)) {
					deferred.push_back({i, suffix, tr.new_texture});
					break;
				}
			}
		}

		for (const auto& d : deferred) {
			const SCP_string& mainName = _currentTextures[d.index]["main"];
			SCP_string inheritedName;
			if (!mainName.empty()) {
				inheritedName = (mainName != "invisible") ? mainName + "-" + d.type : mainName;
			}

			// The dialog's per-type line edit stores the raw new name without the suffix when
			// not inheriting (saveSubMap re-appends it). Strip a matching trailing suffix so a
			// non-inherited round-trip preserves what the user typed.
			SCP_string typedName = d.newText;
			SCP_string typedSuffix = "-" + d.type;
			if (typedName.size() > typedSuffix.size() &&
				lcase_equal(typedName.substr(typedName.size() - typedSuffix.size()), typedSuffix)) {
				typedName = typedName.substr(0, typedName.size() - typedSuffix.size());
			}

			_currentTextures[d.index][d.type] = typedName;
			_replaceMap[d.index][d.type] = true;
			_inheritMap[d.index][d.type] = !inheritedName.empty() && lcase_equal(d.newText, inheritedName);
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
								Assertion((objp->type == OBJ_SHIP) || (objp->type == OBJ_START), "Expected ship or start object"); // NOLINT(readability-simplify-boolean-expr)
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
		// Subtype entries are saved as old_texture = "<base>-<type>" by saveSubMap, so the standard
		// ship::apply_replacement_textures path would resolve them correctly. We instead write slots
		// directly from _currentTextures because the dialog already has inherit-from-main resolved
		// and the new bitmaps still need to be loaded. Doing both in one pass is simpler than
		// pre-populating new_texture_id on every entry just to call apply_replacement_textures.
		// After rebuilding we re-layer from_table entries (which remove_ship_entries left intact)
		// so they survive the make_shared reset of pmi->texture_replace.
		auto load_tex = [](const SCP_string& name) -> int {
			if (lcase_equal(name, "invisible"))
				return REPLACE_WITH_INVISIBLE;
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

				const SCP_string& mainName = _currentTextures[i].at("main");

				// Apply to every texture group that uses this base texture; two parts can share
				// a base bitmap and ship::apply_replacement_textures updates them all.
				for (int groupIdx = 0; groupIdx < pm->n_textures; groupIdx++) {
					if (pm->maps[groupIdx].FindTexture(_defaultTextures[i].c_str()) < 0)
						continue;

					// Main (base) slot.
					if (!mainName.empty() && !lcase_equal(mainName, _defaultTextures[i])) {
						int id = load_tex(mainName);
						if (id != -1)
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
						if (id != -1)
							(*pmi->texture_replace)[groupIdx * TM_NUM_TYPES + tmType] = id;
					}
				}
			}

			// Re-layer from_table replacements; remove_ship_entries left them in place but the
			// make_shared above wiped them from the pmi.
			for (const auto& tr : Fred_texture_replacements) {
				if (!tr.from_table)
					continue;
				if (stricmp(tr.ship_name, shipp.ship_name) != 0)
					continue;
				int id = (tr.new_texture_id != -1) ? tr.new_texture_id : load_tex(tr.new_texture);
				if (id == -1)
					continue;
				for (int j = 0; j < pm->n_textures; j++) {
					int tnum = pm->maps[j].FindTexture(tr.old_texture);
					if (tnum >= 0)
						(*pmi->texture_replace)[j * TM_NUM_TYPES + tnum] = id;
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
				Assertion((objp->type == OBJ_SHIP) || (objp->type == OBJ_START), "Expected ship or start object"); // NOLINT(readability-simplify-boolean-expr)
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
	Assertion(index < _defaultTextures.size(), "Texture index out of bounds");
	return _defaultTextures[index];
}

void ShipTextureReplacementDialogModel::setMap(size_t index, const SCP_string& type, const SCP_string& newName)
{
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
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
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
	auto pos = _currentTextures[index].find(type);
	if (pos == _currentTextures[index].end()) {
		error_display(1, "Asked for non existant map type %s. Get a programmer", type.c_str());
		return "";
	}
	else {
		return pos->second;
	}
}

SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getSubtypesForMap(size_t index) const
{
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
	return _subTypesAvailable[index];
}

SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getReplace(size_t index) const
{
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
	return _replaceMap[index];
}

SCP_map<SCP_string, bool> ShipTextureReplacementDialogModel::getInherit(size_t index) const
{
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
	return _inheritMap[index];
}

void ShipTextureReplacementDialogModel::setReplace(size_t index, const SCP_string& type, bool state)
{
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
	modify(_replaceMap[index][type], state);
}

void ShipTextureReplacementDialogModel::setInherit(size_t index, const SCP_string& type, bool state)
{
	Assertion(index < _currentTextures.size(), "Texture index out of bounds");
	modify(_inheritMap[index][type], state);
}

} // namespace fso::fred::dialogs
