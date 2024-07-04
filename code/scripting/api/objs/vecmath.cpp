
#include "vecmath.h"

#include "math/vecmat.h"
#include "render/3d.h"
#include "render/3dinternal.h"

#include "network/multi.h"
#include "network/multimsgs.h"

void vec3d::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	vec3d vec;
	value.getValue(scripting::api::l_Vector.Get(&vec));
	ADD_VECTOR(vec);
}

void vec3d::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	vec3d vec;
	GET_VECTOR(vec);
	new(data_ptr) vec3d(std::move(vec));
}

namespace scripting {
namespace api {

//**********OBJECT: orientation matrix
//WMC - So matrix can use vector, I define it up here.
ADE_OBJ(l_Vector, vec3d, "vector", "Vector object");

void matrix_h::ValidateAngles() {
	if (status == MatrixState::AnglesOutOfDate) {
		vm_extract_angles_matrix(&ang, &mtx);
		status = MatrixState::Fine;
	}
}
void matrix_h::ValidateMatrix() {
	if (status == MatrixState::MatrixOutOfdate) {
		vm_angles_2_matrix(&mtx, &ang);
		status = MatrixState::Fine;
	}
}
matrix_h::matrix_h() {
	mtx = vmd_identity_matrix;
	status = MatrixState::AnglesOutOfDate;
}
matrix_h::matrix_h(const matrix* in) {
	mtx = *in;
	status = MatrixState::AnglesOutOfDate;
}
matrix_h::matrix_h(const angles* in) {
	ang = *in;
	status = MatrixState::MatrixOutOfdate;
}
matrix_h::matrix_h(const vec3d *fvec, const vec3d *uvec, const vec3d *rvec) {
	vm_vector_2_matrix(&mtx, fvec, uvec, rvec);
	status = MatrixState::AnglesOutOfDate;
}
angles* matrix_h::GetAngles() {
	this->ValidateAngles();
	return &ang;
}
matrix* matrix_h::GetMatrix() {
	this->ValidateMatrix();
	return &mtx;
}
void matrix_h::SetStatus(MatrixState n_status) {
	status = n_status;
}

void matrix_h::serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size) {
	matrix_h mat;
	value.getValue(l_Matrix.Get(&mat));
	vec3d vec = *reinterpret_cast<vec3d*>(mat.GetAngles());
	ADD_VECTOR(vec);
}

void matrix_h::deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset) {
	vec3d orientationAngles;
	GET_VECTOR(orientationAngles);
	new(data_ptr) matrix_h(reinterpret_cast<angles*>(&orientationAngles));
}

//LOOK LOOK LOOK LOOK LOOK LOOK
//IMPORTANT!!!:
//LOOK LOOK LOOK LOOK LOOK LOOK
//Don't forget to set status appropriately when you change ang or mtx.

ADE_OBJ(l_Matrix, matrix_h, "orientation", "Orientation matrix object");

ADE_INDEXER(l_Matrix,
	ade_overload_list({"string axis /* p, b, h */", "number element /* 1-9 */"}),
	"Orientation component - pitch, bank, heading, or index into 3x3 matrix (1-9)",
	"number",
	"Number at the specified index, or 0 if index is invalid.")
{
	matrix_h* mh;
	const char* s = nullptr;
	float newval = 0.0f;
	int numargs = ade_get_args(L, "os|f", l_Matrix.GetPtr(&mh), &s, &newval);

	if (!numargs || s[1] != '\0') {
		return ade_set_error(L, "f", 0.0f);
	}

	int idx = 0;
	if (s[0] == 'p') {
		idx = -1;
	} else if (s[0] == 'b') {
		idx = -2;
	} else if (s[0] == 'h') {
		idx = -3;
	} else if (atoi(s)) {
		idx = atoi(s);
	}

	if (idx < -3 || idx == 0 || idx > 9) {
		return ade_set_error(L, "f", 0.0f);
	}

//Handle out of date stuff.
	float* val = NULL;
	if (idx < 0) {
		angles* ang = mh->GetAngles();

		if (idx == -1) {
			val = &ang->p;
		}
		if (idx == -2) {
			val = &ang->b;
		}
		if (idx == -3) {
			val = &ang->h;
		}
		
	} else {
		idx--;    //Lua->FS2
		val = &mh->GetMatrix()->a1d[idx];
	}

	if (ADE_SETTING_VAR && *val != newval) {
//WMC - I figure this is quicker
//than just assuming matrix or angles is diff
//and recalculating every time.

		if (idx < 0) {
			mh->SetStatus(MatrixState::MatrixOutOfdate);
		} else {
			mh->SetStatus(MatrixState::AnglesOutOfDate);
		}

//Might as well put this here
		*val = newval;
	}

	return ade_set_args(L, "f", *val);
}

