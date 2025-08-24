#pragma once

#include "globalincs/pstypes.h"

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {

// Mission Specs is responsible for committing to The_mission on its own Apply or discarding on Reject
class CustomDataDialogModel : public AbstractDialogModel {
  public:
	CustomDataDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setInitial(const SCP_map<SCP_string, SCP_string>& in);

	const SCP_map<SCP_string, SCP_string>& items() const noexcept
	{
		return _items;
	}

	bool add(const std::pair<SCP_string, SCP_string>& e, SCP_string* errorOut);
	bool updateAt(size_t index, const std::pair<SCP_string, SCP_string>& e, SCP_string* errorOut);
	bool removeAt(size_t index);
	bool hasKey(const SCP_string& key) const;
	std::optional<size_t> indexOfKey(const SCP_string& key) const;

    // Validation helpers
    static bool validateKeySyntax(const SCP_string& key, SCP_string* err = nullptr);
    static bool validateValue(const SCP_string& val, SCP_string* err = nullptr);

  private:
	bool keyIsUnique(const SCP_string& key, std::optional<size_t> ignoreIndex = std::nullopt) const;

	SCP_map<SCP_string, SCP_string> _items;
};

} // namespace fso::fred::dialogs
