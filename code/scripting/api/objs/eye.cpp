//
//

#include "eye.h"
#include "vecmath.h"

namespace scripting {
namespace api {

eye_h::eye_h() {
	model = -1;
	eye_idx = -1;
}

eye_h::eye_h(int n_m, int n_e) {
	model = n_m;
	eye_idx = n_e;
}

bool eye_h::isValid() const {
	polymodel* pm = NULL;
	return (model > -1
		&& (pm = model_get(model)) != NULL
		&& eye_idx > -1
		&& eye_idx < pm->n_view_positions);
}


ADE_OBJ(l_Eyepoint, eye_h, "eyepoint", "Eyepoint handle");

ADE_VIRTVAR(Normal, l_Eyepoint, "vector", "Eyepoint normal", "vector", "Eyepoint normal, or null vector if handle is invalid")
{
	eye_h *eh;
	vec3d *v;
	if(!ade_get_args(L, "o|o", l_Eyepoint.GetPtr(&eh), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!eh->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = model_get(eh->model);

	if(ADE_SETTING_VAR && v != NULL)
	{
		pm->view_positions[eh->eye_idx].norm = *v;
	}

	return ade_set_args(L, "o", l_Vector.Set(pm->view_positions[eh->eye_idx].norm));
}

ADE_VIRTVAR(Position, l_Eyepoint, "vector", "Eyepoint location (Local vector)", "vector", "Eyepoint location, or null vector if handle is invalid")
{
	eye_h *eh;
	vec3d *v;
	if(!ade_get_args(L, "o|o", l_Eyepoint.GetPtr(&eh), l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	if(!eh->isValid())
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));

	polymodel *pm = model_get(eh->model);

	if(ADE_SETTING_VAR && v != NULL)
	{
		pm->view_positions[eh->eye_idx].pnt = *v;
	}

	return ade_set_args(L, "o", l_Vector.Set(pm->view_positions[eh->eye_idx].pnt));
}

ADE_FUNC(isValid, l_Eyepoint, nullptr, "Detect whether this handle is valid", "boolean", "true if valid false otherwise")
{
	eye_h* eh = nullptr;
	if (!ade_get_args(L, "o", l_Eyepoint.GetPtr(&eh))) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", eh->isValid());
}

ADE_FUNC_DEPRECATED(IsValid,
	l_Eyepoint,
	nullptr,
	"Detect whether this handle is valid",
	"boolean",
	"true if valid false otherwise",
	gameversion::version(24, 0),
	"IsValid is named with the incorrect case. Use isValid instead.")
{
	eye_h *eh = nullptr;
	if (!ade_get_args(L, "o", l_Eyepoint.GetPtr(&eh)))
	{
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", eh->isValid());
}

}
}
