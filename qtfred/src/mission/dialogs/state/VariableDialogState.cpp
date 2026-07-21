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

// ---------------------------------------------------------------------------
// Working-state capture/restore for the in-dialog undo stack: the WIP
// variable and container lists plus the deleted-variables bookkeeping.
// ---------------------------------------------------------------------------

static void writeStringVector(QDataStream& ds, const SCP_vector<SCP_string>& v)
{
	ds << static_cast<qint32>(v.size());
	for (const auto& s : v)
		ds << QString::fromStdString(s);
}

static void readStringVector(QDataStream& ds, SCP_vector<SCP_string>& v)
{
	v.clear();
	qint32 count;
	ds >> count;
	v.reserve(count);
	for (int i = 0; i < count; ++i) {
		QString s;
		ds >> s;
		v.push_back(s.toStdString());
	}
}

QByteArray VariableDialogModel::captureWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(m_variables.size());
	for (const auto& var : m_variables) {
		ds << QString::fromStdString(var.name);
		ds << QString::fromStdString(var.originalName);
		ds << static_cast<quint8>(var.is_string ? 1 : 0);
		ds << static_cast<qint32>(var.flags);
		ds << static_cast<qint32>(var.numberValue);
		ds << QString::fromStdString(var.stringValue);
	}

	ds << static_cast<qint32>(m_containers.size());
	for (const auto& cont : m_containers) {
		ds << QString::fromStdString(cont.name);
		ds << QString::fromStdString(cont.originalName);
		ds << static_cast<quint8>(cont.is_list ? 1 : 0);
		ds << static_cast<quint8>(cont.values_are_strings ? 1 : 0);
		ds << static_cast<quint8>(cont.keys_are_strings ? 1 : 0);
		ds << static_cast<qint32>(cont.flags);
		writeStringVector(ds, cont.keys);
		ds << static_cast<qint32>(cont.numberValues.size());
		for (int val : cont.numberValues)
			ds << static_cast<qint32>(val);
		writeStringVector(ds, cont.stringValues);
	}

	writeStringVector(ds, m_deleted_variables);

	return data;
}

void VariableDialogModel::restoreWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 varCount;
	ds >> varCount;
	m_variables.clear();
	m_variables.reserve(varCount);
	for (int i = 0; i < varCount; ++i) {
		auto& var = m_variables.emplace_back();
		QString name, originalName, stringValue;
		quint8 isString;
		qint32 flags, numberValue;
		ds >> name >> originalName >> isString >> flags >> numberValue >> stringValue;
		var.name         = name.toStdString();
		var.originalName = originalName.toStdString();
		var.is_string    = (isString != 0);
		var.flags        = static_cast<int>(flags);
		var.numberValue  = static_cast<int>(numberValue);
		var.stringValue  = stringValue.toStdString();
	}

	qint32 contCount;
	ds >> contCount;
	m_containers.clear();
	m_containers.reserve(contCount);
	for (int i = 0; i < contCount; ++i) {
		auto& cont = m_containers.emplace_back();
		QString name, originalName;
		quint8 isList, valuesAreStrings, keysAreStrings;
		qint32 flags;
		ds >> name >> originalName >> isList >> valuesAreStrings >> keysAreStrings >> flags;
		cont.name               = name.toStdString();
		cont.originalName       = originalName.toStdString();
		cont.is_list            = (isList != 0);
		cont.values_are_strings = (valuesAreStrings != 0);
		cont.keys_are_strings   = (keysAreStrings != 0);
		cont.flags              = static_cast<int>(flags);
		readStringVector(ds, cont.keys);
		qint32 numCount;
		ds >> numCount;
		cont.numberValues.clear();
		cont.numberValues.reserve(numCount);
		for (int j = 0; j < numCount; ++j) {
			qint32 val;
			ds >> val;
			cont.numberValues.push_back(static_cast<int>(val));
		}
		readStringVector(ds, cont.stringValues);
	}

	readStringVector(ds, m_deleted_variables);

	set_modified();
}

} // namespace fso::fred::dialogs
