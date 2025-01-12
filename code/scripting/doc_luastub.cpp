#include "doc_html.h"
#include "utils/string_utils.h"

#include <regex>

namespace scripting {
namespace {

SCP_vector<std::pair<SCP_string, SCP_string>> Hook_variables;
SCP_vector<std::pair<SCP_string, SCP_string>> Aliases;

//Forward declrattion
SCP_string get_output_type_link(const ade_type_info& type_info, SCP_string nestedFuncName = "");

SCP_string sanitizeParamName(const SCP_string& name)
{
	// List of common Lua reserved keywords
	static const SCP_vector<SCP_string> luaKeywords = {"and",
		"break",
		"do",
		"else",
		"elseif",
		"end",
		"false",
		"for",
		"function",
		"if",
		"in",
		"local",
		"nil",
		"not",
		"or",
		"repeat",
		"return",
		"then",
		"true",
		"until",
		"while"};

	// Check if the name matches any Lua keyword
	if (util::isStringOneOf(name, luaKeywords)) {
		return name + "Val";
	}

	return name;
}

SCP_string create_function_alias(const ade_type_info& type_info, SCP_string aliasName = "")
{
	static int dupCount = 1; // Static counter for unique aliases

	if (aliasName.empty() || aliasName.find("param") == 0) {
		aliasName = "aliasFunc";
	}

	// Sanitize aliasName
	for (char& c : aliasName) {
		if (c == '?') {
			c = '_';
		}
	}

	SCP_string alias = "fun(";

	const auto& elements = type_info.elements();
	int count = 1;

	// Process function parameters
	for (auto iter = elements.begin() + 1; iter != elements.end(); ++iter) {
		if (count > 1) {
			alias += ", ";
		}
		SCP_string paramName = iter->getName().empty() ? "param" + std::to_string(count) : sanitizeParamName(iter->getName());
		alias += paramName + ": " + get_output_type_link(*iter);
		count++;
	}

	// Add return type
	alias += "): " + get_output_type_link(elements.front());

	// Check for duplicates
	auto it = std::find_if(Aliases.begin(), Aliases.end(), [&](const auto& entry) {
		return entry.second == alias;
	});

	if (it != Aliases.end()) {
		return it->first; // Alias already exists, reuse it
	}

	// Check if aliasName conflicts with an existing name
	auto nameConflict = std::find_if(Aliases.begin(), Aliases.end(), [&](const auto& entry) {
		return entry.first == aliasName;
	});

	// Modify aliasName to make it unique if necessary
	if (nameConflict != Aliases.end()) {
		aliasName += "_" + std::to_string(dupCount++);
	}

	Aliases.emplace_back(aliasName, alias);

	return aliasName;
}

SCP_string get_output_type_link(const ade_type_info& type_info, SCP_string nestedFuncName)
{
	SCP_string rstring;

	switch (type_info.getType()) {
	case ade_type_info_type::Empty:
		return "nil";

	case ade_type_info_type::Simple:
		rstring = type_info.getIdentifier();
		return lcase_equal(rstring, "void") ? "nil" : rstring;

	case ade_type_info_type::Tuple: {
		bool first = true;
		for (const auto& type : type_info.elements()) {
			if (!first)
				rstring += ", ";
			first = false;
			rstring += get_output_type_link(type);
		}
		return rstring;
	}

	case ade_type_info_type::Array:
		rstring = "{ ";
		rstring += get_output_type_link(type_info.arrayType());
		rstring += " ... }";
		return rstring;

	case ade_type_info_type::Map:
		rstring = "table";
		if (!type_info.elements().empty() && type_info.elements().size() == 2) {
			rstring += "<" + get_output_type_link(type_info.elements()[0]) + ", " +
					   get_output_type_link(type_info.elements()[1]) + ">";
		}
		return rstring;

	case ade_type_info_type::Iterator:
		return get_output_type_link(type_info.arrayType()) + "[]";

	case ade_type_info_type::Alternative: {
		bool first = true;
		for (const auto& type : type_info.elements()) {
			if (!first)
				rstring += " | ";
			first = false;
			rstring += get_output_type_link(type);
		}
		return rstring;
	}

	case ade_type_info_type::Function:
		return create_function_alias(type_info, std::move(nestedFuncName));

	case ade_type_info_type::Generic: {
		rstring = get_output_type_link(type_info.elements().front());
		if (type_info.elements().size() > 1) {
			rstring += "<";
			bool first = true;
			for (auto iter = type_info.elements().begin() + 1; iter != type_info.elements().end(); ++iter) {
				if (!first)
					rstring += ", ";
				first = false;
				rstring += get_output_type_link(*iter);
			}
			rstring += ">";
		}
		return rstring;
	}

	case ade_type_info_type::Varargs:
		return get_output_type_link(type_info.elements().front());

	default:
		Assertion(false, "Unhandled type!");
		return "";
	}
}

SCP_string escapeNewlines(const SCP_string& str)
{
	SCP_string result = str;
	std::replace(result.begin(), result.end(), '\r', ' '); // Replace carriage returns
	std::replace(result.begin(), result.end(), '\n', ' '); // Replace newlines
	return result;
}

SCP_string buildFullPath(const SCP_vector<const DocumentationElement*>& parents, bool func = false)
{
	SCP_string fullPath;
	if (!parents.empty()) {
		for (size_t i = 0; i < parents.size(); ++i) {
			SCP_string name = parents[i]->shortName.empty() ? parents[i]->name : parents[i]->shortName; // Use shortName if available, otherwise name
			fullPath += name;
			if (i < parents.size() - 1) { // Not the last element
				fullPath += ".";
			} else if (func && parents[i]->type == ElementType::Class) { // Last element and is a class
				fullPath += ":";
			} else { // Last element and is not a class (e.g., library)
				fullPath += ".";
			}
		}
	}
	return fullPath;
}

// This is kind of a hacky check but it's the best we can do given the quirks of the FSO API
bool isTableLikeLibrary(const DocumentationElement* el)
{
	if (!el) {
		return false; // Null pointer safety check
	}

	return el->shortName.empty() && !el->children.empty() && util::isStringOneOf(el->children[0]->name, {"__indexer", "__len"});
}

// Function to output a Library
void outputLibrary(FILE* fp, const DocumentationElement* el, const SCP_vector<const DocumentationElement*>& parents)
{
	SCP_string fullPath = buildFullPath(parents);
	SCP_string libraryName = el->shortName.empty() ? el->name : el->shortName;

	if (isTableLikeLibrary(el)) {
		// Document as a table of objects
		const auto first = static_cast<DocumentationElementFunction*>(el->children.front().get());
		fprintf(fp, "\n--- @field %s ", el->name.c_str());
		fputs(get_output_type_link(first->returnType).c_str(), fp);
		fprintf(fp, "[] # %s - %s", escapeNewlines(first->description).c_str(), escapeNewlines(first->returnDocumentation).c_str());
	} else {
		// Document as a regular library
		fprintf(fp, "\n\n--- %s: %s", el->name.c_str(), escapeNewlines(el->description).c_str());
		fprintf(fp, "\n--- @class %s", el->name.c_str());
	}
}


// Function to output a Class
void outputClass(FILE* fp, const DocumentationElement* el)
{
	SCP_string escapedDescription;
	if (!el->description.empty()) {
		escapedDescription = escapeNewlines(el->description);
	}

	SCP_string parentClass;

	// If the class can also access the members of the Object class
	// This list may be incomplete
	if (util::isStringOneOf(el->name, {"asteroid", "debris", "fireball", "ship", "waypoint", "weapon"})) {
		parentClass = " : object";
	}

	// Enumeration class also works like integer apparently
	if (el->name == "enumeration") {
		parentClass = " : integer";
	}

	fprintf(fp, "\n\n-- %s: %s", el->name.c_str(), escapedDescription.c_str());
	fprintf(fp, "\n%s = {}", el->name.c_str());
	fprintf(fp, "\n--- @class %s%s", el->name.c_str(), parentClass.c_str());
}

// Function to output a Function/Operator or a Metamethod
void outputFunction(FILE* fp, const DocumentationElement* el, const SCP_vector<const DocumentationElement*>& parents)
{
	const auto funcEl = static_cast<const DocumentationElementFunction*>(el);
	if (funcEl) {
		for (const auto& overload : funcEl->overloads) {
			SCP_string functionName = el->name.empty() ? el->shortName : el->name;
			
			// Functions
			if (functionName.rfind("__", 0) != 0) {
				// Handle regular functions
				fprintf(fp, "\n--- @field %s fun(", functionName.c_str());

				// Check if the most immediate parent is a Library
				bool isLibraryParent = !parents.empty() && parents.back()->type == ElementType::Library;

				int count = 1;
				if (!isLibraryParent) {
					// Include self: self if the parent is not a Library
					// because our class-like objects always require the
					// object: func syntax and this ensures that's validated
					fputs("self: self", fp);
					count++;
				}

				// List function arguments
				for (const auto& arg : overload.arguments) {
					if (count > 1) {
						fputs(", ", fp);
					}
					SCP_string paramName = arg.name.empty() ? "param" + std::to_string(count) : arg.name;

					paramName = sanitizeParamName(paramName);

					if (arg.optional) {
						paramName += "?";
					}

					if (arg.type.getType() == ade_type_info_type::Varargs) {
						paramName = "...";
					}

					fprintf(fp, "%s: ", paramName.c_str());
					fputs(get_output_type_link(arg.type, paramName).c_str(), fp);
					count++;
				}

			// Metamethods
			} else {
				// List of metamethods supported by LuaLS
				static const std::vector<std::string> supportedMetamethods = {
					"__unm",      // Unary minus
					"__bnot",     // Bitwise NOT
					"__len",      // Length
					"__add",      // Addition
					"__sub",      // Subtraction
					"__mul",      // Multiplication
					"__div",      // Division
					"__mod",      // Modulo
					"__pow",      // Exponentiation
					"__idiv",     // Floor division
					"__band",     // Bitwise AND
					"__bor",      // Bitwise OR
					"__bxor",     // Bitwise XOR
					"__shl",      // Bitwise LEFT SHIFT
					"__shr",      // Bitwise RIGHT SHIFT
					"__concat",   // Concatenation
					"__call",     // Call
					"__indexer"   // Indexing
				};
				// Check if the metamethod is supported
				if (!util::isStringOneOf(functionName, supportedMetamethods)) {
					return;
				}

				// Indexer is special
				if (functionName == "__indexer") {
					fputs("\n--- @field [", fp);
				} else {

					functionName.erase(0, 2);
					fprintf(fp, "\n--- @operator %s(", functionName.c_str());
				}

				// Documentation seems to only allow up to one argument for a metamethod
				if (!overload.arguments.empty()) {
					fputs(get_output_type_link(overload.arguments[0].type).c_str(), fp);
				}

			}

			// Return type
			if (functionName == "__indexer") {
				fputs("] ", fp);
			} else {
				fputs("): ", fp);
			}
			fputs(get_output_type_link(funcEl->returnType).c_str(), fp);

			if (el->description.find("or nil") != std::string::npos ||
				el->description.find("nil if") != std::string::npos) {
				fputs("?", fp);
			}

			if (!el->deprecationMessage.empty()) {
				SCP_string escapedMessage = escapeNewlines(el->deprecationMessage);
				SCP_string version = gameversion::format_version(el->deprecationVersion);
				fprintf(fp, " # DEPRECATED %s: %s -- ", version.c_str(), escapedMessage.c_str());
			}

			if (!el->description.empty()) {
				SCP_string escapedDescription = escapeNewlines(el->description);
				fprintf(fp, " # %s", escapedDescription.c_str());
			}
		}
	}
}


// Function to output a Property
void outputProperty(FILE* fp, const DocumentationElement* el)
{
	if (el) {
		const auto propEl = static_cast<const DocumentationElementProperty*>(el);
		SCP_string propName = propEl->name.empty() ? propEl->shortName : propEl->name;

		if (propEl->description.find("or nil") != std::string::npos ||
			propEl->description.find("nil if") != std::string::npos)
		{
			propName += "?";
		}

		fprintf(fp, "\n--- @field %s ", propName.c_str());
		fputs(get_output_type_link(propEl->getterType).c_str(), fp);

		if (!propEl->returnDocumentation.empty()) {
			fprintf(fp, " # %s, ", propEl->returnDocumentation.c_str());
		}

		if (!el->deprecationMessage.empty()) {
			SCP_string escapedMessage = escapeNewlines(el->deprecationMessage);
			SCP_string version = gameversion::format_version(el->deprecationVersion);
			fprintf(fp, " # DEPRECATED %s: %s -- ", version.c_str(), escapedMessage.c_str());
		}

		fprintf(fp, " %s", propEl->description.empty() ? "No description available." : propEl->description.c_str());
	}
}

void parseFunctionSignature(SCP_string& input)
{
	// Regular expression to match the input format
	static std::regex functionRegex(R"(function\((.*?)\s(\w+)\)\s->\s(.+))");
	std::smatch match;

	if (std::regex_match(input, match, functionRegex)) {
		// Extract parameter types, parameter name, and return type
		SCP_string paramTypes = match[1].str(); // e.g., "number | string | nil"
		SCP_string paramName = match[2].str();  // e.g., "result"
		SCP_string returnType = match[3].str(); // e.g., "nil"

		// Format the output as desired
		input =  "fun(" + paramName + ": " + paramTypes + "): " + returnType;
	}
}

void OutputElement(FILE* fp, const std::unique_ptr<DocumentationElement>& el, const SCP_vector<const DocumentationElement*>& parents)
{
	if (!el || (el->name.empty() && el->shortName.empty())) {
		Warning(LOCATION, "Found ade_table_entry with no name or shortname");
		return;
	}

	// First output any child libraries, excluding table-like libraries
	for (const auto& child : el->children) {
		if (child->type == ElementType::Library && !isTableLikeLibrary(child.get())) {
			SCP_vector<const DocumentationElement*> newParents(parents);
			newParents.push_back(el.get());
			OutputElement(fp, child, newParents);
		}
	}
	
	switch (el->type) {
		case ElementType::Library:
			outputLibrary(fp, el.get(), parents);
			break;
		case ElementType::Class:
			outputClass(fp, el.get());
			break;
		case ElementType::Function:
		case ElementType::Operator:
			outputFunction(fp, el.get(), parents);
			break;
		case ElementType::Property:
			outputProperty(fp, el.get());
			break;
		default:
			Warning(LOCATION, "Unknown type '%d' passed to ade_table_entry::OutputMeta", static_cast<int>(el->type));
			break;
	}

	// Now output everything else for non table-like libraries
	// If we're a library and we're table-like, then do not print any children
	// because we've been defined as an array already and that would be redundant
	if (el->type != ElementType::Library || !isTableLikeLibrary(el.get())) {
		for (const auto& child : el->children) {

			// If the child is anything but a regular library then output it, including table-like libraries
			if (child->type != ElementType::Library || isTableLikeLibrary(child.get())) {
				SCP_vector<const DocumentationElement*> newParents(parents);
				newParents.push_back(el.get());
				OutputElement(fp, child, newParents);
			}
		}
	}

	// For Hook Variables we output all the possible HVs as fields of the HV class
	// This allows the Lua environment to pick them up as valid variables with defined
	// return types. Scripters should refer to scripting.html to know when they are valid
	if (el->name == "HookVariables") {
		for (const auto& var : Hook_variables) {
			SCP_string type = var.second;
			if (type.find("function") != SCP_string::npos) {
				parseFunctionSignature(type);
			}
			fprintf(fp, "\n--- @field %s? %s", var.first.c_str(), type.c_str());
		}
	}

	// After processing all children, initialize the table if necessary
	if (el->type == ElementType::Library && !isTableLikeLibrary(el.get())) {
		SCP_string fullPath = buildFullPath(parents);
		SCP_string libraryName = el->shortName.empty() ? el->name : el->shortName;
		fprintf(fp, "\n%s%s = {}", fullPath.c_str(), libraryName.c_str());
	}
}

void OutputLuaMeta(FILE* fp, const ScriptingDocumentation& doc)
{
	// First pass: Output classes
	for (const auto& el : doc.elements) {
		if (el && el->type == ElementType::Class) {
			SCP_vector<const DocumentationElement*> emptyParents;
			OutputElement(fp, el, emptyParents);
		}
	}
	
	// Second pass: Output everything else (libraries, functions, fields, etc.)
	for (const auto& el : doc.elements) {
		if (el && el->type != ElementType::Class) { // Skip classes this time
			SCP_vector<const DocumentationElement*> emptyParents;
			OutputElement(fp, el, emptyParents);
		}
	}

	fputs("\n", fp);
}

void OutputLuaJsonDocumentation(FILE* fp) {
    if (!fp) return;

	fputs("--- dkjson is a built-in lua method for encoding and decoding values to/from json files\n", fp);
	fputs("--- call dkjson with <local json = require('dkjson')>\n", fp);
    fputs("--- @class json\n", fp);
    fprintf(fp, "--- @field encode json_encode\n");
    fprintf(fp, "--- @field decode json_decode\n");
    fprintf(fp, "--- @field use_lpeg json_use_lpeg\n");
    fprintf(fp, "--- @field quotestring json_quote_string\n");
    fprintf(fp, "--- @field addnewline json_addnewline\n");
    fprintf(fp, "--- @field encodeexception json_encodeexception\n");
    fputs("json = {}\n\n", fp);

    fputs("--- Encodes a Lua value into a JSON string.\n", fp);
    fputs("---- value: any The Lua value to encode (e.g., table, string, number, boolean, nil).\n", fp);
    fputs("---- state: table (Optional) A table to configure encoding options:\n", fp);
    fputs("---   - `indent`: Enables pretty-printing if set to true.\n", fp);
    fputs("---   - `level`: Sets the current indentation level (used internally).\n", fp);
    fputs("---   - `keyorder`: Specifies a custom order for object keys.\n", fp);
    fputs("--- @return string The JSON string representation of the Lua value.\n", fp);
    fputs("--- @alias json_encode fun(value: any, state?: table): string\n\n", fp);

    fputs("--- Decodes a JSON string into a Lua value.\n", fp);
    fputs("---- jsonString: string The JSON string to decode.\n", fp);
    fputs("---- pos: number (Optional) The position in the string to start decoding (default is 1).\n", fp);
    fputs("---- nullval: any (Optional) A value to represent JSON `null` (default is `nil`).\n", fp);
    fputs("---- ...: table (Optional) Custom metatables for objects and arrays.\n", fp);
    fputs("--- @return any The Lua value resulting from the decoding.\n", fp);
    fputs("--- @return number The position in the string where parsing ended.\n", fp);
    fputs("--- @return string (Optional) An error message if decoding failed.\n", fp);
    fputs("--- @alias json_decode fun(jsonString: string, pos?: number, nullval?: any, ...?: table): any, number, string\n\n", fp);

    fputs("--- Enables the use of LPeg for JSON parsing and encoding.\n", fp);
    fputs("--- @return table The `dkjson` library with LPeg integration enabled.\n", fp);
    fputs("--- @alias json_use_lpeg fun(): table\n\n", fp);

    fputs("--- Encodes a Lua string as a JSON string literal.\n", fp);
    fputs("---- value: string The Lua string to encode.\n", fp);
    fputs("--- @return string The JSON string literal representation of the Lua string.\n", fp);
    fputs("--- @alias json_quote_string fun(value: string): string\n\n", fp);

    fputs("--- Adds a newline and indentation to the JSON buffer.\n", fp);
    fputs("---- state: table The current encoding state.\n", fp);
    fputs("--- @alias json_addnewline fun(state: table): nil\n\n", fp);

    fputs("--- Handles encoding exceptions by returning a placeholder JSON string.\n", fp);
    fputs("---- reason: string The reason for the exception.\n", fp);
    fputs("---- value: any The value that caused the exception.\n", fp);
    fputs("---- state: table The current encoding state.\n", fp);
    fputs("---- defaultmessage: string The default error message.\n", fp);
    fputs("--- @return string A JSON string representing the exception.\n", fp);
    fputs("--- @alias json_encodeexception fun(reason: string, value: any, state: table, defaultmessage: string): string\n", fp);
}

} // namespace

void output_luastub_doc(const ScriptingDocumentation& doc, const SCP_string& filename)
{
	FILE* fp = fopen(filename.c_str(), "w");

	if (fp == nullptr) {
		return;
	}

	// Header
	fprintf(fp, "-- Lua Stub File\n");
	fprintf(fp, "-- Generated for FSO v%s (%s)\n\n", FS_VERSION_FULL, doc.name.c_str());

	//***Version info
	fprintf(fp, "-- Lua Version: %s\n", LUA_RELEASE);
	fputs("---@meta\n", fp);

	// Get all the hook variables to print and save them to the vector
	std::unordered_set<std::string> uniqueHookVars;
	for (const auto& action : doc.actions) {
		if (!action.description.empty()) {
			for (const auto& param : action.parameters) {
				if (uniqueHookVars.insert(param.name).second) { // Insert returns false if already exists
					Hook_variables.emplace_back(param.name, param.type.getIdentifier());
				}
			}
		}
	}

	OutputLuaMeta(fp, doc);

	fputs("\n-- Aliases help with nested function calls in the above pseudo classes", fp);
	for (const auto& alias : Aliases) {
		fprintf(fp, "\n--- @alias %s %s", alias.first.c_str(), alias.second.c_str());
	}

	fputs("\n", fp);
	fputs("\n", fp);

	//***Enumerations
	fprintf(fp, "-- Enumerations\n");
	for (const auto& enumeration : doc.enumerations) {
		// Cyborg17 -- Omit the deprecated flag
		if (enumeration.name == "VM_EXTERNAL_CAMERA_LOCKED") {
			continue;
		}
		// WMC - For now, print to the file without the description.
		fprintf(fp, "--- @const %s\n", enumeration.name.c_str());
		fprintf(fp, "%s = enumeration\n", enumeration.name.c_str());
	}
	fputs("\n", fp);

	OutputLuaJsonDocumentation(fp);
	fputs("\n", fp);

	fclose(fp);

	Hook_variables.clear();
	Aliases.clear();
}

} // namespace scripting
