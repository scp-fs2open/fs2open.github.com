//
//

#ifndef FS2_OPEN_VECMATH_H
#define FS2_OPEN_VECMATH_H

#include "globalincs/pstypes.h"
#include "scripting/ade.h"
#include "scripting/ade_api.h"
#include "math/vecmat.h"

namespace scripting {
namespace api {

DECLARE_ADE_OBJ(l_Vector, vec3d);

//WMC - Due to the exorbitant times required to store matrix data,
//I initially store the matrix in this struct.
enum class MatrixState {
	Fine,
	MatrixOutOfdate,
	AnglesOutOfDate
};
struct matrix_h {
 private:
	MatrixState status;

	matrix mtx;
	angles ang;

	//WMC - Call these to make sure what you want
	//is up to date
	void ValidateAngles();

	void ValidateMatrix();
 public:
	matrix_h();
	explicit matrix_h(const matrix* in);
	explicit matrix_h(const angles* in);
	explicit matrix_h(const vec3d *fvec, const vec3d *uvec = nullptr, const vec3d *rvec = nullptr);

	angles* GetAngles();

	matrix* GetMatrix();

	void SetStatus(MatrixState n_status);

	void serialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, const luacpp::LuaValue& value, ubyte* data, int& packet_size);
	void deserialize(lua_State* /*L*/, const scripting::ade_table_entry& /*tableEntry*/, char* data_ptr, ubyte* data, int& offset);
};

DECLARE_ADE_OBJ(l_Matrix, matrix_h);

}
}

#endif //FS2_OPEN_VECMATH_H
