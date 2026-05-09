// captureState() and restoreState() for VariableDialogModel.
// Snapshots Sexp_variables[] and the live sexp container list so that
// undo/redo can restore all variables and containers to the pre-accept state.

#include <mission/dialogs/VariableDialogModel.h>

#include <algorithm>

#include <globalincs/vmallocator.h>
#include <parse/sexp.h>
#include <parse/sexp_container.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray VariableDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	// Variables: count active entries first, then write each one
	qint32 var_count = 0;
	for (const auto& v : Sexp_variables) {
		if (!(v.type & SEXP_VARIABLE_NOT_USED))
			++var_count;
	}
	ds << var_count;
	for (const auto& v : Sexp_variables) {
		if (v.type & SEXP_VARIABLE_NOT_USED)
			continue;
		ds << static_cast<qint32>(v.type) << QString::fromLatin1(v.variable_name)
		   << QString::fromLatin1(v.text);
	}

	// Containers
	const auto& containers = get_all_sexp_containers();
	ds << static_cast<qint32>(containers.size());
	for (const auto& c : containers) {
		ds << QString::fromStdString(c.container_name);
		ds << static_cast<qint32>(static_cast<ContainerTypeInt>(c.type));

		ds << static_cast<qint32>(c.list_data.size());
		for (const auto& s : c.list_data)
			ds << QString::fromStdString(s);

		ds << static_cast<qint32>(c.map_data.size());
		for (const auto& [k, v] : c.map_data)
			ds << QString::fromStdString(k) << QString::fromStdString(v);
	}

	return data;
}

void VariableDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	// Reset all variable slots
	for (auto& v : Sexp_variables) {
		v.type             = SEXP_VARIABLE_NOT_USED;
		v.variable_name[0] = '\0';
		v.text[0]          = '\0';
	}

	qint32 var_count;
	ds >> var_count;
	for (int i = 0; i < static_cast<int>(var_count) && i < MAX_SEXP_VARIABLES; ++i) {
		qint32 type;
		QString name, text;
		ds >> type >> name >> text;

		auto& v     = Sexp_variables[i];
		v.type      = static_cast<int>(type);
		strncpy(v.variable_name, name.toLatin1().constData(), TOKEN_LENGTH - 1);
		v.variable_name[TOKEN_LENGTH - 1] = '\0';
		strncpy(v.text, text.toLatin1().constData(), TOKEN_LENGTH - 1);
		v.text[TOKEN_LENGTH - 1] = '\0';
	}

	// Containers
	qint32 cont_count;
	ds >> cont_count;

	SCP_vector<sexp_container> new_containers;
	new_containers.reserve(static_cast<size_t>(cont_count));

	for (int i = 0; i < static_cast<int>(cont_count); ++i) {
		sexp_container c;
		QString name;
		qint32 type_int;
		ds >> name >> type_int;

		c.container_name = name.toStdString();
		c.type           = static_cast<ContainerType>(static_cast<ContainerTypeInt>(type_int));

		qint32 list_count;
		ds >> list_count;
		for (int j = 0; j < static_cast<int>(list_count); ++j) {
			QString s;
			ds >> s;
			c.list_data.push_back(s.toStdString());
		}

		qint32 map_count;
		ds >> map_count;
		for (int j = 0; j < static_cast<int>(map_count); ++j) {
			QString k, v;
			ds >> k >> v;
			c.map_data[k.toStdString()] = v.toStdString();
		}

		new_containers.push_back(std::move(c));
	}

	// Rewrite sexp container references to match the restored container names.
	// apply() renamed references oldName -> newName; restoring the pre-apply
	// blob must invert that, and restoring the post-apply blob (redo after an
	// undo) must reapply it. Which direction is needed follows from which name
	// the restored container list contains.
	SCP_unordered_map<SCP_string, SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to> renames;
	for (const auto& [oldName, newName] : m_applied_container_renames) {
		// Init-capture: C++17 lambdas cannot capture structured bindings directly.
		const bool restoredHasOld = std::any_of(new_containers.begin(), new_containers.end(),
			[&oldName = oldName](const sexp_container& c) { return !stricmp(c.container_name.c_str(), oldName.c_str()); });
		if (restoredHasOld) {
			renames[newName] = oldName; // undo: live references use newName
		} else {
			renames[oldName] = newName; // redo: live references use oldName
		}
	}
	update_sexp_containers(new_containers, renames);
}

} // namespace fso::fred::dialogs
