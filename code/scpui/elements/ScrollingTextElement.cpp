
#include "ScrollingTextElement.h"

#include "globalincs/pstypes.h"

#include "gamesnd/gamesnd.h"
#include "scpui/RocketRenderingInterface.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <Rocket/Core.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#pragma pop_macro("Assert")

namespace scpui {
namespace elements {

ScrollingTextElement::ScrollingTextElement(const String& tag_in) : Element(tag_in) {
	// Need to focce a local stacking context so that all elements are rendered without our Render() call. This allows
	// us to set up the rendering context and reset it again once all our children have been rendered.
	ForceLocalStackingContext();
}
ScrollingTextElement::~ScrollingTextElement() = default;

void ScrollingTextElement::OnAttributeChange(const AttributeNameList& changed_attributes)
{
	Element::OnAttributeChange(changed_attributes);

	if (changed_attributes.find("duration") == changed_attributes.end()) {
		_duration = GetAttribute<float>("duration", _duration);
	}
}
void ScrollingTextElement::OnChildAdd(Element* child)
{
	Element::OnChildAdd(child);
	_animation_start_time = Rocket::Core::GetSystemInterface()->GetElapsedTime();
	_newAnimationStarted = true;
}
void ScrollingTextElement::OnChildRemove(Element* child)
{
	Element::OnChildRemove(child);
	_animation_start_time = Rocket::Core::GetSystemInterface()->GetElapsedTime();
	_newAnimationStarted = true;
}
void ScrollingTextElement::OnUpdate()
{
	if (_newAnimationStarted) {
		gamesnd_play_iface(InterfaceSounds::BRIEF_TEXT_WIPE);
		_newAnimationStarted = false;
	}
}
void ScrollingTextElement::OnBeforeRender()
{
	if (_animation_start_time >= 0.0f) {
		Assertion(dynamic_cast<RocketRenderingInterface*>(GetRenderInterface()) != nullptr,
			"This element can only be used with out custom render interface!");
		auto* renderInterface = static_cast<RocketRenderingInterface*>(GetRenderInterface());

		const auto timeSince = Rocket::Core::GetSystemInterface()->GetElapsedTime() - _animation_start_time;
		const auto progress = timeSince / _duration;

		if (progress > 1.0f) {
			renderInterface->setHorizontalSwipeOffset(std::numeric_limits<float>::infinity());
		} else {
			const auto left = GetParentNode()->GetAbsoluteLeft() + GetParentNode()->GetClientLeft();

			const auto swipeLocation = left + progress * GetParentNode()->GetClientWidth();

			// Swipe across in quantized steps to emulate the character by character effect the normal scrolling system
			// does
			const auto quantizedLocation = swipeLocation - std::fmod(swipeLocation, 15.f);

			renderInterface->setHorizontalSwipeOffset(quantizedLocation);
		}
	}
}
void ScrollingTextElement::OnAfterRender()
{
	Assertion(dynamic_cast<RocketRenderingInterface*>(GetRenderInterface()) != nullptr,
		"This element can only be used with out custom render interface!");
	auto* renderInterface = static_cast<RocketRenderingInterface*>(GetRenderInterface());
	// NOTE: This means that we currently do not support nesting these swipe effects but we don't need that at the
	// moment
	renderInterface->setHorizontalSwipeOffset(std::numeric_limits<float>::infinity());
}

} // namespace elements
} // namespace scpui
