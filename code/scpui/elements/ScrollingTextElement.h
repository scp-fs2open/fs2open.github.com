#pragma once

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/Element.h>
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core/Texture.h>

#pragma pop_macro("Assert")

namespace scpui {
namespace elements {

using namespace Rocket::Core;

class ScrollingTextElement : public Rocket::Core::Element {
  public:
	ScrollingTextElement(const String& tag_in);
	~ScrollingTextElement() override;

  protected:
	void OnAttributeChange(const AttributeNameList& changed_attributes) override;

	void OnBeforeRender() override;
	void OnAfterRender() override;

	void OnChildAdd(Element* child) override;
	void OnChildRemove(Element* child) override;
	void OnUpdate() override;

	float _duration = 1.5f;
	float _animation_start_time = -1.0f;
	bool _newAnimationStarted = false;
};

} // namespace elements
} // namespace scpui
