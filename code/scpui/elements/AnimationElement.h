#pragma once

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <Rocket/Core/Element.h>
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core/Texture.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#pragma pop_macro("Assert")

namespace scpui {
namespace elements {

using namespace Rocket::Core;

class AnimationElement : public Rocket::Core::Element {
  public:
	AnimationElement(const String& tag_in);
	~AnimationElement() override;

	/// Returns the element's inherent size.
	/// @param[out] The element's intrinsic dimensions.
	/// @return True.
	bool GetIntrinsicDimensions(Vector2f& dimensions) override;

  protected:
	/// Renders the image.
	void OnRender() override;

	void OnUpdate() override;

	/// Checks for changes to the image's source or dimensions.
	/// @param[in] changed_attributes A list of attributes changed on the element.
	void OnAttributeChange(const AttributeNameList& changed_attributes) override;

	/// Regenerates the element's geometry on a resize event.
	/// @param[in] event The event to process.
	void ProcessEvent(Event& event) override;

  private:
	// Generates the element's geometry.
	void GenerateGeometry();
	// Loads the element's texture, as specified by the 'src' attribute.
	bool LoadTexture();
	// Resets the values of the 'coords' attribute to mark them as unused.
	void ResetCoords();

	// The texture this element is rendering from.
	Texture texture;
	// True if we need to refetch the texture's source from the element's attributes.
	bool texture_dirty;
	// The element's computed intrinsic dimensions. If either of these values are set to -1, then
	// that dimension has not been computed yet.
	Vector2f dimensions;

	// The integer coords extracted from the 'coords' attribute. using_coords will be false if
	// these have not been specified or are invalid.
	int coords[4];
	bool using_coords;

	// The geometry used to render this element.
	Geometry geometry;
	bool geometry_dirty;

	float animation_last_update_time = -1.0f;
};

} // namespace elements
} // namespace scpui
