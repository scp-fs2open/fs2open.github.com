#include "missioneditor/sexp_tree_actions.h"
#include "missioneditor/sexp_annotation_model.h"

#include "parse/sexp.h"
#include "parse/sexp_container.h"

// Sexp tree action logic — action execution that bridges model and UI.

// Construct with references to the shared model (tree node data) and the
// UI callback interface (widget updates). Both must outlive this object.
SexpTreeActions::SexpTreeActions(SexpTreeModel& model, ISexpTreeUI& ui)
	: _model(model), _ui(ui)
{
}

// Free all children of a node from both the model and the UI widget.
// After this call, the node has no children in either layer.
void SexpTreeActions::clear_node_children(int node_index)
{
	Assertion(node_index >= 0, "clear_node_children called with invalid node_index %d", node_index);

	int child = _model.tree_nodes[node_index].child;
	if (child > -1)
		_model.free_node2(child);

	_model.tree_nodes[node_index].child = -1;

	void* h = _model.tree_nodes[node_index].handle;
	if (h == nullptr) {
		return;
	}

	while (_ui.ui_has_children(h))
		_ui.ui_delete_item(_ui.ui_get_child_item(h));
}

// -----------------------------------------------------------------------
// Replace operations
// -----------------------------------------------------------------------

// Replace the currently selected node (item_index) with plain data.
// Clears any existing children, sets the node type/text, updates the widget icon and text,
// marks the node as EDITABLE, then walks subsequent siblings to verify argument validity.
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

