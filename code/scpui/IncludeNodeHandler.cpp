//
//

#include "IncludeNodeHandler.h"

#include "globalincs/pstypes.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/Log.h>
#include <Rocket/Core/StreamFile.h>
#include <Rocket/Core/XMLParseTools.h>

#pragma pop_macro("Assert")

using namespace Rocket;
using namespace Rocket::Core;

namespace scpui {

Element* IncludeNodeHandler::ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes)
{
	Assertion(name == "include", "IncludeNodeHandler used with invalid tag '%s'!", name.CString());

	String include_src;
	if (!attributes.GetInto("src", include_src)) {
		Log::ParseError(parser->GetParseFrame()->element->GetOwnerDocument()->GetSourceURL(), parser->GetLineNumber(),
		                "'include' requires a 'src' attribute!");
		return parser->GetParseFrame()->element;
	}

	auto stream = new StreamFile();

	if (!stream->Open(include_src)) {
		Log::ParseError(parser->GetParseFrame()->element->GetOwnerDocument()->GetSourceURL(), parser->GetLineNumber(),
		                "Failed to open file '%s'!", include_src.CString());
		stream->RemoveReference();
		return parser->GetParseFrame()->element;
	}

	XMLParser include_parser(parser->GetParseFrame()->element);
	include_parser.Parse(stream);

	stream->RemoveReference();

	// Tell the parser to use the element handler for all child nodes
	parser->PushDefaultHandler();

	return parser->GetParseFrame()->element;
}
bool IncludeNodeHandler::ElementEnd(XMLParser* /*parser*/, const String& /*name*/) { return true; }
bool IncludeNodeHandler::ElementData(XMLParser* /*parser*/, const String& /*data*/) { return true; }
void IncludeNodeHandler::Release() { delete this; }

} // namespace scpui
