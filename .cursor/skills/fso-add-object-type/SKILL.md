---
name: fso-add-object-type
description: >-
  Add a new in-world object type to the FSO object system (alongside ships,
  weapons, debris, asteroids, etc.). Use when introducing a new OBJ_ type, a new
  kind of simulated entity, or wiring a new entity into obj_create/obj_move_all
  and the collision system.
---

# FSO: Add a New Object Type

Every world entity is an `object` (`code/object/object.h`) whose `type` is an
`OBJ_*` constant and whose `instance` indexes a type-specific array. Adding a new
type means defining that array + the create/move/delete trio and registering it
with the object system.

## Checklist

```
- [ ] 1. Define OBJ_MY_TYPE + bump MAX_OBJECT_TYPES (object.h)
- [ ] 2. Add the name to Object_type_names[] (object.cpp)
- [ ] 3. Create the instance storage + *_info if data-driven (info-vs-instance)
- [ ] 4. Implement my_create() (wraps obj_create), my_move(), my_delete()
- [ ] 5. Hook my_move into obj_move_all_pre/post dispatch
- [ ] 6. Hook deletion into obj_delete_all_that_should_be_dead path
- [ ] 7. Add collision handling if it collides (object/objcollide + collide*.cpp)
- [ ] 8. Add rendering in the object render dispatch
```

## Steps

1. **Type constant** — add `#define OBJ_MY_TYPE NN` in `object.h` and increase
   `MAX_OBJECT_TYPES`. Add the matching string to `Object_type_names[]` in
   `object.cpp` (order must match).

2. **Storage** — follow the **info-vs-instance** pattern used by ships/weapons:
   a `my_type_info` class table (parsed from a `.tbl`, see `fso-add-table-field`)
   if the type is data-driven, plus a `my_type` instance array or `SCP_vector`.

3. **Create** — write `int my_create(...)` that calls
   `obj_create(OBJ_MY_TYPE, parent, instance, &orient, &pos, radius, flags)` and
   initializes the instance. Set `Collides` flag only if it participates in collisions.

4. **Move** — write `my_move(object *objp, float frametime)` and call it from the
   per-type dispatch inside `obj_move_all_pre()` / `obj_move_all_post()`
   (`object.cpp`). Physics runs via `obj_move_call_physics()` if `phys_info` is used.

5. **Delete** — to destroy, set the `Should_be_dead` flag. Add a `case OBJ_MY_TYPE:`
   in `obj_delete_all_that_should_be_dead()` (or the delete dispatch) to free
   instance data, then `obj_delete()`.

6. **Collisions** (optional) — register pairs in `code/object/objcollide.cpp` and
   add a `collide_my_type_*` handler (model the existing `collide*.cpp` files).

7. **Rendering** — add a `case OBJ_MY_TYPE:` to the object render dispatch
   (`obj_render` / `obj_queue_render` in `object.cpp`) that queues your draw call.

## Conventions

- Use `object_h`/signatures for cross-frame references, not raw objnums.
- Change flags via `obj_set_flags()` so collision-pair state stays consistent.
- Respect `MAX_OBJECTS` (must stay < 2^16-1 for collision-pair caching).
- Keep warning-clean (`FSO_FATAL_WARNINGS=ON`).

## Verify

- Build (see `fso-build-and-test`), spawn the entity (via code, SEXP, or Lab),
  confirm it moves, renders, collides, and cleans up without asserts.

## Reference

- `code/object/object.h` / `object.cpp`, `documentation/modules/object.md`.
