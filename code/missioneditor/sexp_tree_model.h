/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#pragma once

// Shared sexp tree model — UI-independent data structures and logic.
// Used by both FRED2 (MFC) and QtFRED (Qt) sexp tree implementations.

#include "globalincs/globals.h"
#include "globalincs/flagset.h"
#include "globalincs/vmallocator.h"  // SCP_string, SCP_vector

// -----------------------------------------------------------------------
// SEXPT_* node type/status constants
// -----------------------------------------------------------------------

#define SEXPT_UNUSED          0x0000
#define SEXPT_UNINIT          0x0001
#define SEXPT_UNKNOWN         0x0002

#define SEXPT_VALID           0x1000
#define SEXPT_TYPE_MASK       0x07ff
#define SEXPT_TYPE(X)         (SEXPT_TYPE_MASK & (X))

#define SEXPT_OPERATOR        0x0010
#define SEXPT_NUMBER          0x0020
#define SEXPT_STRING          0x0040
#define SEXPT_VARIABLE        0x0080
#define SEXPT_CONTAINER_NAME  0x0100
#define SEXPT_CONTAINER_DATA  0x0200
#define SEXPT_MODIFIER        0x0400

// -----------------------------------------------------------------------
// Node flags (editability)
// -----------------------------------------------------------------------

#define NOT_EDITABLE  0x00
#define OPERAND       0x01
#define EDITABLE      0x02
#define COMBINED      0x04

// -----------------------------------------------------------------------
// NodeImage — unified icon enum for tree nodes
// -----------------------------------------------------------------------
// Replaces FRED2's BITMAP_* #defines and QtFRED's NodeImage enum class.
// Numeric values match the original BITMAP_* constants for compatibility.

enum class NodeImage : int {
	OPERATOR = 0,
	DATA = 1,
	VARIABLE = 2,
	ROOT = 3,
	ROOT_DIRECTIVE = 4,
	CHAIN = 5,
	CHAIN_DIRECTIVE = 6,
	GREEN_DOT = 7,
	BLACK_DOT = 8,
	// Numbered data icons (DATA_00 through DATA_95, stepping by 5)
	DATA_00 = 9,
	DATA_05 = 10,
	DATA_10 = 11,
	DATA_15 = 12,
	DATA_20 = 13,
	DATA_25 = 14,
	DATA_30 = 15,
	DATA_35 = 16,
	DATA_40 = 17,
	DATA_45 = 18,
	DATA_50 = 19,
	DATA_55 = 20,
	DATA_60 = 21,
	DATA_65 = 22,
	DATA_70 = 23,
	DATA_75 = 24,
	DATA_80 = 25,
	DATA_85 = 26,
	DATA_90 = 27,
	DATA_95 = 28,
	COMMENT = 29,
	CONTAINER_NAME = 30,
	CONTAINER_DATA = 31,
};

// -----------------------------------------------------------------------
// Tree behavior mode flags and modes
// -----------------------------------------------------------------------

FLAG_LIST(TreeFlags) {
	LabeledRoot = 0,
	RootDeletable,
	RootEditable,
	AnnotationsAllowed,

	NUM_VALUES
};

// Legacy ST_* / MODE_* constants (FRED2 compatibility)
#define ST_LABELED_ROOT   0x10000
#define ST_ROOT_DELETABLE 0x20000
#define ST_ROOT_EDITABLE  0x40000

#define MODE_GOALS     (1 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)
#define MODE_EVENTS    (2 | ST_LABELED_ROOT | ST_ROOT_DELETABLE | ST_ROOT_EDITABLE)
#define MODE_CAMPAIGN  (3 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)
#define MODE_CUTSCENES (4 | ST_LABELED_ROOT | ST_ROOT_DELETABLE)

// -----------------------------------------------------------------------
// sexp_tree_item — a single node in the sexp tree
// -----------------------------------------------------------------------
// The handle member is a void* that each UI layer casts to its native type:
//   FRED2:  static_cast<HTREEITEM>(handle)
//   QtFRED: static_cast<QTreeWidgetItem*>(handle)

class sexp_tree_item {
public:
	sexp_tree_item() : type(SEXPT_UNUSED), parent(-1), child(-1), next(-1), flags(0), handle(nullptr) {
		text[0] = '\0';
	}

	int type;
	int parent;    // index of parent node (-1 if none)
	int child;     // index of first child node (-1 if none)
	int next;      // index of next sibling (-1 if none)
	int flags;
	char text[2 * TOKEN_LENGTH + 2];
	void* handle;  // opaque UI handle — never dereferenced by model code
};

// -----------------------------------------------------------------------
// sexp_list_item — linked list node for building option listings
// -----------------------------------------------------------------------

class sexp_list_item {
public:
	int type;
	int op;
	SCP_string text;
	sexp_list_item* next;

	sexp_list_item() : type(0), op(-1), next(nullptr) {}

	void set_op(int op_num);
	void set_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_op(int op_num);
	void add_data(const char* str, int t = (SEXPT_STRING | SEXPT_VALID));
	void add_list(sexp_list_item* list);
	void shallow_copy(const sexp_list_item* src);
	void destroy();
};

// -----------------------------------------------------------------------
// SexpTreeEditorInterface — context interface implemented by editor dialogs
// -----------------------------------------------------------------------
// Both FRED2 and QtFRED dialogs can implement this to provide context-dependent
// data (messages, goals, events, mission names) to the shared tree model.
// Uses SCP types — UI layers translate at the boundary as needed.

class SexpTreeEditorInterface {
	flagset<TreeFlags> _flags;

public:
	SexpTreeEditorInterface();
	explicit SexpTreeEditorInterface(const flagset<TreeFlags>& flags);
	virtual ~SexpTreeEditorInterface();

	virtual bool hasDefaultMessageParamter();
	virtual SCP_vector<SCP_string> getMessages();

	virtual SCP_vector<SCP_string> getMissionGoals(const SCP_string& reference_name);
	virtual bool hasDefaultGoal(int operator_value);

	virtual SCP_vector<SCP_string> getMissionEvents(const SCP_string& reference_name);
	virtual bool hasDefaultEvent(int operator_value);

	virtual SCP_vector<SCP_string> getMissionNames();
	virtual bool hasDefaultMissionName();

	virtual int getRootReturnType() const;

	const flagset<TreeFlags>& getFlags() const;

	virtual bool requireCampaignOperators() const;
};
