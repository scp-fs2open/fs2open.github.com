#pragma once

#include "../AbstractDialogModel.h"

#include "globalincs/pstypes.h"

namespace fso::fred::dialogs {

// Mission Specs is responsible for committing to The_mission on its own Apply or discarding on Reject
class CustomStringsDialogModel : public AbstractDialogModel {
  public:

	CustomStringsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setInitial(const SCP_vector<custom_string>& in);

	const SCP_vector<custom_string>& items() const noexcept
	{
		return _items;
	}

	bool add(const custom_string& e, SCP_string* errorOut = nullptr);
	bool updateAt(size_t index, const custom_string& e, SCP_string* errorOut = nullptr);
	bool removeAt(size_t index);

	bool hasKey(const SCP_string& key) const;
	std::optional<size_t> indexOfKey(const SCP_string& key) const;

	static bool validateKeySyntax(const SCP_string& key, SCP_string* errorOut = nullptr);
	static bool validateValue(const SCP_string& value, SCP_string* errorOut = nullptr);
	static bool validateText(const SCP_string& text, SCP_string* errorOut = nullptr);

  private:
	bool keyIsUnique(const SCP_string& key, std::optional<size_t> ignoreIndex = std::nullopt) const;

	SCP_vector<custom_string> _items;
};

} // namespace fso::fred::dialogs
