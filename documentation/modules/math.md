# Module: math — `code/math/`

## Purpose
The engine's **math library**: vector/matrix algebra, the fixed-point type,
line/ray/sphere/polygon intersection tests, deterministic ("static") random
numbers, splines, curves, and an inverse-kinematics solver. It has no game
state of its own — every other module calls into it.

## Key files
- `vecmat.cpp` / `vecmat.h` — the core: all `vm_*` vector, matrix, and angle
  operations plus the standard constants.
- `fvi.cpp` / `fvi.h` — "find vector intersection": the geometric intersection
  primitives that collision detection is built on.
- `fix.cpp` / `fix.h` — arithmetic for the fixed-point type `fix` (an
  `std::int32_t` in `pstypes.h`, 16.16 format: `F1_0` = 65536 is 1.0).
- `floating.cpp` / `floating.h` — float helpers (`fl_sqrt`, `frand_range`,
  `acosf_safe`/`asinf_safe`, `golden_ratio_rand`).
- `staticrand.cpp` / `staticrand.h` — seeded, repeatable random numbers keyed by
  an index, so the same object gets the same "random" value every frame and
  across the network.
- `curve.cpp` / `curve.h` — named, table-driven interpolation curves (`Curve`).
- `spline.cpp` / `spline.h` — `bez_spline` (Bezier) and `herm_spline` (Hermite).
- `ik_solver.cpp` / `ik_solver.h` — inverse kinematics for model animation.
- `bitarray.h` — small fixed bit-array helpers.

## Core data structures / globals
- `vec3d`, `matrix`, `matrix4`, `angles` — **defined in
  `code/globalincs/pstypes.h`**, not here; `vecmat.h` supplies the operations on
  them. Both headers are needed to work with them.
- Standard values (`vecmat.h`): `vmd_zero_vector`, `vmd_identity_matrix`,
  `vmd_zero_matrix`, `vmd_x_vector` / `_y_` / `_z_`, `vmd_zero_matrix4`.
- `Curve` + `SCP_vector<Curve> Curves` — parsed curves, looked up with
  `curve_get_by_name()`.
- `CurveInterpFunction`, `curve_keyframe` — how a curve interpolates between
  keyframes.

## Naming conventions
- `vm_*` — vector/matrix operations. A trailing `2` means "in place"
  (`vm_vec_add(dest, a, b)` vs. `vm_vec_add2(dest, src)`).
- `fvi_*` — intersection tests; most return a hit count/flag and write the hit
  point through an out-parameter.
- `static_rand*` — deterministic; pass a stable index (usually an object
  signature) as the seed so the result is reproducible on every machine.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `curves.tbl` (+ `*-crv.tbm`) | `parse_curve_table()` / `curves_init()` (`curve.cpp`) | Named interpolation curves usable from other tables |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/globalincs/pstypes.h` (the type definitions themselves),
  `code/physics/` and `code/object/objcollide.*` (the heaviest `fvi_*` users),
  `code/model/modelcollide.cpp` (ray-vs-BSP built on `fvi_*`).