ADE_FUNC(__mul,
		 l_Matrix,
		 "orientation",
		 "Multiplies two matrix objects)",
		 "orientation",
		 "matrix, or empty matrix if unsuccessful") {
	matrix_h* mha = NULL, * mhb = NULL;
	if (!ade_get_args(L, "oo", l_Matrix.GetPtr(&mha), l_Matrix.GetPtr(&mhb))) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix mr;

	vm_matrix_x_matrix(&mr, mha->GetMatrix(), mhb->GetMatrix());

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&mr)));
}

ADE_FUNC(__tostring,
		 l_Matrix,
		 NULL,
		 "Converts a matrix to a string with format \"[r1c1 r2c1 r3c1 | r1c2 r2c2 r3c2| r1c3 r2c3 r3c3]\"",
		 "string",
		 "Formatted string or \"<NULL\"") {
	matrix_h* mh;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "s", "<NULL>");
	}

	char buf[128];
	float* a = &mh->GetMatrix()->a1d[0];
	sprintf(buf, "[%f %f %f | %f %f %f | %f %f %f]", a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);

	return ade_set_args(L, "s", buf);
}

ADE_FUNC(copy,
		 l_Matrix,
		 nullptr,
		 "Returns a copy of the orientation",
		 "orientation",
		 "The copy, or null orientation on failure")
{
	matrix_h *mh = nullptr;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh)))
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	
	return ade_set_args(L, "o", l_Matrix.Set(*mh));
}

ADE_FUNC(getInterpolated,
		 l_Matrix,
		 "orientation Final, number Factor",
		 "Returns orientation that has been interpolated to Final by Factor (0.0-1.0).  This is a pure linear interpolation with no consideration given to matrix validity or normalization.  You may want 'rotationalInterpolate' instead.",
		 "orientation",
		 "Interpolated orientation, or null orientation on failure") {
	matrix_h* oriA = NULL;
	matrix_h* oriB = NULL;
	float factor = 0.0f;
	if (!ade_get_args(L, "oof", l_Matrix.GetPtr(&oriA), l_Matrix.GetPtr(&oriB), &factor)) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix* A = oriA->GetMatrix();
	matrix* B = oriB->GetMatrix();
	matrix final;

	//matrix subtraction & scaling
	for (int i = 0; i < 9; i++) {
		final.a1d[i] = A->a1d[i] + (B->a1d[i] - A->a1d[i]) * factor;
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&final)));
}

ADE_FUNC(rotationalInterpolate,
	l_Matrix,
	"orientation final, number t",
	"Interpolates between this (initial) orientation and a second one, using t as the multiplier of progress between them.  Intended values for t are [0.0f, 1.0f], but values outside this range are allowed.",
	"orientation",
	"The interpolated orientation, or NIL if any handle is invalid")
{
	matrix_h *initial_h, *final_h;
	float t;
	if (!ade_get_args(L, "oof", l_Matrix.GetPtr(&initial_h), l_Matrix.GetPtr(&final_h), &t))
		return ADE_RETURN_NIL;

	matrix interpolated;
	vm_interpolate_matrices(&interpolated, initial_h->GetMatrix(), final_h->GetMatrix(), t);

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&interpolated)));
}

ADE_FUNC(getTranspose,
		 l_Matrix,
		 NULL,
		 "Returns a transpose version of the specified orientation",
		 "orientation",
		 "Transpose matrix, or null orientation on failure") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	matrix final = *mh->GetMatrix();
	vm_transpose(&final);

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(&final)));
}


