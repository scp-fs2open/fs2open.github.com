/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

// Sexp tree action logic — action execution that bridges model and UI.

#include "missioneditor/sexp_tree_actions.h"

#include "parse/sexp.h"
#include "parse/sexp_container.h"

SexpTreeActions::SexpTreeActions(SexpTreeModel& model, ISexpTreeUI& ui)
	: _model(model), _ui(ui)
{
}

// Helper: delete all UI children of a node and clear model child link
void SexpTreeActions::clear_node_children(int node_index)
{
	int child = _model.tree_nodes[node_index].child;
	if (child != -1)
		_model.free_node2(child);

	_model.tree_nodes[node_index].child = -1;

	void* h = _model.tree_nodes[node_index].handle;
	while (_ui.ui_has_children(h))
		_ui.ui_delete_item(_ui.ui_get_child_item(h));
}

// -----------------------------------------------------------------------
// Replace operations
// -----------------------------------------------------------------------

void SexpTreeActions::replace_data(const char* data, int type)
{
	int node_idx = _model.item_index;

	clear_node_children(node_idx);

	_model.set_node(node_idx, type, data);
	void* h = _model.tree_nodes[node_idx].handle;
	_ui.ui_set_item_text(h, data);
	NodeImage bmap = _model.get_data_image(node_idx);
	_ui.ui_set_item_image(h, bmap);
	_model.tree_nodes[node_idx].flags = EDITABLE;

	// check remaining data beyond replaced data for validity
	verify_and_fix_arguments(_model.tree_nodes[node_idx].parent);

	if (_model.modified)
		*_model.modified = 1;
	_ui.ui_update_help(h);
}

void SexpTreeActions::replace_variable_data(int var_idx, int type)
{
	char buf[128];

	Assert(type & SEXPT_VARIABLE);

	int node_idx = _model.item_index;

	clear_node_children(node_idx);

	// Assemble name
	sprintf(buf, "%s(%s)", Sexp_variables[var_idx].variable_name, Sexp_variables[var_idx].text);

	_model.set_node(node_idx, type, buf);
	void* h = _model.tree_nodes[node_idx].handle;
	_ui.ui_set_item_text(h, buf);
	_ui.ui_set_item_image(h, NodeImage::VARIABLE);
	_model.tree_nodes[node_idx].flags = NOT_EDITABLE;

	// check remaining data beyond replaced data for validity
	verify_and_fix_arguments(_model.tree_nodes[node_idx].parent);

	if (_model.modified)
		*_model.modified = 1;
	_ui.ui_update_help(h);
}

void SexpTreeActions::replace_container_name(const sexp_container& container)
{
	int node_idx = _model.item_index;

	clear_node_children(node_idx);

	_model.set_node(node_idx, (SEXPT_VALID | SEXPT_STRING | SEXPT_CONTAINER_NAME), container.container_name.c_str());
	void* h = _model.tree_nodes[node_idx].handle;
	_ui.ui_set_item_image(h, NodeImage::CONTAINER_NAME);
	_ui.ui_set_item_text(h, container.container_name.c_str());
	_model.tree_nodes[node_idx].flags = NOT_EDITABLE;

	if (_model.modified)
		*_model.modified = 1;
	_ui.ui_update_help(h);
}

void SexpTreeActions::replace_container_data(const sexp_container& container,
	int type,
	bool test_child_nodes,
	bool delete_child_nodes,
	bool set_default_modifier)
{
	int node_idx = _model.item_index;

	// if this is already a container of the right type, don't alter the child nodes
	if (test_child_nodes && (_model.tree_nodes[node_idx].type & SEXPT_CONTAINER_DATA)) {
		if (container.is_list()) {
			const auto* p_old_container = get_sexp_container(_model.tree_nodes[node_idx].text);

			Assertion(p_old_container != nullptr,
				"Attempt to Replace Container Data of unknown previous container %s. Please report!",
				_model.tree_nodes[node_idx].text);

			if (p_old_container->is_list()) {
				if (container.opf_type == p_old_container->opf_type) {
					delete_child_nodes = false;
					set_default_modifier = false;
				}
			}
		}
	}

	if (delete_child_nodes) {
		clear_node_children(node_idx);
	}

	_model.set_node(node_idx, type, container.container_name.c_str());
	void* h = _model.tree_nodes[node_idx].handle;
	_ui.ui_set_item_image(h, NodeImage::CONTAINER_DATA);
	_ui.ui_set_item_text(h, container.container_name.c_str());
	_model.tree_nodes[node_idx].flags = NOT_EDITABLE;

	if (set_default_modifier) {
		add_default_modifier(container);
	}

	if (_model.modified)
		*_model.modified = 1;
	_ui.ui_update_help(h);
}

