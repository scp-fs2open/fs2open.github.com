//
//

#include "AnimationElement.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include <Rocket/Core.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/GeometryUtilities.h>
#include <Rocket/Core/String.h>
#include <scpui/RocketRenderingInterface.h>

#pragma GCC diagnostic pop

#pragma pop_macro("Assert")

namespace scpui {
namespace elements {

using namespace Rocket;
using namespace Rocket::Core;

// This is basically a copy of ElementImage with a few changes

AnimationElement::AnimationElement(const String& tag_in) : Element(tag_in), dimensions(-1, -1), geometry(this)
{
	ResetCoords();
	geometry_dirty = false;
	texture_dirty  = true;
}

AnimationElement::~AnimationElement() = default;

// Sizes the box to the element's inherent size.
bool AnimationElement::GetIntrinsicDimensions(Vector2f& _dimensions)
{
	// Check if we need to reload the texture.
	if (texture_dirty) {
		LoadTexture();
	}

	// Calculate the x dimension.
	if (HasAttribute("width")) {
		dimensions.x = GetAttribute<float>("width", -1);
	} else if (using_coords) {
		dimensions.x = (float)(coords[2] - coords[0]);
	} else {
		dimensions.x = (float)texture.GetDimensions(GetRenderInterface()).x;
	}

	// Calculate the y dimension.
	if (HasAttribute("height")) {
		dimensions.y = GetAttribute<float>("height", -1);
	} else if (using_coords) {
		dimensions.y = (float)(coords[3] - coords[1]);
	} else {
		dimensions.y = (float)texture.GetDimensions(GetRenderInterface()).y;
	}

	// Return the calculated dimensions. If this changes the size of the element, it will result in
	// a 'resize' event which is caught below and will regenerate the geometry.
	_dimensions = dimensions;
	return true;
}

// Renders the element.
void AnimationElement::OnRender()
{
	// Regenerate the geometry if required (this will be set if 'coords' changes but does not
	// result in a resize).
	if (geometry_dirty) {
		GenerateGeometry();
	}

	// Render the geometry beginning at this element's content region.
	geometry.Render(GetAbsoluteOffset(Rocket::Core::Box::CONTENT));
}

// Called when attributes on the element are changed.
void AnimationElement::OnAttributeChange(const Rocket::Core::AttributeNameList& changed_attributes)
{
	// Call through to the base element's OnAttributeChange().
	Rocket::Core::Element::OnAttributeChange(changed_attributes);

	float dirty_layout = false;

	// Check for a changed 'src' attribute. If this changes, the old texture handle is released,
	// forcing a reload when the layout is regenerated.
	if (changed_attributes.find("src") != changed_attributes.end()) {
		texture_dirty = true;
		dirty_layout  = true;

		// Reset the animation timestamps to make sure the animation starts at the beginning
		animation_last_update_time = -1.0f;
	}

	// Check for a changed 'width' attribute. If this changes, a layout is forced which will
	// recalculate the dimensions.
	if (changed_attributes.find("width") != changed_attributes.end() ||
	    changed_attributes.find("height") != changed_attributes.end()) {
		dirty_layout = true;
	}

	// Check for a change to the 'coords' attribute. If this changes, the coordinates are
	// recomputed and a layout forced.
	if (changed_attributes.find("coords") != changed_attributes.end()) {
		if (HasAttribute("coords")) {
			StringList coords_list;
			StringUtilities::ExpandString(coords_list, GetAttribute<String>("coords", ""));

			if (coords_list.size() != 4) {
				Rocket::Core::Log::Message(
				    Log::LT_WARNING,
				    "Element '%s' has an invalid 'coords' attribute; coords requires 4 values, found %d.",
				    GetAddress().CString(), coords_list.size());
				ResetCoords();
			} else {
				for (size_t i = 0; i < 4; ++i) {
					coords[i] = atoi(coords_list[i].CString());
				}

				// Check for the validity of the coordinates.
				if (coords[0] < 0 || coords[2] < coords[0] || coords[1] < 0 || coords[3] < coords[1]) {
					Rocket::Core::Log::Message(
					    Log::LT_WARNING,
					    "Element '%s' has an invalid 'coords' attribute; invalid coordinate values specified.",
					    GetAddress().CString());
					ResetCoords();
				} else {
					// We have new, valid coordinates; force the geometry to be regenerated.
					geometry_dirty = true;
					using_coords   = true;
				}
			}
		} else {
			ResetCoords();
		}

		// Coordinates have changes; this will most likely result in a size change, so we need to force a layout.
		dirty_layout = true;
	}

	if (dirty_layout) {
		DirtyLayout();
	}
}

// Regenerates the element's geometry.
void AnimationElement::ProcessEvent(Rocket::Core::Event& event)
{
	Element::ProcessEvent(event);

	if (event.GetTargetElement() == this && event == "resize") {
		GenerateGeometry();
	}
}

void AnimationElement::GenerateGeometry()
{
	// Release the old geometry before specifying the new vertices.
	geometry.Release(true);

	std::vector<Rocket::Core::Vertex>& vertices = geometry.GetVertices();
	std::vector<int>& indices                   = geometry.GetIndices();

	vertices.resize(4);
	indices.resize(6);

	// Generate the texture coordinates.
	Vector2f texcoords[2];
	if (using_coords) {
		Vector2f texture_dimensions((float)texture.GetDimensions(GetRenderInterface()).x,
		                            (float)texture.GetDimensions(GetRenderInterface()).y);
		if (texture_dimensions.x == 0) {
			texture_dimensions.x = 1;
		}
		if (texture_dimensions.y == 0) {
			texture_dimensions.y = 1;
		}

		texcoords[0].x = (float)coords[0] / texture_dimensions.x;
		texcoords[0].y = (float)coords[1] / texture_dimensions.y;

		texcoords[1].x = (float)coords[2] / texture_dimensions.x;
		texcoords[1].y = (float)coords[3] / texture_dimensions.y;
	} else {
		texcoords[0] = Vector2f(0, 0);
		texcoords[1] = Vector2f(1, 1);
	}

	Rocket::Core::GeometryUtilities::GenerateQuad(&vertices[0],                                 // vertices to write to
	                                              &indices[0],                                  // indices to write to
	                                              Vector2f(0, 0),                               // origin of the quad
	                                              GetBox().GetSize(Rocket::Core::Box::CONTENT), // size of the quad
	                                              Colourb(255, 255, 255, 255), // colour of the vertices
	                                              texcoords[0],                // top-left texture coordinate
	                                              texcoords[1]);               // top-right texture coordinate

	geometry_dirty = false;
}

bool AnimationElement::LoadTexture()
{
	texture_dirty = false;

	// Get the source URL for the image.
	String image_source = GetAttribute<String>("src", "");
	if (image_source.Empty()) {
		return false;
	}

	geometry_dirty = true;

	Rocket::Core::ElementDocument* document = GetOwnerDocument();
	URL source_url(document == nullptr ? "" : document->GetSourceURL());

	if (!texture.Load(image_source, source_url.GetPath())) {
		geometry.SetTexture(nullptr);
		return false;
	}

	// Set the texture onto our geometry object.
	geometry.SetTexture(&texture);
	return true;
}

void AnimationElement::ResetCoords()
{
	using_coords = false;

	for (int & coord : coords) {
		coord = -1;
	}
}
void AnimationElement::OnUpdate()
{
	Element::OnUpdate();

	if (texture.GetHandle(GetRenderInterface()) == 0) {
		// Make sure that the handle is valid
		return;
	}

	if (animation_last_update_time > 0.0f) {
		const auto animation_frame_time =
			Rocket::Core::GetSystemInterface()->GetElapsedTime() - animation_last_update_time;

		// Set the advance time for the render call so that the generic anim inside the handle gets rendered correctly
		scpui::RocketRenderingInterface::advanceAnimation(texture.GetHandle(GetRenderInterface()),
			animation_frame_time);
	}
	animation_last_update_time = Rocket::Core::GetSystemInterface()->GetElapsedTime();
}

} // namespace elements
} // namespace scpui
