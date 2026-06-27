# Module: physics — `code/physics/`

## Purpose
The **flight/movement model**. Integrates velocity, acceleration, rotation,
damping, afterburner, gliding and Newtonian behaviour for every object. A
`physics_info` struct is embedded in every `object`; this module advances it each
frame. Also provides physics *snapshots* used by multiplayer interpolation/rollback.

## Key files
- `physics.h` / `physics.cpp` — `physics_info`, `physics_sim()`, viewer handling.
- `physics_state.h` / `physics_state.cpp` — `physics_snapshot` capture/restore.

## Core data structures / globals
- `struct physics_info` — velocity, rotvel, mass, forces, damping, flags.
- `physics_snapshot` — serializable state for networking/interpolation.
- Object integration entry: `obj_move_call_physics()` (in `code/object/object.cpp`).

## Major constants
- Viewer directions: `PHYSICS_VIEWER_FRONT`, `PHYSICS_VIEWER_REAR`,
  `PHYSICS_VIEWER_UP`, `PHYSICS_VIEWER_LEFT`, `PHYSICS_VIEWER_RIGHT`.
- Physics behaviour flags `PF_*` (accelerating, gliding, afterburner, etc.).

## Configuration tables
None. Per-ship physics values (mass, thrust, max velocities, rotation time,
glide settings) are defined in `ships.tbl` and applied by the ship module.

## Architecture diagram (per-object integration)

```mermaid
flowchart TD
    inputs["inputs: pi->desired_vel, desired_rotvel<br/>(from AI control_info / player), gravity, damping"] --> call
    call["obj_move_call_physics(objp, frametime)<br/>(code/object)"] --> sim["physics_sim(pos, orient, pi, frametime)"]
    sim --> vel["physics_sim_vel()<br/>translational (physics.cpp ~274)"]
    sim --> rot["physics_sim_rot()<br/>rotational (physics.cpp ~151)"]
    vel --> apply["apply_physics()<br/>damped, frame-rate-independent (physics.cpp ~108)"]
    rot --> apply
    apply --> outp["new object pos / orient / velocities"]
    outp -.->|capture for networking| snap["physics_populate_snapshot()"]
    snap -.-> restore["physics_apply_pstate_to_object()<br/>(multiplayer interpolation/rollback)"]
```

## See also
- `code/object/object.h` (`physics_populate_snapshot`, `physics_apply_pstate_to_object`).
- `code/math/` (vectors/matrices), `code/network/multi_interpolate.*`.
- Table option reference: https://wiki.hard-light.net/index.php/Tables
