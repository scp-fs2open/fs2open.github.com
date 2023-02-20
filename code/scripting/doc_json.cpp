
#include "doc_json.h"

#include "libs/jansson.h"

namespace scripting {

static void json_doc_generate_class(json_t* elObj, const DocumentationElementClass* lib)
{
	json_object_set_new(elObj, "superClass", json_string(lib->superClass.c_str()));
}
static json_t* json_doc_generate_return_type(const scripting::ade_type_info& type_info)
{
	switch (type_info.getType()) {
	case ade_type_info_type::Empty:
		return json_string("void");
	case ade_type_info_type::Simple:
		return json_string(type_info.getIdentifier());
	case ade_type_info_type::Tuple: {
		json_t* tupleTypes = json_array();

		for (const auto& type : type_info.elements()) {
			json_array_append_new(tupleTypes, json_doc_generate_return_type(type));
		}

		return json_pack("{ssso}", "type", "tuple", "elements", tupleTypes);
	}
	case ade_type_info_type::Array: {
		return json_pack("{ssso}",
			"type",
			"list",
			"element",
			json_doc_generate_return_type(type_info.elements().front()));
	}
	case ade_type_info_type::Map: {
		return json_pack("{sssoso}",
			"type",
			"map",
			"key",
			json_doc_generate_return_type(type_info.elements()[0]),
			"value",
			json_doc_generate_return_type(type_info.elements()[1]));
	}
	case ade_type_info_type::Iterator: {
		return json_pack("{ssso}",
			"type",
			"iterator",
			"element",
			json_doc_generate_return_type(type_info.elements().front()));
	}
	case ade_type_info_type::Alternative: {
		json_t* alternativeTypes = json_array();

		for (const auto& type : type_info.elements()) {
			json_array_append_new(alternativeTypes, json_doc_generate_return_type(type));
		}

		return json_pack("{ssso}", "type", "alternative", "elements", alternativeTypes);
	}
	case ade_type_info_type::Function: {
		const auto& elements = type_info.elements();

		json_t* parameterTypes = json_array();

		if (elements.size() > 1) {
			for (auto iter = elements.begin() + 1; iter != elements.end(); ++iter) {
				json_array_append_new(parameterTypes,
					json_pack("{ssso}", "name", iter->getName().c_str(), "type", json_doc_generate_return_type(*iter)));
			}
		}

		return json_pack("{sssoso}",
			"type",
			"function",
			"returnType",
			json_doc_generate_return_type(elements.front()),
			"parameters",
			parameterTypes);
	}
	case ade_type_info_type::Generic: {
		const auto& elements = type_info.elements();

		json_t* parameterTypes = json_array();

		if (elements.size() > 1) {
			for (auto iter = elements.begin() + 1; iter != elements.end(); ++iter) {
				json_array_append_new(parameterTypes, json_doc_generate_return_type(*iter));
			}
		}

		return json_pack("{sssoso}",
			"type",
			"generic",
			"baseType",
			json_doc_generate_return_type(elements.front()),
			"parameters",
			parameterTypes);
	}
	case ade_type_info_type::Varargs: {
		return json_pack("{ssso}",
			"type",
			"varargs",
			"baseType",
			json_doc_generate_return_type(type_info.elements().front()));
	}
	}

	UNREACHABLE("Unknown type type!");
	return nullptr;
}
static json_t* json_doc_function_signature(const SCP_vector<scripting::argument_def>& args)
{
	json_t* arr = json_array();

	for (const auto& arg : args) {
		json_array_append_new(arr,
			json_pack("{sosssssbss}",
				"type",
				json_doc_generate_return_type(arg.type),
				"name",
				arg.name.c_str(),
				"default",
				arg.def_val.c_str(),
				"optional",
				arg.optional,
				"description",
				arg.comment.c_str()));
	}

	return arr;
}
static void json_doc_generate_function(json_t* elObj, const DocumentationElementFunction* lib)
{
	json_object_set_new(elObj, "returnType", json_doc_generate_return_type(lib->returnType));
	json_object_set_new(elObj, "returnDocumentation", json_string(lib->returnDocumentation.c_str()));

	if (lib->overloads.size() == 1 && !lib->overloads.front().simple.empty()) {
		// Legacy argument list not filled with argument data
		json_object_set_new(elObj, "parameters", json_string(lib->overloads.front().simple.c_str()));
	} else {
		json_t* arr = json_array();

		for (const auto& overload : lib->overloads) {
			if (!overload.simple.empty()) {
				json_array_append_new(arr, json_string(overload.simple.c_str()));
			} else {
				json_array_append_new(arr, json_doc_function_signature(overload.arguments));
			}
		}

		json_object_set_new(elObj, "parameters", arr);
	}
}
static void json_doc_generate_property(json_t* elObj, const DocumentationElementProperty* lib)
{
	json_object_set_new(elObj, "getterType", json_doc_generate_return_type(lib->getterType));
	json_object_set_new(elObj, "setterType", json_doc_generate_return_type(lib->setterType));
	json_object_set_new(elObj, "returnDocumentation", json_string(lib->returnDocumentation.c_str()));
}
static json_t* json_doc_generate_elements(const SCP_vector<std::unique_ptr<DocumentationElement>>& elements);
static json_t* json_doc_generate_element(const std::unique_ptr<DocumentationElement>& element)
{
	json_t* elementsObj = json_object();

	json_object_set_new(elementsObj, "name", json_string(element->name.c_str()));
	json_object_set_new(elementsObj, "shortName", json_string(element->shortName.c_str()));
	json_object_set_new(elementsObj, "description", json_string(element->description.c_str()));

	switch (element->type) {
	case ElementType::Library:
		json_object_set_new(elementsObj, "type", json_string("library"));
		break;
	case ElementType::Class:
		json_object_set_new(elementsObj, "type", json_string("class"));
		json_doc_generate_class(elementsObj, static_cast<DocumentationElementClass*>(element.get()));
		break;
	case ElementType::Function:
		json_object_set_new(elementsObj, "type", json_string("function"));
		json_doc_generate_function(elementsObj, static_cast<DocumentationElementFunction*>(element.get()));
		break;
	case ElementType::Operator:
		json_object_set_new(elementsObj, "type", json_string("operator"));
		json_doc_generate_function(elementsObj, static_cast<DocumentationElementFunction*>(element.get()));
		break;
	case ElementType::Property:
		json_object_set_new(elementsObj, "type", json_string("property"));
		json_doc_generate_property(elementsObj, static_cast<DocumentationElementProperty*>(element.get()));
		break;
	default:
		json_object_set_new(elementsObj, "type", json_string("unknown"));
		break;
	}

	json_object_set_new(elementsObj, "children", json_doc_generate_elements(element->children));

	return elementsObj;
}

static json_t* json_doc_generate_elements(const SCP_vector<std::unique_ptr<DocumentationElement>>& elements)
{
	json_t* elementsArray = json_array();

	for (const auto& el : elements) {
		json_array_append_new(elementsArray, json_doc_generate_element(el));
	}

	return elementsArray;
}

static json_t* json_doc_generate_param_element(const HookVariableDocumentation& param)
{
	return json_pack("{s:s,s:s,s:o}",
		"name",
		param.name,
		"description",
		param.description,
		"type",
		json_doc_generate_return_type(param.type));
}

static json_t* json_doc_generate_action_element(const DocumentationAction& action)
{
	json_t* paramArray = json_array();
	json_t* conditionArray = json_array();

	for (const auto& param : action.parameters) {
		json_array_append_new(paramArray, json_doc_generate_param_element(param));
	}

	for (const auto& condition : action.conditions) {
		json_array_append_new(conditionArray, json_pack("{s:s,s:s}",
			"name",
			condition.first.c_str(),
			"description",
			condition.second->documentation.c_str()));
	}

	return json_pack("{s:s,s:s,s:o,s:o,s:b}",
		"name",
		action.name.c_str(),
		"description",
		action.description.c_str(),
		"hookVars",
		paramArray,
		"conditions",
		conditionArray,
		"overridable",
		action.overridable);
}

void output_json_doc(const ScriptingDocumentation& doc, const SCP_string& filename)
{
	std::unique_ptr<json_t> root(json_object());
	// Should be incremented every time an incompatible change is made
	json_object_set_new(root.get(), "version", json_integer(2));

	{
		json_t* actionArray = json_array();

		for (const auto& action : doc.actions) {
			json_array_append_new(actionArray, json_doc_generate_action_element(action));
		}

		json_object_set_new(root.get(), "actions", actionArray);
	}
	{
		json_t* conditionArray = json_array();

		for (const auto& cond : doc.conditions) {
			json_array_append_new(conditionArray, json_string(cond.c_str()));
		}

		json_object_set_new(root.get(), "conditions", conditionArray);
	}
	{
		json_t* enumObject = json_object();

		for (const auto& enumVal : doc.enumerations) {
			json_object_set_new(enumObject, enumVal.name.c_str(), json_integer(enumVal.value));
		}

		json_object_set_new(root.get(), "enums", enumObject);
	}
	{
		json_t* globalVariables = json_array();

		for (const auto& param : doc.globalVariables) {
			json_array_append_new(globalVariables, json_doc_generate_param_element(param));
		}

		json_object_set_new(root.get(), "globalVars", globalVariables);
	}
	json_object_set_new(root.get(), "elements", json_doc_generate_elements(doc.elements));

	const auto jsonStr = json_dump_string(root.get(), JSON_INDENT(2) | JSON_SORT_KEYS);

	std::ofstream outStr(filename, std::ios::binary);
	outStr << jsonStr;
}

} // namespace scripting
