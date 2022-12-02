#pragma once

#include "globalincs/pstypes.h"

#include "scripting/ade_doc.h"
#include "scripting/doc_parser.h"
#include "scripting/hook_api.h"

namespace scripting {

enum class ElementType {
	Unknown,
	Library,
	Class,
	Function,
	Operator,
	Property,
};

struct DocumentationElement {
	virtual ~DocumentationElement() = default;

	ElementType type = ElementType::Unknown;

	SCP_string name;
	SCP_string shortName;

	SCP_string description;

	gameversion::version deprecationVersion;
	SCP_string deprecationMessage;

	SCP_vector<std::unique_ptr<DocumentationElement>> children;
};

struct DocumentationElementClass : public DocumentationElement {
	~DocumentationElementClass() override = default;

	SCP_string superClass;
};

struct DocumentationElementProperty : public DocumentationElement {
	~DocumentationElementProperty() override = default;

	scripting::ade_type_info getterType;
	scripting::ade_type_info setterType;

	SCP_string returnDocumentation;
};

struct DocumentationElementFunction : public DocumentationElement {
	~DocumentationElementFunction() override = default;

	scripting::ade_type_info returnType;

	struct argument_list {
		SCP_string simple;

		SCP_vector<scripting::argument_def> arguments;

		SCP_vector<SCP_string> genericTypes;
	};

	SCP_vector<argument_list> overloads;

	SCP_string returnDocumentation;
};

struct DocumentationEnum {
	SCP_string name;
	int value;
};

struct DocumentationAction {
	SCP_string name;
	SCP_string description;
	SCP_vector<HookVariableDocumentation> parameters;
	const SCP_unordered_map<SCP_string, const std::unique_ptr<const ParseableCondition>>& conditions;
	bool overridable;
};

struct ScriptingDocumentation {
	SCP_string name;

	SCP_vector<SCP_string> conditions;
	SCP_vector<HookVariableDocumentation> globalVariables;
	SCP_vector<DocumentationAction> actions;

	SCP_vector<std::unique_ptr<DocumentationElement>> elements;

	SCP_vector<DocumentationEnum> enumerations;
};

}