void SexpTreeActions::replace_operator(const char* op)
{
	int node_idx = _model.item_index;

	clear_node_children(node_idx);

	_model.set_node(node_idx, (SEXPT_OPERATOR | SEXPT_VALID), op);
	void* h = _model.tree_nodes[node_idx].handle;
	_ui.ui_set_item_text(h, op);
	_model.tree_nodes[node_idx].flags = OPERAND;

	if (_model.modified)
		*_model.modified = 1;
	_ui.ui_update_help(h);
}

// -----------------------------------------------------------------------
// Expand/merge operations
// -----------------------------------------------------------------------

void SexpTreeActions::expand_operator(int node)
{
	int data;

	if (_model.tree_nodes[node].flags & COMBINED) {
		node = _model.tree_nodes[node].parent;
		Assert((_model.tree_nodes[node].flags & OPERAND) && (_model.tree_nodes[node].flags & EDITABLE));
	}

	if ((_model.tree_nodes[node].flags & OPERAND) && (_model.tree_nodes[node].flags & EDITABLE)) {
		Assert(_model.tree_nodes[node].type & SEXPT_OPERATOR);
		void* h = _model.tree_nodes[node].handle;
		data = _model.tree_nodes[node].child;
		Assert(data != -1 && _model.tree_nodes[data].next == -1 && _model.tree_nodes[data].child == -1);

		_ui.ui_set_item_text(h, _model.tree_nodes[node].text);
		_model.tree_nodes[node].flags = OPERAND;
		NodeImage bmap = _model.get_data_image(data);
		_model.tree_nodes[data].handle = _ui.ui_insert_item(_model.tree_nodes[data].text, bmap, h, nullptr);
		_model.tree_nodes[data].flags = EDITABLE;
		_ui.ui_expand_item(h);
	}
}

// -----------------------------------------------------------------------
// Add operations
// -----------------------------------------------------------------------

int SexpTreeActions::add_data(const char* data, int type)
{
	int node_idx = _model.item_index;

	expand_operator(node_idx);
	int node = _model.allocate_node(node_idx);
	_model.set_node(node, type, data);
	NodeImage bmap = _model.get_data_image(node);
	_model.tree_nodes[node].handle = _ui.ui_insert_item(data, bmap, _model.tree_nodes[node_idx].handle, nullptr);
	_model.tree_nodes[node].flags = EDITABLE;
	if (_model.modified)
		*_model.modified = 1;
	return node;
}

int SexpTreeActions::add_variable_data(const char* data, int type)
{
	Assert(type & SEXPT_VARIABLE);

	int node_idx = _model.item_index;

	expand_operator(node_idx);
	int node = _model.allocate_node(node_idx);
	_model.set_node(node, type, data);
	_model.tree_nodes[node].handle = _ui.ui_insert_item(data, NodeImage::VARIABLE, _model.tree_nodes[node_idx].handle, nullptr);
	_model.tree_nodes[node].flags = NOT_EDITABLE;
	if (_model.modified)
		*_model.modified = 1;
	return node;
}

int SexpTreeActions::add_container_name(const char* container_name)
{
	Assertion(container_name != nullptr, "Attempt to add null container name. Please report!");
	Assertion(get_sexp_container(container_name) != nullptr,
		"Attempt to add unknown container name %s. Please report!",
		container_name);

	int node_idx = _model.item_index;

	expand_operator(node_idx);
	int node = _model.allocate_node(node_idx);
	_model.set_node(node, (SEXPT_VALID | SEXPT_CONTAINER_NAME | SEXPT_STRING), container_name);
	_model.tree_nodes[node].handle =
		_ui.ui_insert_item(container_name, NodeImage::CONTAINER_NAME, _model.tree_nodes[node_idx].handle, nullptr);
	_model.tree_nodes[node].flags = NOT_EDITABLE;
	if (_model.modified)
		*_model.modified = 1;
	return node;
}

