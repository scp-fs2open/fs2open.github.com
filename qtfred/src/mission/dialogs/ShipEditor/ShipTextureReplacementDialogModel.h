#pragma once

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {

constexpr auto NUM__SUBTEXTURE_TYPES = 7;

class ShipTextureReplacementDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	ShipTextureReplacementDialogModel(QObject* parent, EditorViewport* viewport, bool multi);

	bool apply() override;
	void reject() override;

	size_t getSize() const;
	SCP_string getDefaultName(size_t index) const;

	void setMap(size_t index, const SCP_string& type, const SCP_string& newName);
	SCP_string getMap(size_t index, const SCP_string& type) const;

	SCP_map<SCP_string, bool> getSubtypesForMap(size_t index) const;
	SCP_map<SCP_string, bool> getReplace(size_t index) const;
	SCP_map<SCP_string, bool> getInherit(size_t index) const;
	void setReplace(size_t index, const SCP_string& type, bool state);
	void setInherit(size_t index, const SCP_string& type, bool state);

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData(bool multi);
	void initSubTypes(polymodel* model, int mapNum);
	void saveSubMap(size_t index, const SCP_string& type);
	static bool testTexture(const SCP_string& name);
	static texture_replace* texture_set(texture_replace* dest, const texture_replace* src);

	bool _multi;
	SCP_vector<SCP_map<SCP_string, bool>> _subTypesAvailable;
	SCP_vector<SCP_map<SCP_string, bool>> _replaceMap;
	SCP_vector<SCP_map<SCP_string, bool>> _inheritMap;
	SCP_vector<SCP_string> _defaultTextures;
	SCP_vector<SCP_map<SCP_string, SCP_string>> _currentTextures;
	bool _mainChanged = false;
};

} // namespace fso::fred::dialogs
