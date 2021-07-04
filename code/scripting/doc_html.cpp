#include "doc_html.h"

namespace scripting {
namespace {

void ade_output_toc(FILE* fp, const std::unique_ptr<DocumentationElement>& el)
{
	Assert(fp != nullptr);
	Assert(el != nullptr);

	// WMC - sanity checking
	if (el->name.empty() && el->shortName.empty()) {
		Warning(LOCATION, "Found ade_table_entry with no name or shortname");
		return;
	}

	fputs("<dd>", fp);

	if (el->name.empty()) {
		fprintf(fp, "<a href=\"#%s\">%s", el->shortName.c_str(), el->shortName.c_str());
	} else {
		fprintf(fp, "<a href=\"#%s\">%s", el->name.c_str(), el->name.c_str());
		if (!el->shortName.empty())
			fprintf(fp, " (%s)", el->shortName.c_str());
	}
	fputs("</a>", fp);

	if (!el->description.empty())
		fprintf(fp, " - %s\n", el->description.c_str());

	fputs("</dd>\n", fp);
}

void OutputToc(FILE* fp, const ScriptingDocumentation& doc)
{
	//***TOC: Libraries
	fputs("<dt><b>Libraries</b></dt>\n", fp);
	for (const auto& el : doc.elements) {
		if (el->type != ElementType::Library) {
			continue;
		}

		ade_output_toc(fp, el);
	}

	//***TOC: Objects
	fputs("<dt><b>Types</b></dt>\n", fp);
	for (const auto& el : doc.elements) {
		if (el->type != ElementType::Class) {
			continue;
		}

		ade_output_toc(fp, el);
	}

	//***TOC: Enumerations
	fputs("<dt><b><a href=\"#Enumerations\">Enumerations</a></b></dt>", fp);

	//***End TOC
	fputs("</dl><br/><br/>", fp);
}

SCP_string element_name(const std::unique_ptr<DocumentationElement>& el)
{
	if (!el->name.empty()) {
		return el->name;
	}

	return el->shortName;
}

void ade_output_type_link(FILE* fp, const ade_type_info& type_info)
{
	switch (type_info.getType()) {
	case ade_type_info_type::Empty:
		fputs("nothing", fp);
		break;
	case ade_type_info_type::Simple:
		if (ade_is_internal_type(type_info.getIdentifier())) {
			fputs(type_info.getIdentifier(), fp);
		} else {
			fprintf(fp, "<a href=\"#%s\">%s</a>", type_info.getIdentifier(), type_info.getIdentifier());
		}
		break;
	case ade_type_info_type::Tuple: {
		bool first = true;
		for (const auto& type : type_info.elements()) {
			if (!first) {
				fprintf(fp, ", ");
			}
			first = false;

			ade_output_type_link(fp, type);
		}
		break;
	}
	case ade_type_info_type::Array:
		fputs("{ ", fp);
		ade_output_type_link(fp, type_info.arrayType());
		fputs(" ... }", fp);
		break;
	case ade_type_info_type::Map:
		fputs("{ ", fp);
		ade_output_type_link(fp, type_info.elements()[0]);
		fputs(" -> ", fp);
		ade_output_type_link(fp, type_info.elements()[1]);
		fputs(" ... }", fp);
		break;
	case ade_type_info_type::Iterator:
		fputs("iterator< ", fp);
		ade_output_type_link(fp, type_info.arrayType());
		fputs(" >", fp);
		break;
	case ade_type_info_type::Alternative: {
		bool first = true;
		for (const auto& type : type_info.elements()) {
			if (!first) {
				fputs(" | ", fp);
			}
			first = false;

			ade_output_type_link(fp, type);
		}
		break;
	}
	case ade_type_info_type::Function: {
		const auto& elements = type_info.elements();
		if (elements.size() == 1) {
			fputs("function() -> ", fp);
		} else {
			fputs("function(", fp);
			bool first = true;
			for (auto iter = elements.begin() + 1; iter != elements.end(); ++iter) {
				if (!first) {
					fputs(", ", fp);
				}
				first = false;

				ade_output_type_link(fp, *iter);

				fprintf(fp, " %s", iter->getName().c_str());
			}
			fputs(") -> ", fp);
		}
		ade_output_type_link(fp, elements.front());
		break;
	}
	case ade_type_info_type::Generic: {
		const auto& elements = type_info.elements();
		ade_output_type_link(fp, elements.front());
		if (elements.size() > 1) {
			fputs("<", fp);
			bool first = true;
			for (auto iter = elements.begin() + 1; iter != elements.end(); ++iter) {
				if (!first) {
					fputs(", ", fp);
				}
				first = false;

				ade_output_type_link(fp, *iter);
			}
			fputs(">", fp);
		}
		break;
	}
	case ade_type_info_type::Varargs: {
		ade_output_type_link(fp, type_info.elements().front());
		fputs("...", fp);
		break;
	}
	default:
		Assertion(false, "Unhandled type!");
		break;
	}
}

void output_argument_list(FILE* fp, const DocumentationElementFunction::argument_list& overload)
{
	if (overload.simple.empty()) {
		bool first    = true;
		bool optional = false;
		for (const auto& arg : overload.arguments) {
			if (!first) {
				fputs(", ", fp);
			}
			first = false;

			if (arg.optional && !optional) {
				fputs("[", fp);
				optional = true;
			}

			ade_output_type_link(fp, arg.type);

			if (!arg.name.empty()) {
				fprintf(fp, " <i>%s</i>", arg.name.c_str());

				if (!arg.def_val.empty()) {
					fprintf(fp, " = <i>%s</i>", arg.def_val.c_str());
				}
			}

			if (!arg.comment.empty()) {
				fprintf(fp, " (%s)", arg.comment.c_str());
			}
		}

		if (optional) {
			fputs("]", fp);
		}
	} else {
		fprintf(fp, "<i>%s</i>", overload.simple.c_str());
	}
}

void OutputElement(FILE* fp,
	const std::unique_ptr<DocumentationElement>& el,
	const DocumentationElement* parent,
	const DocumentationElement* parentParent)
{
	if (el->name.empty() && el->shortName.empty()) {
		Warning(LOCATION, "Found ade_table_entry with no name or shortname");
		return;
	}

	bool skip_this = false;

	// WMC - Hack
	if (parent != nullptr) {
		for (const auto& child : el->children) {
			if (child->name == "__indexer") {
				OutputElement(fp, child, el.get(), parent);
				skip_this = true;
				break;
			}
		}
	}

	//***Begin entry
	if (!skip_this) {
		fputs("<dd><dl>", fp);

		switch (el->type) {
		case ElementType::Library:
		case ElementType::Class: {
			//***Name (ShortName)
			if (parent == nullptr) {
				fprintf(fp, "<dt id=\"%s\">", element_name(el).c_str());
			}
			if (el->name.empty()) {
				if (el->type == ElementType::Class) {
					const auto classEl = static_cast<DocumentationElementClass*>(el.get());
					if (classEl->superClass.empty())
						fprintf(fp, "<h2>%s</h2>\n", classEl->shortName.c_str());
					else {
						fprintf(fp,
							"<h2>%s:<a href=\"#%s\">%s</a></h2>\n",
							classEl->superClass.c_str(),
							classEl->superClass.c_str(),
							classEl->superClass.c_str());
					}
				} else {
					fprintf(fp, "<h2>%s</h2>\n", el->shortName.c_str());
				}
			} else {
				fprintf(fp, "<h2>%s", el->name.c_str());

				if (!el->shortName.empty())
					fprintf(fp, " (%s)", el->shortName.c_str());

				if (el->type == ElementType::Class) {
					const auto classEl = static_cast<DocumentationElementClass*>(el.get());

					if (!classEl->superClass.empty()) {
						fprintf(fp,
							":<a href=\"#%s\">%s</a>",
							classEl->superClass.c_str(),
							classEl->superClass.c_str());
					}
				}

				fputs("</h2>\n", fp);
			}
			fputs("</dt>\n", fp);

			//***Description
			if (!el->description.empty()) {
				fprintf(fp, "<dd>%s</dd>\n", el->description.c_str());
			}
			break;
		}
		case ElementType::Operator:
		case ElementType::Function: {
			const auto funcEl = static_cast<DocumentationElementFunction*>(el.get());

			//***Name(ShortName)(Arguments)
			for (const auto& overload : funcEl->overloads) {
				fputs("<dt>", fp);

				fputs("<i>", fp);
				if (!funcEl->returnType.isEmpty())
					ade_output_type_link(fp, funcEl->returnType);
				else
					fputs("nil", fp);
				fputs(" </i>", fp);

				const string_conv* op = nullptr;
				if (el->name.empty()) {
					fprintf(fp, "<b>%s", el->shortName.c_str());
				} else {
					op = ade_get_operator(el->name.c_str());

					// WMC - Do we have an operator?
					if (op == nullptr) {
						fprintf(fp, "<b>%s", el->name.c_str());
						if (!el->shortName.empty()) {
							const auto op2 = ade_get_operator(el->shortName.c_str());
							if (op2 == nullptr)
								fprintf(fp, "(%s)", el->shortName.c_str());
							else
								fprintf(fp, "(%s)", op2->dest);
						}
					} else {
						// WMC - Hack
						if (parent != nullptr && !parent->name.empty() && parentParent != nullptr &&
							el->name == "__indexer") {
							fprintf(fp, "<b>%s%s", parent->name.c_str(), op->dest);
						} else
							fprintf(fp, "<b>%s", op->dest);
					}
				}

				if (op == nullptr) {
					fputs("(</b>", fp);
					output_argument_list(fp, overload);
					fputs("<b>)</b>\n", fp);
				} else {
					fputs("</b> ", fp);
					output_argument_list(fp, overload);
				}
				fputs("</dt>\n", fp);
			}

			//***Description
			if (!el->description.empty()) {
				fprintf(fp, "<dd>%s</dd>\n", el->description.c_str());
			}

			if (el->deprecationVersion.isValid()) {
				if (!el->deprecationMessage.empty()) {
					fprintf(fp,
						"<dd><b>Deprecated starting with version %d.%d.%d:</b> %s</dd>\n",
						el->deprecationVersion.major,
						el->deprecationVersion.minor,
						el->deprecationVersion.build,
						el->deprecationMessage.c_str());
				} else {
					fprintf(fp,
						"<dd><b>Deprecated starting with version %d.%d.%d.</b></dd>\n",
						el->deprecationVersion.major,
						el->deprecationVersion.minor,
						el->deprecationVersion.build);
				}
			}

			//***Result: ReturnDescription
			if (!funcEl->returnDocumentation.empty()) {
				fprintf(fp, "<dd><b>Returns:</b> %s</dd>\n", funcEl->returnDocumentation.c_str());
			} else {
				fputs("<dd><b>Returns:</b> Nothing</dd>\n", fp);
			}
			break;
		}
		case ElementType::Property: {
			const auto propEl = static_cast<DocumentationElementProperty*>(el.get());

			//***Type Name(ShortName)
			fputs("<dt>\n", fp);
			fputs("<i>", fp);
			ade_output_type_link(fp, propEl->getterType);
			fputs("</i> ", fp);

			if (propEl->name.empty()) {
				fprintf(fp, "<b>%s</b>\n", propEl->shortName.c_str());
			} else {
				fprintf(fp, "<b>%s", propEl->name.c_str());
				if (!propEl->shortName.empty())
					fputs(propEl->shortName.c_str(), fp);
				fputs("</b>\n", fp);
			}
			if (!propEl->setterType.isEmpty()) {
				fprintf(fp, " = ");
				ade_output_type_link(fp, propEl->setterType);
			}
			fputs("</dt>\n", fp);

			//***Description
			if (!propEl->description.empty())
				fprintf(fp, "<dd>%s</dd>\n", propEl->description.c_str());

			//***Also settable with: Arguments
			if (!propEl->returnDocumentation.empty())
				fprintf(fp, "<dd><b>Value:</b> %s</b></dd>\n", propEl->returnDocumentation.c_str());
			break;
		}
		default:
			Warning(LOCATION, "Unknown type '%d' passed to ade_table_entry::OutputMeta", static_cast<int>(el->type));
			break;
		}
	}

	fputs("<dd><dl>\n", fp);
	for (const auto& child : el->children) {
		if (parent == nullptr || child->name != "__indexer")
			OutputElement(fp, child, el.get(), parent);
	}
	fputs("</dl></dd>\n", fp);

	if (!skip_this)
		fputs("<br></dl></dd>\n", fp);
}

void OutputLuaMeta(FILE* fp, const ScriptingDocumentation& doc)
{
	OutputToc(fp, doc);

	SCP_vector<ade_table_entry*> table_entries;

	//***Everything
	fputs("<dl>\n", fp);
	for (const auto& el : doc.elements) {
		OutputElement(fp, el, nullptr, nullptr);
	}
	fputs("</dl>\n", fp);
}

template <typename Container>
static void output_hook_variable_list(FILE* fp, const Container& vars)
{
	fputs("<dl>", fp);
	for (const auto& param : vars) {
		fputs("<dt>", fp);
		ade_output_type_link(fp, param.type);
		fprintf(fp, " <i>%s</i></dt>", param.name);
		fprintf(fp, "<dd><b>Description:</b> %s</dt>", param.description);
	}
	fputs("</dl>", fp);
}

} // namespace

void output_html_doc(const ScriptingDocumentation& doc, const SCP_string& filename)
{
	FILE* fp = fopen(filename.c_str(), "w");

	if (fp == nullptr) {
		return;
	}

	fprintf(fp,
		"<html>\n<head>\n\t<title>Script Output - FSO v%s (%s)</title>\n</head>\n",
		FS_VERSION_FULL,
		doc.name.c_str());
	fputs("<body>", fp);
	fprintf(fp, "\t<h1>Script Output - FSO v%s (%s)</h1>\n", FS_VERSION_FULL, doc.name.c_str());

	// Scripting languages links
	fputs("<dl>", fp);

	//***Hooks
	fputs("<dt><h2>Conditional Hooks</h2></dt>", fp);
	fputs("<dd><dl>", fp);

	// Conditions
	fputs("<dt><b>Conditions</b></dt>", fp);
	for (const auto& conditions : doc.conditions) {
		fprintf(fp, "<dd>%s</dd>", conditions.c_str());
	}

	// Actions
	fputs("<dt><b>Actions</b></dt>", fp);
	fputs("<dd><dl>", fp);
	for (const auto& hook : doc.actions) {
		fprintf(fp, "<dt><b>%s</b></dt>", hook.name.c_str());
		if (!hook.description.empty()) {
			fprintf(fp, "<dd><b>Description:</b> %s", hook.description.c_str());
			// Only write this for hooks with descriptions since this information is useless for legacy hooks
			if (hook.overridable) {
				fputs("<br><i>This hook is overridable</i>", fp);
			} else {
				fputs("<br><i>This hook is <b>not</b> overridable</i>", fp);
			}
			if (!hook.parameters.empty()) {
				fputs("<br><b>Hook Variables:</b>", fp);
				output_hook_variable_list(fp, hook.parameters);
			}
			fputs("</dd>", fp);
		}
	}
	fputs("</dl></dd>", fp);

	fputs("<dt><b>Global Hook Variables:</b> (set globally independent of a specific hook)</dt>", fp);
	output_hook_variable_list(fp, doc.globalVariables);
	fputs("</dl></dd><br />", fp);

	// Languages
	fputs("<dl>", fp);
	//***Version info
	fprintf(fp, "<dd>Lua Version: %s</dd>\n", LUA_RELEASE);

	fputs("<dd>", fp);

	OutputLuaMeta(fp, doc);

	fputs("</dd>", fp);

	//***Enumerations
	fprintf(fp, "<dt id=\"Enumerations\"><h2>Enumerations</h2></dt>");
	for (const auto& enumeration : doc.enumerations) {
		// Cyborg17 -- Omit the deprecated flag
		if (enumeration.name == "VM_EXTERNAL_CAMERA_LOCKED") {
			continue;
		}
		// WMC - For now, print to the file without the description.
		fprintf(fp, "<dd><b>%s</b></dd>", enumeration.name.c_str());
	}
	fputs("</dl></body></html>", fp);

	fclose(fp);
}

} // namespace scripting
