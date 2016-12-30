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
	explicit matrix_h(matrix* in);
	explicit matrix_h(angles* in);

	angles* GetAngles();

	matrix* GetMatrix();

	void SetStatus(MatrixState n_status);
};

DECLARE_ADE_OBJ(l_Matrix, matrix_h);

}
}

#endif //FS2_OPEN_VECMATH_H