ADE_FUNC(rotateVector,
		 l_Matrix,
		 "vector Input",
		 "Returns rotated version of given vector",
		 "vector",
		 "Rotated vector, or empty vector on error") {
	matrix_h* mh;
	vec3d* v3;
	if (!ade_get_args(L, "oo", l_Matrix.GetPtr(&mh), l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	vec3d v3r;
	vm_vec_rotate(&v3r, v3, mh->GetMatrix());

	return ade_set_args(L, "o", l_Vector.Set(v3r));
}

ADE_FUNC(unrotateVector,
		 l_Matrix,
		 "vector Input",
		 "Returns unrotated version of given vector",
		 "vector",
		 "Unrotated vector, or empty vector on error") {
	matrix_h* mh;
	vec3d* v3;
	if (!ade_get_args(L, "oo", l_Matrix.GetPtr(&mh), l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	vec3d v3r;
	vm_vec_unrotate(&v3r, v3, mh->GetMatrix());

	return ade_set_args(L, "o", l_Vector.Set(v3r));
}

ADE_FUNC(getUvec,
		 l_Matrix,
		 NULL,
		 "Returns the vector that points up (0,1,0 unrotated by this matrix)",
		 "vector",
		 "Vector or null vector on error") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(mh->GetMatrix()->vec.uvec));
}

ADE_FUNC(getFvec,
		 l_Matrix,
		 NULL,
		 "Returns the vector that points to the front (0,0,1 unrotated by this matrix)",
		 "vector",
		 "Vector or null vector on error") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(mh->GetMatrix()->vec.fvec));
}

ADE_FUNC(getRvec,
		 l_Matrix,
		 NULL,
		 "Returns the vector that points to the right (1,0,0 unrotated by this matrix)",
		 "vector",
		 "Vector or null vector on error") {
	matrix_h* mh = NULL;
	if (!ade_get_args(L, "o", l_Matrix.GetPtr(&mh))) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	return ade_set_args(L, "o", l_Vector.Set(mh->GetMatrix()->vec.rvec));
}

ADE_INDEXER(l_Vector,
	ade_overload_list({"string axis /* x,y,z */", "number element /* 1-3 */"}),
	"Vector component",
	"number",
	"Value at index, or 0 if vector handle is invalid")
{
	vec3d* v3;
	const char* s = nullptr;
	float newval = 0.0f;
	int numargs = ade_get_args(L, "os|f", l_Vector.GetPtr(&v3), &s, &newval);

	if (!numargs || s[1] != '\0') {
		return ade_set_error(L, "f", 0.0f);
	}

	int idx = -1;
	if (s[0] == 'x' || s[0] == '1') {
		idx = 0;
	} else if (s[0] == 'y' || s[0] == '2') {
		idx = 1;
	} else if (s[0] == 'z' || s[0] == '3') {
		idx = 2;
	}

	if (idx < 0 || idx > 3) {
		return ade_set_error(L, "f", 0.0f);
	}

	if (ADE_SETTING_VAR) {
		v3->a1d[idx] = newval;
	}

	return ade_set_args(L, "f", v3->a1d[idx]);
}

ADE_FUNC(__add,
		 l_Vector,
		 "number/vector",
		 "Adds vector by another vector, or adds all axes by value",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d v3 = vmd_zero_vector;
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			v3.xyz.x += f;
			v3.xyz.y += f;
			v3.xyz.z += f;
		}
	} else {
		vec3d v3b;
		//WMC - doesn't really matter which is which
		if (ade_get_args(L, "oo", l_Vector.Get(&v3), l_Vector.Get(&v3b))) {
			vm_vec_add2(&v3, &v3b);
		}
	}
	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(__sub,
		 l_Vector,
		 "number/vector",
		 "Subtracts vector from another vector, or subtracts all axes by value",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d v3 = vmd_zero_vector;
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			v3.xyz.x += f;
			v3.xyz.y += f;
			v3.xyz.z += f;
		}
	} else {
		vec3d v3b;
		//WMC - doesn't really matter which is which
		if (ade_get_args(L, "oo", l_Vector.Get(&v3), l_Vector.Get(&v3b))) {
			vm_vec_sub2(&v3, &v3b);
		}
	}

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(__mul,
		 l_Vector,
		 "number/vector",
		 "Scales vector object (Multiplies all axes by number), or multiplies each axes by the other vector's axes.",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d v3 = vmd_zero_vector;
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			vm_vec_scale(&v3, f);
		}
	} else {
		vec3d* v1 = NULL;
		vec3d* v2 = NULL;
		if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2))) {
			return ade_set_args(L, "o", l_Vector.Set(vmd_zero_vector));
		}

		v3.xyz.x = v1->xyz.x * v2->xyz.x;
		v3.xyz.y = v1->xyz.y * v2->xyz.y;
		v3.xyz.z = v1->xyz.z * v2->xyz.z;
	}

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(__div,
		 l_Vector,
		 "number/vector",
		 "Scales vector object (Divide all axes by number), or divides each axes by the dividing vector's axes.",
		 "vector",
		 "Final vector, or null vector if error occurs") {
	vec3d v3 = vmd_zero_vector;
	if (lua_isnumber(L, 1) || lua_isnumber(L, 2)) {
		float f;
		if ((lua_isnumber(L, 1) && ade_get_args(L, "fo", &f, l_Vector.Get(&v3)))
			|| (lua_isnumber(L, 2) && ade_get_args(L, "of", l_Vector.Get(&v3), &f))) {
			vm_vec_scale(&v3, 1.0f / f);
		}
	} else {
		vec3d* v1 = NULL;
		vec3d* v2 = NULL;
		if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v1), l_Vector.GetPtr(&v2))) {
			return ade_set_args(L, "o", l_Vector.Set(vmd_zero_vector));
		}

		v3.xyz.x = v1->xyz.x / v2->xyz.x;
		v3.xyz.y = v1->xyz.y / v2->xyz.y;
		v3.xyz.z = v1->xyz.z / v2->xyz.z;
	}

	return ade_set_args(L, "o", l_Vector.Set(v3));
}