void SexpTreeActions::add_container_data(const char* container_name)
{
	Assertion(container_name != nullptr, "Attempt to add null container. Please report!");
	Assertion(get_sexp_container(container_name) != nullptr,
		"Attempt to add unknown container %s. Please report!",
		container_name);
	int node_idx = _model.item_index;
	int node = _model.allocate_node(node_idx);
	_model.set_node(node, (SEXPT_VALID | SEXPT_CONTAINER_DATA | SEXPT_STRING), container_name);
	_model.tree_nodes[node].handle =
		_ui.ui_insert_item(container_name, NodeImage::CONTAINER_DATA, _model.tree_nodes[node_idx].handle, nullptr);
	_model.tree_nodes[node].flags = NOT_EDITABLE;
	_model.item_index = node;
	if (_model.modified)
		*_model.modified = 1;
}

void SexpTreeActions::add_operator(const char* op, void* parent_handle)
{
	int node;

	if (_model.item_index == -1) {
		node = _model.allocate_node(-1);
		_model.set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), op);
		_model.tree_nodes[node].handle = _ui.ui_insert_item(op, NodeImage::OPERATOR, parent_handle, nullptr);

	} else {
		expand_operator(_model.item_index);
		node = _model.allocate_node(_model.item_index);
		_model.set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), op);
		_model.tree_nodes[node].handle = _ui.ui_insert_item(op, NodeImage::OPERATOR, _model.tree_nodes[_model.item_index].handle, nullptr);
	}

	_model.tree_nodes[node].flags = OPERAND;
	_model.item_index = node;
	if (_model.modified)
		*_model.modified = 1;
}

void SexpTreeActions::add_default_modifier(const sexp_container& container)
{
	sexp_list_item item;

	int type_to_use = (SEXPT_VALID | SEXPT_MODIFIER);

	if (container.is_map()) {
		if (any(container.type & ContainerType::STRING_KEYS)) {
			item.set_data("<any string>");
			type_to_use |= SEXPT_STRING;
		} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
			item.set_data("0");
			type_to_use |= SEXPT_NUMBER;
		} else {
			UNREACHABLE("Unknown map container key type %d", (int)container.type);
		}
	} else if (container.is_list()) {
		item.set_data(get_all_list_modifiers()[0].name);
		type_to_use |= SEXPT_STRING;
	} else {
		UNREACHABLE("Unknown container type %d", (int)container.type);
	}

	item.type = type_to_use;
	add_data(item.text.c_str(), item.type);
}

// -----------------------------------------------------------------------
// Compound operations
// -----------------------------------------------------------------------

