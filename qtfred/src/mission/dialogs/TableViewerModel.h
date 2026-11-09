#pragma once

#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {

class TableViewerModel : public AbstractDialogModel {
	SCP_string _text;

  public:
	// For a full table (e.g. music.tbl)
	TableViewerModel(QObject* parent, EditorViewport* viewport,
	                 const char* table_filename, const char* modular_pattern);
	// For a specific entry (e.g. a ship or weapon by name)
	TableViewerModel(QObject* parent, EditorViewport* viewport,
	                 const char* table_filename, const char* modular_pattern,
	                 const char* entry_name);

	bool apply() override;
	void reject() override;
	SCP_string getText() const;

  private:
	void initializeData(const char* table_filename, const char* modular_pattern, const char* entry_name);
};

} // namespace fso::fred::dialogs