ADE_FUNC(__tostring,
		 l_Vector,
		 NULL,
		 "Converts a vector to string with format \"(x,y,z)\"",
		 "string",
		 "Vector as string, or empty string if handle is invalid") {
	vec3d* v3;
	if (!ade_get_args(L, "o", l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "s", "");
	}

	char buf[128];
	sprintf(buf, "(%f,%f,%f)", v3->xyz.x, v3->xyz.y, v3->xyz.z);

	return ade_set_args(L, "s", buf);
}

ADE_FUNC(copy,
		 l_Vector,
		 nullptr,
		 "Returns a copy of the vector",
		 "vector",
		 "The copy, or null vector on failure")
{
	vec3d *v = nullptr;
	if (!ade_get_args(L, "o", l_Vector.GetPtr(&v)))
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	
	return ade_set_args(L, "o", l_Vector.Set(*v));
}

ADE_FUNC(getInterpolated,
	l_Vector,
	"vector Final, number Factor",
	"Returns vector that has been interpolated to Final by Factor (0.0-1.0)",
	"vector",
	"Interpolated vector, or null vector on failure") {
	vec3d *A = nullptr;
	vec3d *B = nullptr;
	float factor = 0.0f;
	if (!ade_get_args(L, "oof", l_Vector.GetPtr(&A), l_Vector.GetPtr(&B), &factor)) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	vec3d final;

	//vector subtraction & scaling
	for (int i = 0; i < 3; i++) {
		final.a1d[i] = A->a1d[i] + (B->a1d[i] - A->a1d[i]) * factor;
	}

	return ade_set_args(L, "o", l_Vector.Set(final));
}

ADE_FUNC(rotationalInterpolate,
	l_Vector,
	"vector final, number t",
	"Interpolates between this (initial) vector and a second one, using t as the multiplier of progress between them, rotating around their cross product vector.  Intended values for t are [0.0f, 1.0f], but values outside this range are allowed.",
	"vector",
	"The interpolated vector, or NIL if any handle is invalid")
{
	vec3d *initial, *final;
	float t;
	if (!ade_get_args(L, "oof", l_Vector.GetPtr(&initial), l_Vector.GetPtr(&final), &t))
		return ADE_RETURN_NIL;

	vec3d interpolated;
	vm_vec_interp_constant(&interpolated, initial, final, t);

	return ade_set_args(L, "o", l_Vector.Set(interpolated));
}

ADE_FUNC(getOrientation,
		 l_Vector,
		 NULL,
		 "Returns orientation object representing the direction of the vector. Does not require vector to be normalized.  "
		 "Note: the orientation is constructed with the vector as the forward vector (fvec).  You can also specify up (uvec) and right (rvec) vectors as optional arguments.",
		 "orientation",
		 "Orientation object, or null orientation object if handle is invalid") {
	vec3d *fvec = nullptr;
	vec3d *uvec = nullptr;
	vec3d *rvec = nullptr;

	if (!ade_get_args(L, "o|oo", l_Vector.GetPtr(&fvec), l_Vector.GetPtr(&uvec), l_Vector.GetPtr(&rvec))) {
		return ade_set_error(L, "o", l_Matrix.Set(matrix_h()));
	}

	return ade_set_args(L, "o", l_Matrix.Set(matrix_h(fvec, uvec, rvec)));
}

