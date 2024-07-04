//
//

#include "starfield/starfield.h"
#include "background_element.h"
#include "vecmath.h"

namespace scripting {
namespace api {

ADE_OBJ(l_BackgroundElement, background_el_h, "background_element", "Background element handle");

background_el_h::background_el_h(BackgroundType in_type, int in_id) : type(in_type), id(in_id) {}

bool background_el_h::isValid() const
{
	switch (type)
	{
		case BackgroundType::Sun:
			return id >= 0 && id < stars_get_num_suns();
		case BackgroundType::Bitmap:
			return id >= 0 && id < stars_get_num_bitmaps();
		case BackgroundType::Invalid:
		default:
			return false;
	}
}

ADE_FUNC(isValid, l_BackgroundElement, nullptr, "Determines if this handle is valid", "boolean",
         "true if valid, false if not.")
{
	background_el_h* el = nullptr;
	if (!ade_get_args(L, "o", l_BackgroundElement.GetPtr(&el)))
		return ADE_RETURN_FALSE;

	return ade_set_args(L, "b", el->isValid());
}

ADE_VIRTVAR(Orientation, l_BackgroundElement, "orientation", "Backround element orientation (treating the angles as correctly calculated)", "orientation", "Orientation, or null orientation if handle is invalid")
{
	background_el_h* el = nullptr;
	matrix_h* mh = nullptr;
	if (!ade_get_args(L, "o|o", l_BackgroundElement.GetPtr(&el), l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h(&vmd_identity_matrix)));

	if (!el->isValid())
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h(&vmd_identity_matrix)));

	starfield_list_entry sle;
	stars_get_data(el->type == BackgroundType::Sun, el->id, sle);

	if (ADE_SETTING_VAR && mh != nullptr)
	{
		sle.ang = *mh->GetAngles();
		stars_set_data(el->type == BackgroundType::Sun, el->id, sle);
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&sle.ang)));
}

template <typename T>
static int background_element_getset_number_helper(lua_State* L, T starfield_list_entry::* field)
{
	constexpr char type_char = std::conditional<std::is_floating_point<T>::value, std::integral_constant<char, 'f'>, std::integral_constant<char, 'i'>>::type::value;
	constexpr char type_str[] = { type_char, '\0' };
	constexpr char object_and_type_str[] = { 'o', '|', type_char, '\0' };

	background_el_h* el = nullptr;
	T value{};
	if (!ade_get_args(L, object_and_type_str, l_BackgroundElement.GetPtr(&el), &value))
		return ade_set_error(L, type_str, value);

	if (!el->isValid())
		return ade_set_error(L, type_str, value);

	starfield_list_entry sle;
	stars_get_data(el->type == BackgroundType::Sun, el->id, sle);

	if (ADE_SETTING_VAR)
	{
		sle.*field = value;
		stars_set_data(el->type == BackgroundType::Sun, el->id, sle);
	}

	return ade_set_args(L, type_str, sle.*field);
}

template <typename T>
static int background_element_set_number_2x_helper(lua_State* L, T starfield_list_entry::* field1, T starfield_list_entry::* field2)
{
	constexpr char type_char = std::conditional<std::is_floating_point<T>::value, std::integral_constant<char, 'f'>, std::integral_constant<char, 'i'>>::type::value;
	constexpr char object_and_type_str[] = { 'o', type_char, type_char, '\0' };

	background_el_h* el = nullptr;
	T value1{};
	T value2{};
	if (!ade_get_args(L, object_and_type_str, l_BackgroundElement.GetPtr(&el), &value1, &value2))
		return ADE_RETURN_FALSE;

	if (!el->isValid())
		return ADE_RETURN_FALSE;

	starfield_list_entry sle;
	stars_get_data(el->type == BackgroundType::Sun, el->id, sle);

	sle.*field1 = value1;
	sle.*field2 = value2;
	stars_set_data(el->type == BackgroundType::Sun, el->id, sle);

	return ADE_RETURN_TRUE;
}

static int background_element_set_number_4x_helper(lua_State* L)
{
	background_el_h* el = nullptr;
	int div_x = 0;
	int div_y = 0;
	float scale_x = 0.0f;
	float scale_y = 0.0f;
	if (!ade_get_args(L, "offii", l_BackgroundElement.GetPtr(&el), &scale_x, &scale_y, &div_x, &div_y))
		return ADE_RETURN_FALSE;

	if (!el->isValid())
		return ADE_RETURN_FALSE;

	starfield_list_entry sle;
	stars_get_data(el->type == BackgroundType::Sun, el->id, sle);

	sle.scale_x = scale_x;
	sle.scale_y = scale_y;
	sle.div_x = div_x;
	sle.div_y = div_y;
	stars_set_data(el->type == BackgroundType::Sun, el->id, sle);

	return ADE_RETURN_TRUE;
}

ADE_VIRTVAR(DivX, l_BackgroundElement, "number", "Division X", "number", "Division X, or 0 if handle is invalid")
{
	return background_element_getset_number_helper(L, &starfield_list_entry::div_x);
}

ADE_VIRTVAR(DivY, l_BackgroundElement, "number", "Division Y", "number", "Division Y, or 0 if handle is invalid")
{
	return background_element_getset_number_helper(L, &starfield_list_entry::div_y);
}

ADE_VIRTVAR(ScaleX, l_BackgroundElement, "number", "Scale X", "number", "Scale X, or 0 if handle is invalid")
{
	return background_element_getset_number_helper(L, &starfield_list_entry::scale_x);
}

ADE_VIRTVAR(ScaleY, l_BackgroundElement, "number", "Scale Y", "number", "Scale Y, or 0 if handle is invalid")
{
	return background_element_getset_number_helper(L, &starfield_list_entry::scale_y);
}

ADE_FUNC(setDiv, l_BackgroundElement, "number, number", "Sets Division X and Division Y at the same time.  For Bitmaps this avoids a double recalculation of the vertex buffer, if both values need to be set.  For all background elements this also avoids fetching and setting the data twice.", "boolean", "True if the operation was successful")
{
	return background_element_set_number_2x_helper(L, &starfield_list_entry::div_x, &starfield_list_entry::div_y);
}

ADE_FUNC(setScale, l_BackgroundElement, "number, number", "Sets Scale X and Scale Y at the same time.  For Bitmaps this avoids a double recalculation of the vertex buffer, if both values need to be set.  For all background elements this also avoids fetching and setting the data twice.", "boolean", "True if the operation was successful")
{
	return background_element_set_number_2x_helper(L, &starfield_list_entry::scale_x, &starfield_list_entry::scale_y);
}

ADE_FUNC(setScaleAndDiv, l_BackgroundElement, "number, number, number, number", "Sets Scale X, Scale Y, Division X, and Division Y at the same time.  For Bitmaps this avoids a quadruple recalculation of the vertex buffer, if all four values need to be set.  For all background elements this also avoids fetching and setting the data four times.", "boolean", "True if the operation was successful")
{
	return background_element_set_number_4x_helper(L);
}


} // namespace api
} // namespace scripting
