#pragma once

#include "../AbstractDialogModel.h"
#include "prop/prop.h"

namespace fso::fred::dialogs {

class ShipTextureReplacementDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	/**
	 * @param propObjnum Object number of the prop to edit, or -1 to edit the editor's current ship(s)
	 */
	ShipTextureReplacementDialogModel(QObject* parent, EditorViewport* viewport, bool multi, int propObjnum = -1);

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
	void addReplacement(const SCP_string& oldTexture, const SCP_string& newTexture);
	prop* getProp() const;
	static bool testTexture(const SCP_string& name);

	bool _multi;
	int _propObjnum;
	SCP_vector<texture_replace> _propReplacements; // a prop's new list, given to the prop only if apply() succeeds
	SCP_vector<SCP_map<SCP_string, bool>> _subTypesAvailable;
	SCP_vector<SCP_map<SCP_string, bool>> _replaceMap;
	SCP_vector<SCP_map<SCP_string, bool>> _inheritMap;
	SCP_vector<SCP_string> _defaultTextures;
	SCP_vector<SCP_map<SCP_string, SCP_string>> _currentTextures;
	bool _mainChanged = false;
};

} // namespace fso::fred::dialogs