ADE_FUNC(getMagnitude,
		 l_Vector,
		 NULL,
		 "Returns the magnitude of a vector (Total regardless of direction)",
		 "number",
		 "Magnitude of vector, or 0 if handle is invalid") {
	vec3d* v3;
	if (!ade_get_args(L, "o", l_Vector.GetPtr(&v3))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_mag(v3));
}

ADE_FUNC(getDistance, l_Vector, "vector otherPos", "Distance", "number", "Returns distance from another vector") {
	vec3d* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_dist(v3a, v3b));
}

ADE_FUNC(getDistanceSquared, l_Vector, "vector otherPos", "Distance squared", "number", "Returns distance squared from another vector") {
	vec3d* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_dist_squared(v3a, v3b));
}

ADE_FUNC(getDotProduct,
		 l_Vector,
		 "vector OtherVector",
		 "Returns dot product of vector object with vector argument",
		 "number",
		 "Dot product, or 0 if a handle is invalid") {
	vec3d* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "f", 0.0f);
	}

	return ade_set_args(L, "f", vm_vec_dot(v3a, v3b));
}

ADE_FUNC(getCrossProduct,
		 l_Vector,
		 "vector OtherVector",
		 "Returns cross product of vector object with vector argument",
		 "vector",
		 "Cross product, or null vector if a handle is invalid") {
	vec3d* v3a, * v3b;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&v3a), l_Vector.GetPtr(&v3b))) {
		return ade_set_error(L, "o", l_Vector.Set(vmd_zero_vector));
	}

	vec3d v3r;
	vm_vec_cross(&v3r, v3a, v3b);

	return ade_set_args(L, "o", l_Vector.Set(v3r));
}

ADE_FUNC(getScreenCoords,
	l_Vector,
	nullptr,
	"Gets screen cordinates of a world vector",
	"number, number",
	"X (number), Y (number), or false if off-screen")
{
	vec3d v3;
	if (!ade_get_args(L, "o", l_Vector.Get(&v3))) {
		return ADE_RETURN_NIL;
	}

	vertex vtx;
	bool do_g3 = G3_count < 1;
	if (do_g3)
		g3_start_frame(1);

	g3_rotate_vertex(&vtx, &v3);
	g3_project_vertex(&vtx);

	if (do_g3)
		g3_end_frame();

	if (vtx.flags & PF_OVERFLOW) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "ff", vtx.screen.xyw.x, vtx.screen.xyw.y);
}

ADE_FUNC(getNormalized,
		 l_Vector,
		 NULL,
		 "Returns a normalized version of the vector",
		 "vector",
		 "Normalized Vector, or NIL if invalid") {
	vec3d v3;
	if (!ade_get_args(L, "o", l_Vector.Get(&v3))) {
		return ADE_RETURN_NIL;
	}

	vm_vec_normalize(&v3);

	return ade_set_args(L, "o", l_Vector.Set(v3));
}

ADE_FUNC(projectParallel,
	l_Vector,
	"vector unitVector",
	"Returns a projection of the vector along a unit vector.  The unit vector MUST be normalized.",
	"vector",
	"The projected vector, or NIL if a handle is invalid")
{
	vec3d *src, *unit;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&src), l_Vector.GetPtr(&unit)))
		return ADE_RETURN_NIL;

	if (!vm_vec_is_normalized(unit))
	{
		LuaError(L, "The unit vector MUST be normalized!");
		return ADE_RETURN_NIL;
	}

	vec3d dest;
	vm_vec_projection_parallel(&dest, src, unit);
	return ade_set_args(L, "o", l_Vector.Set(dest));
}

ADE_FUNC(projectOntoPlane,
	l_Vector,
	"vector surfaceNormal",
	"Returns a projection of the vector onto a plane defined by a surface normal.  The surface normal MUST be normalized.",
	"vector",
	"The projected vector, or NIL if a handle is invalid")
{
	vec3d *src, *normal;
	if (!ade_get_args(L, "oo", l_Vector.GetPtr(&src), l_Vector.GetPtr(&normal)))
		return ADE_RETURN_NIL;

	if (!vm_vec_is_normalized(normal))
	{
		LuaError(L, "The surface normal MUST be normalized!");
		return ADE_RETURN_NIL;
	}

	vec3d dest;
	vm_vec_projection_onto_plane(&dest, src, normal);
	return ade_set_args(L, "o", l_Vector.Set(dest));
}

