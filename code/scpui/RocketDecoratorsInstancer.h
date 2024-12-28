#pragma once

#include "RocketDecorators.h"

#include <Rocket/Core/DecoratorInstancer.h>

namespace scpui {
namespace decorators {

class UnderlineDecoratorInstancer : public Rocket::Core::DecoratorInstancer {
  public:
	UnderlineDecoratorInstancer();
	~UnderlineDecoratorInstancer() override = default;

	// Instances the underline decorator
	Rocket::Core::Decorator* InstanceDecorator(const Rocket::Core::String& name,
		const Rocket::Core::PropertyDictionary& properties) override;

	// Releases the underline decorator
	void ReleaseDecorator(Rocket::Core::Decorator* decorator) override;

	// Releases the instancer itself
	void Release() override;
};

class BorderDecoratorInstancer : public Rocket::Core::DecoratorInstancer {
  public:
	BorderDecoratorInstancer();
	~BorderDecoratorInstancer() override = default;

	// Instances the border decorator
	Rocket::Core::Decorator* InstanceDecorator(const Rocket::Core::String& name,
		const Rocket::Core::PropertyDictionary& properties) override;

	// Releases the border decorator
	void ReleaseDecorator(Rocket::Core::Decorator* decorator) override;

	// Releases the instancer itself
	void Release() override;
};

} // namespace decorators
} // namespace scpui
