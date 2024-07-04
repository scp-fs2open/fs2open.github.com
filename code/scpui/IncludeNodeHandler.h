#pragma once

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <Rocket/Core/XMLNodeHandler.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#pragma pop_macro("Assert")

namespace scpui {

class IncludeNodeHandler : public Rocket::Core::XMLNodeHandler {
  public:
	Rocket::Core::Element* ElementStart(Rocket::Core::XMLParser* parser,
	                                    const Rocket::Core::String& name,
	                                    const Rocket::Core::XMLAttributes& attributes) override;
	bool ElementEnd(Rocket::Core::XMLParser* parser, const Rocket::Core::String& name) override;
	bool ElementData(Rocket::Core::XMLParser* parser, const Rocket::Core::String& data) override;
	void Release() override;
};
} // namespace scpui