ADE_FUNC(findNearestPointOnLine,
	l_Vector,
	"vector point1, vector point2",
	"Finds the point on the line defined by point1 and point2 that is closest to this point.  (The line is assumed to extend infinitely in both directions; the closest point will not necessarily be between the two points.)",
	"vector, number",
	"Returns two arguments.  The first is the nearest point, and the second is a value indicating where on the line the point lies.  From the code: '0.0 means nearest_point is p1; 1.0 means it's p2; 2.0 means it's beyond p2 by 2x; -1.0 means it's \"before\" p1 by 1x'.")
{
	vec3d *point, *p0, *p1;
	if (!ade_get_args(L, "ooo", l_Vector.GetPtr(&point), l_Vector.GetPtr(&p0), l_Vector.GetPtr(&p1)))
		return ADE_RETURN_NIL;

	vec3d dest;
	float f = find_nearest_point_on_line(&dest, p0, p1, point);
	return ade_set_args(L, "of", l_Vector.Set(dest), f);
}

ADE_FUNC(perturb,
	l_Vector,
	"number angle1, [number angle2]",
	"Create a new normalized vector, randomly perturbed around a given (normalized) vector.  Angles are in degrees.  If only one angle is specified, it is the max angle.  If both are specified, the first is the minimum and the second is the maximum.",
	"vector",
	"A vector, somewhat perturbed from the experience")
{
	vec3d *in, out;
	float angle1, angle2;

	int numargs = ade_get_args(L, "of|f", l_Vector.GetPtr(&in), &angle1, &angle2);
	if (numargs < 2)
		return ADE_RETURN_NIL;

	if (numargs == 2)
		vm_vec_random_cone(&out, in, angle1);
	else
		vm_vec_random_cone(&out, in, angle1, angle2);

	return ade_set_args(L, "o", l_Vector.Set(out));
}

ADE_FUNC(perturb,
	l_Matrix,
	"number angle1, [number angle2]",
	"Create a new normalized vector, randomly perturbed around a cone in the given orientation.  Angles are in degrees.  If only one angle is specified, it is the max angle.  If both are specified, the first is the minimum and the second is the maximum.",
	"vector",
	"A vector, somewhat perturbed from the experience")
{
	matrix_h* mh = nullptr;
	vec3d out;
	float angle1, angle2;

	int numargs = ade_get_args(L, "of|f", l_Matrix.GetPtr(&mh), &angle1, &angle2);
	if (numargs < 2)
		return ADE_RETURN_NIL;

	if (numargs == 2)
		vm_vec_random_cone(&out, nullptr, angle1, mh->GetMatrix());
	else
		vm_vec_random_cone(&out, nullptr, angle1, angle2, mh->GetMatrix());

	return ade_set_args(L, "o", l_Vector.Set(out));
}

ADE_FUNC(randomInCircle,
	l_Vector,
	"orientation orient, number radius, boolean on_edge, [boolean bias_towards_center = true]",
	"Given this vector (the origin point), an orientation, and a radius, generate a point on the plane of the circle.  If on_edge is true, the point will be on the edge of the circle. If bias_towards_center is true, the probability will be higher towards the center.",
	"vector",
	"A point within the plane of the circle")
{
	vec3d *in, out;
	matrix_h* mh = nullptr;
	float radius;
	bool on_edge;
	bool bias_towards_center = true;

	if (!ade_get_args(L, "oofb|b", l_Vector.GetPtr(&in), l_Matrix.GetPtr(&mh), &radius, &on_edge, &bias_towards_center))
		return ADE_RETURN_NIL;

	vm_vec_random_in_circle(&out, in, mh->GetMatrix(), radius, on_edge, bias_towards_center);

	return ade_set_args(L, "o", l_Vector.Set(out));
}

ADE_FUNC(randomInSphere,
	l_Vector,
	"number radius, boolean on_surface, [boolean bias_towards_center = true]",
	"Given this vector (the origin point) and a radius, generate a point in the volume of the sphere.  If on_surface is true, the point will be on the surface of the sphere. If bias_towards_center is true, the probability will be higher towards the center",
	"vector",
	"A point within the plane of the circle")
{
	vec3d *in, out;
	float radius;
	bool on_surface;
	bool bias_towards_center = true;

	if (!ade_get_args(L, "ofb|b", l_Vector.GetPtr(&in), &radius, &on_surface, &bias_towards_center))
		return ADE_RETURN_NIL;

	vm_vec_random_in_sphere(&out, in, radius, on_surface, bias_towards_center);

	return ade_set_args(L, "o", l_Vector.Set(out));
}


}
}