void SexpTreeActions::add_or_replace_operator(int op, int replace_flag)
{
	int i, op2;

	int saved_index = _model.item_index;
	if (replace_flag) {
		if (_model.tree_nodes[_model.item_index].type & SEXPT_OPERATOR) {  // are both operators?
			op2 = get_operator_index(_model.tree_nodes[_model.item_index].text);
			Assert(op2 >= 0);
			i = _model.count_args(_model.tree_nodes[_model.item_index].child);
			if ((i >= Operators[op].min) && (i <= Operators[op].max)) {  // are old num args valid?
				while (i--)
					if (query_operator_argument_type(op2, i) != query_operator_argument_type(op, i))
						break;

				if (i < 0) {  // everything is ok, so we can keep old arguments with new operator
					_model.set_node(_model.item_index, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
					_ui.ui_set_item_text(_model.tree_nodes[_model.item_index].handle, Operators[op].text.c_str());
					_model.tree_nodes[_model.item_index].flags = OPERAND;
					return;
				}
			}
		}

		replace_operator(Operators[op].text.c_str());

	} else {
		add_operator(Operators[op].text.c_str());
	}

	// fill in all the required (minimum) arguments with default values
	for (i = 0; i < Operators[op].min; i++)
		add_default_operator(op, i);

	_ui.ui_expand_item(_model.tree_nodes[saved_index].handle);
}

int SexpTreeActions::add_default_operator(int op_index, int argnum)
{
	char buf[256];
	sexp_list_item item;

	void* saved_handle = _model.tree_nodes[_model.item_index].handle;
	int saved_index = _model.item_index;
	if (_model.get_default_value(&item, buf, op_index, argnum))
		return -1;

	if (item.type & SEXPT_OPERATOR) {
		Assert((item.op >= 0) && (item.op < (int)Operators.size()));
		add_or_replace_operator(item.op);
		_model.item_index = saved_index;

	} else {
		int sexp_var_index;
		// special case for sexps that take variables
		const int op_type = query_operator_argument_type(op_index, argnum);
		if ((op_type == OPF_VARIABLE_NAME) && ((sexp_var_index = get_index_sexp_variable_name(item.text)) >= 0)) {
			int type = SEXPT_VALID | SEXPT_VARIABLE;
			if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
				type |= SEXPT_STRING;
			} else if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
				type |= SEXPT_NUMBER;
			} else {
				Int3();
			}

			char node_text[2 * TOKEN_LENGTH + 2];
			sprintf(node_text, "%s(%s)", item.text.c_str(), Sexp_variables[sexp_var_index].text);
			add_variable_data(node_text, type);
		}
		// special case for sexps that take containers
		else if (item.type & SEXPT_CONTAINER_NAME) {
			Assertion(SexpTreeModel::is_container_name_opf_type(op_type) || op_type == OPF_DATA_OR_STR_CONTAINER,
				"Attempt to add default container name for a node of non-container type (%d). Please report!",
				op_type);
			add_container_name(item.text.c_str());
		}
		// modify-variable data type depends on type of variable being modified
		else if (Operators[op_index].value == OP_MODIFY_VARIABLE) {
			char buf2[256];
			Assert(argnum == 1);
			sexp_list_item temp_item;
			_model.get_default_value(&temp_item, buf2, op_index, 0);
			sexp_var_index = get_index_sexp_variable_name(temp_item.text);
			Assert(sexp_var_index != -1);

			int temp_type = Sexp_variables[sexp_var_index].type;
			int type = 0;
			if (temp_type & SEXP_VARIABLE_NUMBER) {
				type = SEXPT_VALID | SEXPT_NUMBER;
			} else if (temp_type & SEXP_VARIABLE_STRING) {
				type = SEXPT_VALID | SEXPT_STRING;
			} else {
				Int3();
			}
			add_data(item.text.c_str(), type);
		}
		// all other sexps and parameters
		else {
			add_data(item.text.c_str(), item.type);
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
// Validation
// -----------------------------------------------------------------------

void SexpTreeActions::verify_and_fix_arguments(int node)
{
	int op_index, arg_num, type, tmp;
	static int here_count = 0;
	sexp_list_item* list;
	sexp_list_item* ptr;
	bool is_variable_arg = false;

	if (here_count)
		return;

	here_count++;
	op_index = get_operator_index(_model.tree_nodes[node].text);
	if (op_index < 0) {
		here_count--;
		return;
	}

	tmp = _model.item_index;

	arg_num = 0;
	_model.item_index = _model.tree_nodes[node].child;
	while (_model.item_index >= 0) {
		// get listing of valid argument values for node item_index
		type = query_operator_argument_type(op_index, arg_num);
		// special case for modify-variable
		if (type == OPF_AMBIGUOUS) {
			is_variable_arg = true;
			type = _model.get_modify_variable_type(node);
		}
		if (_model.tree_nodes[_model.item_index].type & SEXPT_CONTAINER_DATA) {
			_model.item_index = _model.tree_nodes[_model.item_index].next;
			arg_num++;
			continue;
		}
		if (_model.query_restricted_opf_range(type)) {
			list = _model.get_listing_opf(type, node, arg_num);
			if (!list && (arg_num >= Operators[op_index].min)) {
				_model.free_node(_model.item_index, 1);
				_model.item_index = tmp;
				here_count--;
				return;
			}

			if (list) {
				char* text_ptr;
				char default_variable_text[TOKEN_LENGTH];
				if (_model.tree_nodes[_model.item_index].type & SEXPT_VARIABLE) {
					if (type == OPF_VARIABLE_NAME) {
						get_variable_name_from_sexp_tree_node_text(_model.tree_nodes[_model.item_index].text, default_variable_text);
						text_ptr = default_variable_text;
					} else {
						get_variable_name_from_sexp_tree_node_text(_model.tree_nodes[_model.item_index].text, default_variable_text);
						int sexp_var_index = get_index_sexp_variable_name(default_variable_text);
						bool types_match = false;
						Assert(sexp_var_index != -1);

						switch (type) {
							case OPF_NUMBER:
							case OPF_POSITIVE:
								if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
									types_match = true;
								}
								break;

							default:
								if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING) {
									types_match = true;
								}
						}

						if (types_match) {
							_model.item_index = _model.tree_nodes[_model.item_index].next;
							arg_num++;
							continue;
						} else {
							get_variable_default_text_from_variable_text(_model.tree_nodes[_model.item_index].text, default_variable_text);
							text_ptr = default_variable_text;
						}
					}
				} else {
					text_ptr = _model.tree_nodes[_model.item_index].text;
				}

				ptr = list;
				while (ptr) {
					if (!stricmp(ptr->text.c_str(), text_ptr))
						break;

					ptr = ptr->next;
				}

				if (!ptr) {  // argument isn't in list of valid choices
					if (list->op >= 0) {
						replace_operator(list->text.c_str());
					} else {
						replace_data(list->text.c_str(), list->type);
					}
				}

			} else {
				bool invalid = false;
				if (type == OPF_AMBIGUOUS) {
					if (SEXPT_TYPE(_model.tree_nodes[_model.item_index].type) == SEXPT_OPERATOR) {
						invalid = true;
					}
				} else {
					if (SEXPT_TYPE(_model.tree_nodes[_model.item_index].type) != SEXPT_OPERATOR) {
						invalid = true;
					}
				}

				if (invalid) {
					replace_data("<Invalid>", (SEXPT_STRING | SEXPT_VALID));
				}
			}

			if (_model.tree_nodes[_model.item_index].type & SEXPT_OPERATOR)
				verify_and_fix_arguments(_model.item_index);
		}

		// fix the node if it is the argument for modify-variable
		if (is_variable_arg) {
			switch (type) {
				case OPF_AMBIGUOUS:
					_model.tree_nodes[_model.item_index].type |= SEXPT_STRING;
					_model.tree_nodes[_model.item_index].type &= ~SEXPT_NUMBER;
					break;

				case OPF_NUMBER:
					_model.tree_nodes[_model.item_index].type |= SEXPT_NUMBER;
					_model.tree_nodes[_model.item_index].type &= ~SEXPT_STRING;
					break;

				default:
					Int3();
			}
		}

		_model.item_index = _model.tree_nodes[_model.item_index].next;
		arg_num++;
	}

	_model.item_index = tmp;
	here_count--;
}

