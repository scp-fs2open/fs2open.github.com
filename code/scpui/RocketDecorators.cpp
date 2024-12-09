#include "RocketDecorators.h"

#include <Rocket/Core/RenderInterface.h>
#include <Rocket/Core/Vertex.h>

namespace scpui {
namespace decorators {

// ================================
// Element Underline Implementation
// ================================

DecoratorUnderline::DecoratorUnderline(float thickness, LineStyle style, float length, float space, Rocket::Core::Colourb color, bool element_color)
	: line_thickness(thickness), line_style(style), line_length(length), line_space(space), line_color(color), use_element_color(element_color)
{
}

DecoratorUnderline::~DecoratorUnderline() = default;

// Generate per-element data, if needed (return NULL if not needed)
Rocket::Core::DecoratorDataHandle DecoratorUnderline::GenerateElementData(Rocket::Core::Element* /*element*/)
{
	return 0; // No per-element data needed
}

// Release any element-specific data
void DecoratorUnderline::ReleaseElementData(Rocket::Core::DecoratorDataHandle /*element_data*/)
{
	// No element-specific data to release in this case
}

void DecoratorUnderline::RenderElement(Rocket::Core::Element* element,
	Rocket::Core::DecoratorDataHandle /*element_data*/)
{
	// Get the size of the element's content area
	Rocket::Core::Vector2f element_size = element->GetBox().GetSize(Rocket::Core::Box::CONTENT);

	// Determine line rendering properties based on style
	float dash_length = 5.0f;
	float space_length = 3.0f;

	switch (line_style) {
	case LineStyle::Dotted:
		dash_length = line_thickness; // Dotted lines are small circles
		space_length = line_space;
		break;
	case LineStyle::Dashed:
		space_length = line_space;
		dash_length = line_length;
		break;
	case LineStyle::Solid:
	default:
		space_length = 0; // No space for solid lines
		break;
	}

	// Calculate the starting position (bottom-left of the content area)
	Rocket::Core::Vector2f start = element->GetAbsoluteOffset(Rocket::Core::Box::CONTENT);
	float x_position = start.x;
	float y_position = start.y + element_size.y - (line_thickness * 2); // Adjusted y-position using the thickness

	Rocket::Core::RenderInterface* render_interface = element->GetRenderInterface();

	// Create space for vertices and indices for all dashes
	std::vector<Rocket::Core::Vertex> vertices;
	std::vector<int> indices;

	int vertex_count = 0;

	Rocket::Core::Colourb render_color = line_color;
	if (use_element_color) {
		render_color = element->GetProperty<Rocket::Core::Colourb>("color");
	}

	// Loop through to create each dash as a rectangle (or dot)
	while (x_position < start.x + element_size.x) {
		float dash_end_x = std::min(x_position + dash_length, start.x + element_size.x);

		// Create four vertices for the rectangle (two triangles)
		Rocket::Core::Vertex top_left, top_right, bottom_left, bottom_right;

		// Set vertex positions
		top_left.position = Rocket::Core::Vector2f(x_position, y_position);
		top_right.position = Rocket::Core::Vector2f(dash_end_x, y_position);
		bottom_left.position = Rocket::Core::Vector2f(x_position, y_position + line_thickness);
		bottom_right.position = Rocket::Core::Vector2f(dash_end_x, y_position + line_thickness);

		// Set the color
		top_left.colour = render_color;
		top_right.colour = render_color;
		bottom_left.colour = render_color;
		bottom_right.colour = render_color;

		// Set texture coordinates to 0 since we have no texture
		top_left.tex_coord = Rocket::Core::Vector2f(0, 0);
		top_right.tex_coord = Rocket::Core::Vector2f(0, 0);
		bottom_left.tex_coord = Rocket::Core::Vector2f(0, 0);
		bottom_right.tex_coord = Rocket::Core::Vector2f(0, 0);

		// Add the vertices to the list
		vertices.push_back(top_left);
		vertices.push_back(top_right);
		vertices.push_back(bottom_left);
		vertices.push_back(bottom_right);

		// Add indices for two triangles
		indices.push_back(vertex_count); // First triangle
		indices.push_back(vertex_count + 1);
		indices.push_back(vertex_count + 2);

		indices.push_back(vertex_count + 1); // Second triangle
		indices.push_back(vertex_count + 3);
		indices.push_back(vertex_count + 2);

		// Move to the next dash position
		x_position += dash_length + space_length;

		// Increment vertex count for the next set of triangles
		vertex_count += 4;
	}

	// Render the geometry
	render_interface->RenderGeometry(vertices.data(),
		(int)vertices.size(),
		indices.data(),
		(int)indices.size(),
		0,
		Rocket::Core::Vector2f(0, 0));
}

// =============================
// Corner Borders Implementation
// =============================

DecoratorCornerBorders::DecoratorCornerBorders(float thickness, float length_h, float length_v, Rocket::Core::Colourb color)
	: border_thickness(thickness), border_length_h(length_h), border_length_v(length_v), border_color(color)
{
}

DecoratorCornerBorders::~DecoratorCornerBorders() = default;

// Generate per-element data, if needed (return NULL if not needed)
Rocket::Core::DecoratorDataHandle DecoratorCornerBorders::GenerateElementData(Rocket::Core::Element* /*element*/)
{
	return 0; // No per-element data needed
}

// Release any element-specific data
void DecoratorCornerBorders::ReleaseElementData(Rocket::Core::DecoratorDataHandle /*element_data*/)
{
	// No element-specific data to release in this case
}

void DecoratorCornerBorders::RenderElement(Rocket::Core::Element* element,
	Rocket::Core::DecoratorDataHandle /*element_data*/)
{
	// Get the size of the element's content area
	Rocket::Core::Vector2f element_size = element->GetBox().GetSize(Rocket::Core::Box::CONTENT);
	Rocket::Core::Vector2f start = element->GetAbsoluteOffset(Rocket::Core::Box::CONTENT);

	Rocket::Core::RenderInterface* render_interface = element->GetRenderInterface();

	// Create space for vertices and indices for the corner borders
	std::vector<Rocket::Core::Vertex> vertices;
	std::vector<int> indices;

	int vertex_count = 0;

	// Define the four corner positions, moved inward by thickness
	Rocket::Core::Vector2f top_left = start + Rocket::Core::Vector2f(border_thickness, border_thickness);
	Rocket::Core::Vector2f top_right =
		start + Rocket::Core::Vector2f(element_size.x - border_thickness, border_thickness);
	Rocket::Core::Vector2f bottom_left =
		start + Rocket::Core::Vector2f(border_thickness, element_size.y - border_thickness);
	Rocket::Core::Vector2f bottom_right =
		start + Rocket::Core::Vector2f(element_size.x - border_thickness, element_size.y - border_thickness);

	// Function to add a line segment as two triangles
	auto add_line_segment = [&](const Rocket::Core::Vector2f& start_pos, const Rocket::Core::Vector2f& end_pos, float thickness) {
			// Vector direction of the line
			Rocket::Core::Vector2f direction = end_pos - start_pos;
			// Normalize direction and calculate the perpendicular vector for thickness
			Rocket::Core::Vector2f perpendicular =
				Rocket::Core::Vector2f(-direction.y, direction.x).Normalise() * thickness;

			// Define the four vertices of the line as a rectangle (two triangles)
			Rocket::Core::Vertex v1, v2, v3, v4;
			v1.position = start_pos - perpendicular;
			v2.position = start_pos + perpendicular;
			v3.position = end_pos - perpendicular;
			v4.position = end_pos + perpendicular;

			// Set color
			v1.colour = border_color;
			v2.colour = border_color;
			v3.colour = border_color;
			v4.colour = border_color;

			// Add vertices
			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
			vertices.push_back(v4);

			// Add indices for two triangles
			indices.push_back(vertex_count);
			indices.push_back(vertex_count + 1);
			indices.push_back(vertex_count + 2);

			indices.push_back(vertex_count + 1);
			indices.push_back(vertex_count + 3);
			indices.push_back(vertex_count + 2);

			vertex_count += 4;
		};

	// Define the lengths for horizontal and vertical borders
	Rocket::Core::Vector2f horizontal(border_length_h, 0);
	Rocket::Core::Vector2f vertical(0, border_length_v);

	// Add corner borders by adding two line segments for each corner, moved inward by thickness
	// Top-left corner
	add_line_segment(top_left, top_left + horizontal, border_thickness); // Horizontal line
	add_line_segment(top_left, top_left + vertical, border_thickness);   // Vertical line

	// Top-right corner
	add_line_segment(top_right, top_right - horizontal, border_thickness); // Horizontal line
	add_line_segment(top_right, top_right + vertical, border_thickness);   // Vertical line

	// Bottom-left corner
	add_line_segment(bottom_left, bottom_left + horizontal, border_thickness); // Horizontal line
	add_line_segment(bottom_left, bottom_left - vertical, border_thickness);   // Vertical line

	// Bottom-right corner
	add_line_segment(bottom_right, bottom_right - horizontal, border_thickness); // Horizontal line
	add_line_segment(bottom_right, bottom_right - vertical, border_thickness);   // Vertical line

	// Render the geometry
	render_interface->RenderGeometry(vertices.data(),
		(int)vertices.size(),
		indices.data(),
		(int)indices.size(),
		0,
		Rocket::Core::Vector2f(0, 0));
}

} // namespace decorators
} // namespace scpui