// Replace the currently selected node with a variable reference.
// Builds "varname(value)" display text from the global Sexp_variables table,
// sets the VARIABLE icon, marks NOT_EDITABLE, then verifies subsequent arguments.
void SexpTreeActions::replace_variable_data(int var_idx, int type)
{
	char buf[128];

	Assertion(type & SEXPT_VARIABLE, "Invalid variable type 0x%x (SEXPT_VARIABLE bit not set) for var_idx %d", type, var_idx);

	int node_idx = _model.item_index;

	clear_node_children(node_idx);

	// Assemble name
	snprintf(buf, sizeof(buf), "%s(%s)", Sexp_variables[var_idx].variable_name, Sexp_variables[var_idx].text);

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

// Replace the currently selected node with a container name reference.
// Clears children, sets CONTAINER_NAME type flags, updates the icon and text,
// and marks the node as NOT_EDITABLE (container names can only be changed via menus).
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

// Replace the currently selected node with a container data reference.
// If test_child_nodes is true and the node is already a container data node of the
// same list type, child nodes (modifiers) are preserved. Otherwise, if delete_child_nodes
// is true, all children are cleared. If set_default_modifier is true, a default modifier
// (map key or list accessor) is added as the first child.
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

// Replace the currently selected node with a new operator.
// Clears all children (the caller is responsible for adding new arguments),
// sets the OPERATOR type, and marks the node as OPERAND.
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

// Expand a COMBINED operator+data display into separate operator and child nodes.
// When an operator has a single data argument, FRED can display it in condensed form
// ("operator data" on one line with COMBINED flag). This function expands it back to
// the normal tree structure so additional children can be added.
// If the node itself has the COMBINED flag, walks up to the parent operator first.
void SexpTreeActions::expand_operator(int node)
{
	if (_model.tree_nodes[node].flags & COMBINED) {
		node = _model.tree_nodes[node].parent;
		Assertion(_model.tree_nodes[node].flags & OPERAND, "Parent of COMBINED node %d missing OPERAND flag (flags 0x%x, text '%s')", node, _model.tree_nodes[node].flags, _model.tree_nodes[node].text);
		Assertion(_model.tree_nodes[node].flags & EDITABLE, "Parent of COMBINED node %d missing EDITABLE flag (flags 0x%x, text '%s')", node, _model.tree_nodes[node].flags, _model.tree_nodes[node].text);
	}

	if ((_model.tree_nodes[node].flags & OPERAND) && (_model.tree_nodes[node].flags & EDITABLE)) {
		Assertion(_model.tree_nodes[node].type & SEXPT_OPERATOR, "Node %d marked OPERAND+EDITABLE but type 0x%x lacks SEXPT_OPERATOR (text '%s')", node, _model.tree_nodes[node].type, _model.tree_nodes[node].text);
		void* h = _model.tree_nodes[node].handle;
		int data = _model.tree_nodes[node].child;
		Assertion(data != -1, "Node %d ('%s') has no child to expand", node, _model.tree_nodes[node].text);
		Assertion(_model.tree_nodes[data].next == -1, "Child %d of node %d unexpectedly has a sibling (next %d)", data, node, _model.tree_nodes[data].next);
		Assertion(_model.tree_nodes[data].child == -1, "Child %d of node %d unexpectedly has its own children (child %d)", data, node, _model.tree_nodes[data].child);

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

// Add a plain data child (number or string) under the current node.
// First expands the operator if it was in COMBINED form, then allocates a new
// child node, creates the UI widget item, and marks it EDITABLE.
// Returns the index of the newly created node.
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

// Add a variable reference child under the current node.
// The data string should be in "varname(value)" format. Sets the VARIABLE icon
// and marks NOT_EDITABLE (variables must be changed via the replace-variable menu).
// Returns the index of the newly created node.
int SexpTreeActions::add_variable_data(const char* data, int type)
{
	Assertion(type & SEXPT_VARIABLE, "Invalid variable type 0x%x (SEXPT_VARIABLE bit not set) for data '%s'", type, data ? data : "(null)");

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

// Add a container name reference child under the current node.
// Validates that the container exists, sets CONTAINER_NAME type flags,
// and marks NOT_EDITABLE. Returns the index of the newly created node.
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

// Add a container data reference child under the current node.
// Unlike add_container_name, this creates a node that can have modifier children
// (e.g. map key or list index). Updates item_index to the new node so that
// subsequent calls (like add_default_modifier) operate on it.
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

// Add an operator child under the current node, or as a new root if item_index == -1.
// For root insertion (labeled-root trees), parent_handle specifies the UI parent.
// Expands the current operator if needed, creates the node with the OPERATOR icon,
// and sets item_index to the new node so default arguments can be added after.
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

// Add the default first modifier child for a container data node.
// For map containers: adds a string key placeholder ("<any string>") or number "0".
// For list containers: adds the first list modifier name (e.g. "at").
// Uses add_data internally, so item_index must point to the container data node.
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
			UNREACHABLE("Unknown map container key type %d", static_cast<int>(container.type));
		}
	} else if (container.is_list()) {
		item.set_data(get_all_list_modifiers()[0].name);
		type_to_use |= SEXPT_STRING;
	} else {
		UNREACHABLE("Unknown container type %d", static_cast<int>(container.type));
	}

	item.type = type_to_use;
	add_data(item.text.c_str(), item.type);
}

// -----------------------------------------------------------------------
// Compound operations
// -----------------------------------------------------------------------

// Add or replace an operator, then fill in all minimum required default arguments.
// If replacing an existing operator and the old arguments are type-compatible with
// the new operator (same count and matching OPF types), the old arguments are preserved.
// Otherwise, children are cleared and rebuilt with defaults via add_default_operator.
void SexpTreeActions::add_or_replace_operator(int op, int replace_flag)
{
	int saved_index = _model.item_index;
	if (replace_flag) {
		if (_model.tree_nodes[_model.item_index].type & SEXPT_OPERATOR) {  // are both operators?
			int op2 = get_operator_index(_model.tree_nodes[_model.item_index].text);
			Assertion(op2 >= 0, "Invalid operator index %d for operator text '%s'", op2, _model.tree_nodes[_model.item_index].text);
			int i = _model.count_args(_model.tree_nodes[_model.item_index].child);
			if ((i >= Operators[op].min) && (i <= Operators[op].max)) {  // are old num args valid?
				// Walk the argument list backwards comparing OPF types. The loop
				// exits early on the first mismatch; otherwise i reaches -1, meaning
				// every existing argument is type-compatible with the new operator.
				while (i--)
					if (query_operator_argument_type(op2, i) != query_operator_argument_type(op, i))
						break;

				// Type-compatible path: keep the existing argument subtree, just rewrite
				// the operator text in place. We deliberately do NOT add defaults or call
				// ui_expand_item below. The children (and their current expansion state)
				// are already correct, so preserving them avoids collapsing branches the
				// user has expanded.
				if (i < 0) {
					_model.set_node(_model.item_index, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
					_ui.ui_set_item_text(_model.tree_nodes[_model.item_index].handle, Operators[op].text.c_str());
					_model.tree_nodes[_model.item_index].flags = OPERAND;
					return;
				}
			}
		}

		// Type-incompatible (or non-operator) path: replace_operator() wipes the existing
		// children, so we fall through to repopulate defaults and expand below.
		replace_operator(Operators[op].text.c_str());

	} else {
		add_operator(Operators[op].text.c_str());
	}

	// fill in all the required (minimum) arguments with default values, then expand
	// the operator so the newly-added children are visible
	for (int i = 0; i < Operators[op].min; i++)
		add_default_operator(op, i);

	_ui.ui_expand_item(_model.tree_nodes[saved_index].handle);
}

// Add a single default argument for position 'argnum' of the operator at 'op_index'.
// Queries get_default_value for the appropriate default, then dispatches to the right
// add method based on the value type: add_or_replace_operator for operator defaults,
// add_variable_data for OPF_VARIABLE_NAME arguments, add_container_name for container
// arguments, and add_data for everything else.
// Returns 0 on success, -1 if no default value was available.
int SexpTreeActions::add_default_operator(int op_index, int argnum)
{
	sexp_list_item item;

	int saved_index = _model.item_index;
	if (_model._opf.get_default_value(&item, op_index, argnum))
		return -1;

	if (item.type & SEXPT_OPERATOR) {
		Assertion(SCP_vector_inbounds(Operators, item.op), "Invalid operator index %d (Operators size " SIZE_T_ARG ")", item.op, Operators.size());
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
				Assertion(false, "Unexpected sexp variable type %d for variable '%s'",
					Sexp_variables[sexp_var_index].type, item.text.c_str());
			}

			char node_text[SEXP_TREE_NODE_TEXT_SIZE];
			snprintf(node_text, sizeof(node_text), "%s(%s)", item.text.c_str(), Sexp_variables[sexp_var_index].text);
			add_variable_data(node_text, type);
		}
		// special case for sexps that take containers
		else if (item.type & SEXPT_CONTAINER_NAME) {
			Assertion(SexpTreeOPF::is_container_name_opf_type(op_type) || op_type == OPF_DATA_OR_STR_CONTAINER,
				"Attempt to add default container name for a node of non-container type (%d). Please report!",
				op_type);
			add_container_name(item.text.c_str());
		}
		// modify-variable data type depends on type of variable being modified
		else if (Operators[op_index].value == OP_MODIFY_VARIABLE) {
			Assertion(argnum == 1, "Invalid argument number %d for modify-variable default (expected 1)", argnum);
			sexp_list_item temp_item;
			_model._opf.get_default_value(&temp_item, op_index, 0);
			sexp_var_index = get_index_sexp_variable_name(temp_item.text);
			Assertion(sexp_var_index != -1, "Invalid variable index for modify-variable default; lookup of '%s' failed", temp_item.text.c_str());

			int temp_type = Sexp_variables[sexp_var_index].type;
			int type = 0;
			if (temp_type & SEXP_VARIABLE_NUMBER) {
				type = SEXPT_VALID | SEXPT_NUMBER;
			} else if (temp_type & SEXP_VARIABLE_STRING) {
				type = SEXPT_VALID | SEXPT_STRING;
			} else {
				Assertion(false, "Unexpected sexp variable type %d for modify-variable default", temp_type);
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

int SexpTreeActions::insert_operator(int op, void* root_parent_handle)
{
	Assertion(SCP_vector_inbounds(Operators, op), "Invalid operator index %d (Operators size " SIZE_T_ARG ")", op, Operators.size());
	Assertion(_model.item_index >= 0, "Invalid selected node index %d", _model.item_index);

	const int wrapped_node = _model.item_index;
	const int parent_node = _model.tree_nodes[wrapped_node].parent;
	const int node_flags = _model.tree_nodes[wrapped_node].flags;
	void* wrapped_handle = _model.tree_nodes[wrapped_node].handle;

	const int node = _model.allocate_node(parent_node, wrapped_node);
	_model.set_node(node, (SEXPT_OPERATOR | SEXPT_VALID), Operators[op].text.c_str());
	_model.tree_nodes[node].flags = node_flags;

	void* parent_handle = nullptr;
	if (parent_node >= 0) {
		parent_handle = _model.tree_nodes[parent_node].handle;
	} else {
		if (_model._interface && _model._interface->getFlags()[TreeFlags::LabeledRoot]) {
			parent_handle = root_parent_handle;
			_model._interface->onRootInserted(wrapped_node, node);

			// a root label's annotation is keyed to its formula node; re-key it
			// so it follows the event across the formula change
			if (_model.annotation_model) {
				auto* ea = _model.annotation_model->getByKey(SexpAnnotationModel::rootKey(wrapped_node));
				if (ea)
					ea->node_index = SexpAnnotationModel::rootKey(node);
			}
		} else {
			_model.root_item = node;
		}
	}

	_model.tree_nodes[node].handle = _ui.ui_insert_item(Operators[op].text.c_str(), NodeImage::OPERATOR, parent_handle, wrapped_handle);

	_ui.ui_move_branch(wrapped_node, node);
	_model.item_index = node;
	for (int i = 1; i < Operators[op].min; i++) {
		add_default_operator(op, i);
	}

	_ui.ui_expand_item(_model.tree_nodes[node].handle);
	if (_model.modified) {
		*_model.modified = 1;
	}

	return node;
}

int SexpTreeActions::add_or_replace_typed_data(int data_idx, bool replace, int add_count, int replace_count)
{
	Assertion(_model.item_index >= 0, "Invalid item index %d", _model.item_index);
	const int op_node = replace ? _model.tree_nodes[_model.item_index].parent : _model.item_index;
	Assertion(op_node >= 0, "Invalid operator node %d (item_index %d, replace %d)", op_node, _model.item_index, static_cast<int>(replace));

	sexp_list_item* list = nullptr;
	if (_model.tree_nodes[op_node].type & SEXPT_CONTAINER_DATA) {
		if (replace && replace_count == 0) {
			list = _model._opf.get_container_modifiers(op_node);
			Assertion(list, "get_container_modifiers returned null for container data node %d ('%s')", op_node, _model.tree_nodes[op_node].text);
		} else {
			list = _model._opf.get_container_multidim_modifiers(op_node);
			Assertion(list, "get_container_multidim_modifiers returned null for container data node %d ('%s')", op_node, _model.tree_nodes[op_node].text);
		}
	} else {
		const int op = get_operator_index(_model.tree_nodes[op_node].text);
		Assertion(op >= 0, "Invalid operator index %d for operator text '%s'", op, _model.tree_nodes[op_node].text);
		const auto argcount = replace ? replace_count : add_count;
		const auto type = query_operator_argument_type(op, argcount);
		list = _model._opf.get_listing_opf(type, op_node, argcount);
		Assertion(list, "get_listing_opf returned null for operator '%s' (op %d), OPF type %d, argument %d", _model.tree_nodes[op_node].text, op, type, argcount);
	}

	auto* ptr = list;
	const int requested_idx = data_idx;
	while (data_idx) {
		data_idx--;
		ptr = ptr->next;
		Assertion(ptr, "SEXP list ran out before reaching requested index %d (op_node %d, %d steps remaining)", requested_idx, op_node, data_idx + 1);
	}

	Assertion(SEXPT_TYPE(ptr->type) != SEXPT_OPERATOR, "Expected non-operator list item, got SEXPT_OPERATOR (ptr->op %d, text '%s')", ptr->op, ptr->text.c_str());
	Assertion(ptr->op < 0, "Expected ptr->op < 0 for non-operator list item, got %d (text '%s', type 0x%x)", ptr->op, ptr->text.c_str(), ptr->type);
	expand_operator(_model.item_index);
	int added_node = -1;
	if (replace) {
		replace_data(ptr->text.c_str(), ptr->type);
	} else {
		added_node = add_data(ptr->text.c_str(), ptr->type);
	}
	list->destroy();
	return added_node;
}

void SexpTreeActions::replace_variable_with_type_validation(int var_idx, int current_node_type, bool allow_type_coercion)
{
	Assertion(_model.item_index >= 0, "Invalid item index %d", _model.item_index);
	Assertion((var_idx >= 0) && (var_idx < MAX_SEXP_VARIABLES), "Invalid variable index %d (max %d)", var_idx, MAX_SEXP_VARIABLES);
	Assertion((current_node_type & SEXPT_NUMBER) || (current_node_type & SEXPT_STRING), "Invalid node type 0x%x (expected SEXPT_NUMBER or SEXPT_STRING) for var_idx %d", current_node_type, var_idx);

	int resolved_type = current_node_type;
	if (allow_type_coercion) {
		if (Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER) {
			resolved_type = SEXPT_NUMBER;
		} else if (Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING) {
			resolved_type = SEXPT_STRING;
		} else {
			Assertion(false, "Unexpected sexp variable type %d for variable index %d ('%s')", Sexp_variables[var_idx].type, var_idx, Sexp_variables[var_idx].variable_name);
		}
	} else {
		if (resolved_type & SEXPT_NUMBER) {
			Assertion(Sexp_variables[var_idx].type & SEXP_VARIABLE_NUMBER, "Variable '%s' (idx %d) has type %d, expected SEXP_VARIABLE_NUMBER for non-coercing replace", Sexp_variables[var_idx].variable_name, var_idx, Sexp_variables[var_idx].type);
		}
		if (resolved_type & SEXPT_STRING) {
			Assertion(Sexp_variables[var_idx].type & SEXP_VARIABLE_STRING, "Variable '%s' (idx %d) has type %d, expected SEXP_VARIABLE_STRING for non-coercing replace", Sexp_variables[var_idx].variable_name, var_idx, Sexp_variables[var_idx].type);
		}
	}

	replace_variable_data(var_idx, (resolved_type | SEXPT_VARIABLE));
}

// -----------------------------------------------------------------------
// Validation
// -----------------------------------------------------------------------

// Walk through all arguments of the operator at 'node' and verify each is valid
// for its expected OPF type. For each argument:
// - Gets the valid value listing for the argument's OPF type
// - If the current value isn't in the listing, replaces it with the first valid option
// - If no listing exists and the argument is beyond the operator minimum, removes it
// - For variable arguments, checks that the variable type matches the expected data type
// - Recurses into child operators to validate their arguments too
// Uses _verify_arguments_reentry_guard to prevent infinite recursion.
void SexpTreeActions::verify_and_fix_arguments(int node)
{
	if (_verify_arguments_reentry_guard)
		return;

	_verify_arguments_reentry_guard++;
	int op_index = get_operator_index(_model.tree_nodes[node].text);
	if (op_index < 0) {
		_verify_arguments_reentry_guard--;
		return;
	}

	// snapshot item_index so we can restore it after walking the argument list
	int tmp = _model.item_index;

	int arg_num = 0;
	_model.item_index = _model.tree_nodes[node].child;
	while (_model.item_index >= 0) {
		bool is_variable_arg = false;
		// determine the OPF type this argument slot expects
		int type = query_operator_argument_type(op_index, arg_num);
		// modify-variable's value argument has no fixed OPF type; resolve it
		// from the type of the variable being modified (NUMBER vs STRING/AMBIGUOUS)
		if (type == OPF_AMBIGUOUS) {
			is_variable_arg = true;
			type = _model.get_modify_variable_type(node);
		}
		// container data nodes carry their own typing... skip validation for them
		if (_model.tree_nodes[_model.item_index].type & SEXPT_CONTAINER_DATA) {
			_model.item_index = _model.tree_nodes[_model.item_index].next;
			arg_num++;
			continue;
		}
		if (SexpTreeModel::query_restricted_opf_range(type)) {
			// build the listing of valid values for this argument slot
			sexp_list_item* list = _model._opf.get_listing_opf(type, node, arg_num);
			// no valid values AND this argument is past the operator's minimum
			// argument count -> just drop the argument
			if (!list && (arg_num >= Operators[op_index].min)) {
				_model.free_node(_model.item_index, 1);
				_model.item_index = tmp;
				_verify_arguments_reentry_guard--;
				return;
			}

			if (list) {
				// figure out what text to look up in the listing
				char* text_ptr;
				char default_variable_text[TOKEN_LENGTH];
				if (_model.tree_nodes[_model.item_index].type & SEXPT_VARIABLE) {
					if (type == OPF_VARIABLE_NAME) {
						// slot wants a variable name; compare against the bare name
						get_variable_name_from_sexp_tree_node_text(_model.tree_nodes[_model.item_index].text, default_variable_text);
						text_ptr = default_variable_text;
					} else {
						// slot wants a typed value but the node IS a variable...
						// allow it through if the variable's type matches the slot's
						// expected data type
						get_variable_name_from_sexp_tree_node_text(_model.tree_nodes[_model.item_index].text, default_variable_text);
						int sexp_var_index = get_index_sexp_variable_name(default_variable_text);
						bool types_match = false;
						Assertion(sexp_var_index != -1, "Invalid variable index: lookup of '%s' (from node text '%s') failed", default_variable_text, _model.tree_nodes[_model.item_index].text);

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
							// variable type is compatible, accept as-is and move on
							list->destroy();
							list = nullptr;
							_model.item_index = _model.tree_nodes[_model.item_index].next;
							arg_num++;
							continue;
						} else {
							// fall back to the variable's default text for the lookup;
							// if it isn't in the listing it'll be replaced below
							get_variable_default_text_from_variable_text(_model.tree_nodes[_model.item_index].text, default_variable_text);
							text_ptr = default_variable_text;
						}
					}
				} else {
					text_ptr = _model.tree_nodes[_model.item_index].text;
				}

				// search the listing for an entry matching the current node's text
				sexp_list_item* ptr = list;
				while (ptr) {
					if (!stricmp(ptr->text.c_str(), text_ptr))
						break;

					ptr = ptr->next;
				}

				if (!ptr) {
					// current value isn't a valid choice for this slot
					// replace it with the first option from the listing
					if (list->op >= 0) {
						replace_operator(list->text.c_str());
					} else {
						replace_data(list->text.c_str(), list->type);
					}
				}

			} else {
				// No listing for this OPF type. We can't choose a replacement value,
				// but we can still catch the case where the node's kind (operator vs
				// data) doesn't match what the slot wants. OPF_AMBIGUOUS slots expect
				// a data value (number/string), all other slots that reach this branch
				// expect an operator subtree. If the kinds disagree, drop in an
				// "<Invalid>" placeholder so the user notices and fixes it.
				const bool node_is_operator = (SEXPT_TYPE(_model.tree_nodes[_model.item_index].type) == SEXPT_OPERATOR);
				const bool slot_wants_operator = (type != OPF_AMBIGUOUS);
				if (node_is_operator != slot_wants_operator) {
					replace_data("<Invalid>", (SEXPT_STRING | SEXPT_VALID));
				}
			}

			// recurse into child operator subtrees to validate their arguments too
			if (_model.tree_nodes[_model.item_index].type & SEXPT_OPERATOR)
				verify_and_fix_arguments(_model.item_index);

			if (list) {
				list->destroy();
				list = nullptr;
			}
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
					Assertion(false, "Unexpected OPF type %d for modify-variable argument", type);
			}
		}

		_model.item_index = _model.tree_nodes[_model.item_index].next;
		arg_num++;
	}

	_model.item_index = tmp;
	_verify_arguments_reentry_guard--;
}

// -----------------------------------------------------------------------
// Variable/container bulk operations
// -----------------------------------------------------------------------

// Remove all references to the named variable throughout the tree.
// Finds all SEXPT_VARIABLE nodes whose text starts with "varname(" and replaces
// them with a plain "number" or "string" placeholder, stripping the SEXPT_VARIABLE flag.
// Preserves item_index across the operation.
void SexpTreeActions::delete_sexp_tree_variable(const char* var_name)
{
	char search_str[64];
	char replace_text[TOKEN_LENGTH];

	snprintf(search_str, sizeof(search_str), "%s(", var_name);

	int old_item_index = _model.item_index;

	for (int idx = 0; idx < static_cast<int>(_model.tree_nodes.size()); idx++) {
		if (_model.tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(_model.tree_nodes[idx].text, search_str) != nullptr) {
				Assertion((_model.tree_nodes[idx].type & SEXPT_NUMBER) || (_model.tree_nodes[idx].type & SEXPT_STRING), "Variable node %d ('%s') has type 0x%x (expected SEXPT_NUMBER or SEXPT_STRING)", idx, _model.tree_nodes[idx].text, _model.tree_nodes[idx].type);

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

// Update all references to a renamed or type-changed variable.
// Finds all SEXPT_VARIABLE nodes matching "old_name(" and replaces them with
// fresh variable data from Sexp_variables[sexp_var_index], updating the display
// text, type flags, and icon. Preserves item_index across the operation.
void SexpTreeActions::modify_sexp_tree_variable(const char* old_name, int sexp_var_index)
{
	Assertion(Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_SET, "Variable '%s' (idx %d) missing SEXP_VARIABLE_SET flag (type %d)", Sexp_variables[sexp_var_index].variable_name, sexp_var_index, Sexp_variables[sexp_var_index].type);
	Assertion((Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER) ||
	       (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_STRING), "Variable '%s' (idx %d) is neither SEXP_VARIABLE_NUMBER nor SEXP_VARIABLE_STRING (type %d)", Sexp_variables[sexp_var_index].variable_name, sexp_var_index, Sexp_variables[sexp_var_index].type);

	const int type = (Sexp_variables[sexp_var_index].type & SEXP_VARIABLE_NUMBER)
		? (SEXPT_NUMBER | SEXPT_VALID)
		: (SEXPT_STRING | SEXPT_VALID);

	int old_item_index = _model.item_index;

	char search_str[64];
	snprintf(search_str, sizeof(search_str), "%s(", old_name);

	for (int idx = 0; idx < static_cast<int>(_model.tree_nodes.size()); idx++) {
		if (_model.tree_nodes[idx].type & SEXPT_VARIABLE) {
			if (strstr(_model.tree_nodes[idx].text, search_str) != nullptr) {
				_model.item_index = idx;
				replace_variable_data(sexp_var_index, (type | SEXPT_VARIABLE));
			}
		}
	}

	_model.item_index = old_item_index;
}

// -----------------------------------------------------------------------
// Clipboard operations
// -----------------------------------------------------------------------

// Copy the currently selected node (and its subtree) to the global sexp clipboard.
// If a previous clipboard exists, frees it first. The subtree is serialized into
// Sexp_nodes via save_branch and marked persistent to prevent garbage collection.
void SexpTreeActions::clipboard_copy()
{
	if (_model.item_index < 0)
		return;

	// If a clipboard already exists, unmark it as persistent and free old clipboard
	if (Sexp_clipboard != -1) {
		sexp_unmark_persistent(Sexp_clipboard);
		free_sexp2(Sexp_clipboard);
	}

	// Allocate new clipboard and mark persistent
	Sexp_clipboard = _model.save_branch(_model.item_index, 1);
	sexp_mark_persistent(Sexp_clipboard);
}

// Replace the currently selected node with the contents of the global sexp clipboard.
// Dispatches based on clipboard content type:
// - SEXP_ATOM_OPERATOR: replaces with operator, loads children from clipboard
// - SEXP_ATOM_CONTAINER_DATA: replaces with container data, loads modifiers
// - SEXP_ATOM_NUMBER/STRING: replaces with data, handles variable flags
// After replacement, expands the subtree for visibility.
void SexpTreeActions::clipboard_paste_replace()
{
	if (_model.item_index < 0 || Sexp_clipboard < 0)
		return;

	// the following assumptions are made..
	Assertion(Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED, "SEXP clipboard node %d marked SEXP_NOT_USED (text '%s')", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST, "SEXP clipboard node %d has invalid subtype SEXP_ATOM_LIST (text '%s')", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		expand_operator(_model.item_index);
		replace_operator(CTEXT(Sexp_clipboard));
		if (Sexp_nodes[Sexp_clipboard].rest != -1) {
			_model.load_branch(Sexp_nodes[Sexp_clipboard].rest, _model.item_index);
			_ui.ui_add_children_visual(_model.item_index);
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
		expand_operator(_model.item_index);
		const auto* p_container = get_sexp_container(Sexp_nodes[Sexp_clipboard].text);
		Assertion(p_container,
			"Attempt to paste unknown container %s. Please report!",
			Sexp_nodes[Sexp_clipboard].text);
		const auto& container = *p_container;
		// this should always be true, but just in case
		const bool has_modifiers = (Sexp_nodes[Sexp_clipboard].first != -1);
		int new_type = (_model.tree_nodes[_model.item_index].type & ~(SEXPT_VARIABLE | SEXPT_CONTAINER_NAME)) | SEXPT_CONTAINER_DATA;
		replace_container_data(container, new_type, false, true, !has_modifiers);
		if (has_modifiers) {
			_model.load_branch(Sexp_nodes[Sexp_clipboard].first, _model.item_index);
			_ui.ui_add_children_visual(_model.item_index);
		} else {
			add_default_modifier(container);
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
		Assertion(Sexp_nodes[Sexp_clipboard].rest == -1, "Number atom on SEXP clipboard (node %d, text '%s') unexpectedly has rest=%d", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text, Sexp_nodes[Sexp_clipboard].rest);
		if (Sexp_nodes[Sexp_clipboard].type & SEXP_FLAG_VARIABLE) {
			int var_idx = get_index_sexp_variable_name(Sexp_nodes[Sexp_clipboard].text);
			Assertion(var_idx > -1, "Invalid variable index: lookup of '%s' from clipboard NUMBER atom failed", Sexp_nodes[Sexp_clipboard].text);
			replace_variable_data(var_idx, (SEXPT_VARIABLE | SEXPT_NUMBER | SEXPT_VALID));
		} else {
			expand_operator(_model.item_index);
			replace_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
		Assertion(Sexp_nodes[Sexp_clipboard].rest == -1, "String atom on SEXP clipboard (node %d, text '%s') unexpectedly has rest=%d", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text, Sexp_nodes[Sexp_clipboard].rest);
		if (Sexp_nodes[Sexp_clipboard].type & SEXP_FLAG_VARIABLE) {
			int var_idx = get_index_sexp_variable_name(Sexp_nodes[Sexp_clipboard].text);
			Assertion(var_idx > -1, "Invalid variable index: lookup of '%s' from clipboard STRING atom failed", Sexp_nodes[Sexp_clipboard].text);
			replace_variable_data(var_idx, (SEXPT_VARIABLE | SEXPT_STRING | SEXPT_VALID));
		} else {
			expand_operator(_model.item_index);
			replace_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));
		}

	} else
		Assertion(0, "Unknown and/or invalid SEXP subtype %d on clipboard (node %d, type %d, text '%s')", Sexp_nodes[Sexp_clipboard].subtype, Sexp_clipboard, Sexp_nodes[Sexp_clipboard].type, Sexp_nodes[Sexp_clipboard].text);

	_ui.ui_expand_branch(_model.tree_nodes[_model.item_index].handle);
}

// Add the contents of the global sexp clipboard as a new child of the current node.
// Similar to clipboard_paste_replace, but inserts a new child instead of replacing.
// Dispatches based on clipboard content type (operator, container data, number, string).
// After insertion, expands the subtree for visibility.
void SexpTreeActions::clipboard_paste_add()
{
	if (_model.item_index < 0 || Sexp_clipboard < 0)
		return;

	// the following assumptions are made..
	Assertion(Sexp_nodes[Sexp_clipboard].type != SEXP_NOT_USED, "SEXP clipboard node %d marked SEXP_NOT_USED (text '%s')", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_LIST, "SEXP clipboard node %d has invalid subtype SEXP_ATOM_LIST (text '%s')", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text);
	Assertion(Sexp_nodes[Sexp_clipboard].subtype != SEXP_ATOM_CONTAINER_NAME,
		"Attempt to use container name %s from SEXP clipboard. Please report!",
		Sexp_nodes[Sexp_clipboard].text);

	if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_OPERATOR) {
		expand_operator(_model.item_index);
		add_operator(CTEXT(Sexp_clipboard));
		if (Sexp_nodes[Sexp_clipboard].rest != -1) {
			_model.load_branch(Sexp_nodes[Sexp_clipboard].rest, _model.item_index);
			_ui.ui_add_children_visual(_model.item_index);
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_CONTAINER_DATA) {
		expand_operator(_model.item_index);
		add_container_data(Sexp_nodes[Sexp_clipboard].text);
		const int modifier_node = Sexp_nodes[Sexp_clipboard].first;
		if (modifier_node != -1) {
			_model.load_branch(modifier_node, _model.item_index);
			_ui.ui_add_children_visual(_model.item_index);
		} else {
			// this shouldn't happen, but just in case
			const auto* p_container = get_sexp_container(Sexp_nodes[Sexp_clipboard].text);
			Assertion(p_container,
				"Attempt to add-paste unknown container %s. Please report!",
				Sexp_nodes[Sexp_clipboard].text);
			add_default_modifier(*p_container);
		}

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_NUMBER) {
		Assertion(Sexp_nodes[Sexp_clipboard].rest == -1, "Number atom on SEXP clipboard (node %d, text '%s') unexpectedly has rest=%d", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text, Sexp_nodes[Sexp_clipboard].rest);
		expand_operator(_model.item_index);
		add_data(CTEXT(Sexp_clipboard), (SEXPT_NUMBER | SEXPT_VALID));

	} else if (Sexp_nodes[Sexp_clipboard].subtype == SEXP_ATOM_STRING) {
		Assertion(Sexp_nodes[Sexp_clipboard].rest == -1, "String atom on SEXP clipboard (node %d, text '%s') unexpectedly has rest=%d", Sexp_clipboard, Sexp_nodes[Sexp_clipboard].text, Sexp_nodes[Sexp_clipboard].rest);
		expand_operator(_model.item_index);
		add_data(CTEXT(Sexp_clipboard), (SEXPT_STRING | SEXPT_VALID));

	} else
		Assertion(0, "Unknown and/or invalid SEXP subtype %d on clipboard (node %d, type %d, text '%s')", Sexp_nodes[Sexp_clipboard].subtype, Sexp_clipboard, Sexp_nodes[Sexp_clipboard].type, Sexp_nodes[Sexp_clipboard].text);

	_ui.ui_expand_branch(_model.tree_nodes[_model.item_index].handle);
}

// Rename all container references (both CONTAINER_NAME and CONTAINER_DATA nodes)
// that match old_name to new_name. Updates both the model text and the UI widget text.
// Returns true if any nodes were renamed, false if no matches were found.
bool SexpTreeActions::rename_container_nodes(const SCP_string& old_name, const SCP_string& new_name)
{
	Assertion(!old_name.empty(),
		"Attempt to rename container nodes looking for empty name. Please report!");
	Assertion(!new_name.empty(),
		"Attempt to rename container nodes with empty name. Please report!");
	Assertion(new_name.length() <= sexp_container::NAME_MAX_LENGTH,
		"Attempt to rename container nodes with name %s that is too long (%d > %d). Please report!",
		new_name.c_str(), static_cast<int>(new_name.length()), sexp_container::NAME_MAX_LENGTH);

	bool renamed_anything = false;

	for (int node_idx = 0; node_idx < static_cast<int>(_model.tree_nodes.size()); node_idx++) {
		if (_model.is_matching_container_node(node_idx, old_name)) {
			strcpy_s(_model.tree_nodes[node_idx].text, new_name.c_str());
			_ui.ui_set_item_text(_model.tree_nodes[node_idx].handle, new_name.c_str());
			renamed_anything = true;
		}
	}

	return renamed_anything;
}