// -----------------------------------------------------------------------
// Variable/container bulk operations
// -----------------------------------------------------------------------

void SexpTreeActions::delete_sexp_tree_variable(const char* var_name)
{
	char search_str[64];
	char replace_text[TOKEN_LENGTH];

	sprintf(search_str, "%s(", var_name);

	int old_item_index = _model.item_index;

	for (int idx = 0; idx < (int)_model.tree_nodes.size(); idx++) {
		if (_model.tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(_model.tree_nodes[idx].text, search_str) != nullptr) {
				Assert((_model.tree_nodes[idx].type & SEXPT_NUMBER) || (_model.tree_nodes[idx].type & SEXPT_STRING));

				int type = _model.tree_nodes[idx].type &= ~SEXPT_VARIABLE;

				if (_model.tree_nodes[idx].type & SEXPT_NUMBER) {
					strcpy_s(replace_text, "number");
				} else {
					strcpy_s(replace_text, "string");
				}

				_model.item_index = idx;
				replace_data(replace_text, type);
			}
		}
	}

	_model.item_index = old_item_index;
}

void SexpTreeActions::modify_sexp_tree_variable(const char* old_name, int sexp_var_index)
{
	char search_str[64];
	int type;

	Assert(Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_SET);
	Assert((Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) ||
	       (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING));

	if (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) {
		type = (SEXPT_NUMBER | SEXPT_VALID);
	} else {
		type = (SEXPT_STRING | SEXPT_VALID);
	}

	int old_item_index = _model.item_index;

	sprintf(search_str, "%s(", old_name);

	for (int idx = 0; idx < (int)_model.tree_nodes.size(); idx++) {
		if (_model.tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(_model.tree_nodes[idx].text, search_str) != nullptr) {
				_model.item_index = idx;
				replace_variable_data(sexp_var_index, (type | SEXPT_VARIABLE));
			}
		}
	}

	_model.item_index = old_item_index;
}

bool SexpTreeActions::rename_container_nodes(const SCP_string& old_name, const SCP_string& new_name)
{
	Assertion(!old_name.empty(),
		"Attempt to rename container nodes looking for empty name. Please report!");
	Assertion(!new_name.empty(),
		"Attempt to rename container nodes with empty name. Please report!");
	Assertion(new_name.length() <= sexp_container::NAME_MAX_LENGTH,
		"Attempt to rename container nodes with name %s that is too long (%d > %d). Please report!",
		new_name.c_str(), (int)new_name.length(), sexp_container::NAME_MAX_LENGTH);

	bool renamed_anything = false;

	for (int node_idx = 0; node_idx < (int)_model.tree_nodes.size(); node_idx++) {
		if (_model.is_matching_container_node(node_idx, old_name)) {
			strcpy_s(_model.tree_nodes[node_idx].text, new_name.c_str());
			_ui.ui_set_item_text(_model.tree_nodes[node_idx].handle, new_name.c_str());
			renamed_anything = true;
		}
	}

	return renamed_anything;
}
