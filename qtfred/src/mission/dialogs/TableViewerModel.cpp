#include "TableViewerModel.h"

#include <utils/table_viewer.h>

namespace fso::fred::dialogs {

TableViewerModel::TableViewerModel(QObject* parent, EditorViewport* viewport,
                                   const char* table_filename, const char* modular_pattern)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(table_filename, modular_pattern, nullptr);
}

TableViewerModel::TableViewerModel(QObject* parent, EditorViewport* viewport,
                                   const char* table_filename, const char* modular_pattern,
                                   const char* entry_name)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(table_filename, modular_pattern, entry_name);
}

bool TableViewerModel::apply()
{
	return true;
}
void TableViewerModel::reject() {}

void TableViewerModel::initializeData(const char* table_filename, const char* modular_pattern,
                                       const char* entry_name)
{
	if (entry_name) {
		const SCP_string msg = "Could not find table entry: " + SCP_string(entry_name) + " in table: " + SCP_string(table_filename);
		_text = table_viewer::get_table_entry_text(table_filename, modular_pattern, entry_name, msg.c_str());
	} else {
		const SCP_string msg = "Could not find table: " + SCP_string(table_filename);
		_text = table_viewer::get_complete_table_text(table_filename, modular_pattern, msg.c_str());
	}
	modelChanged();
}

SCP_string TableViewerModel::getText() const
{
	return _text;
}

} // namespace fso::fred::dialogs
