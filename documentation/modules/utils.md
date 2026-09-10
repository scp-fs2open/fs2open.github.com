# Module: utils — `code/utils/`

## Purpose
**General-purpose C++ utilities** with no game knowledge: typed id handles,
random-number ranges, Unicode-aware string handling, scope guards, a small event
type, a worker-thread pool, and the modular-curves template. Most of it is
header-only and lives in `namespace util`.

> This is *not* the same as `code/globalincs/`. `globalincs` holds the
> engine-wide foundations that are included everywhere (`pstypes.h`, the `SCP_*`
> container aliases, `MAX_*` limits, `flagset`). `utils` holds self-contained
> helpers you include only where you need them.

## Key files
- `id.h` — `util::ID<Tag, Impl, default_value>`: a **type-safe integer handle**.
  This is the pattern behind `gamesnd_id`, `interface_snd_id`,
  `sound_load_id`, `ParticleEffectHandle`, and others; two IDs with different
  tags will not compile if you mix them up. `util::ID_Hash` lets one be a map
  key.
- `RandomRange.h` — `util::RandomRange<Value, Distribution, Generator>` and the
  concrete aliases built on it: `UniformFloatRange` (from `UniformRange<T>`,
  alongside `UniformIntRange` and `UniformUIntRange`),
  `BoundedNormalFloatRange`, and `CurveFloatRange`. Plus
  `util::ParsedRandomFloatRange`, a `std::variant` over those three float
  ranges, which is what table code parses when a field may be "a number or a
  random range".
- `Random.h` / `Random.cpp` — the shared generator behind those ranges.
- `unicode.h` / `unicode.cpp` — `unicode::text_iterator` and
  `unicode::codepoint_range` for iterating a string by codepoint rather than
  by byte. Needed anywhere non-ASCII text is measured or drawn.
- `string_utils.*`, `strings.h`, `join_string.h`, `encoding.*` — string
  helpers and encoding conversion.
- `base64.*` — base64 encode/decode (see `code/pngutils/` for the main user).
- `finally.h` — `util::FinalAction`, a scope guard: run a lambda when the scope
  exits, whatever path it takes.
- `event.h` — `util::event<Ret, Args...>`, a small multicast callback list.
- `reset_on_move.h` — `util::reset_on_move<T, NullVal>`: a member that resets
  itself to `NullVal` when the owning object is moved from, which makes
  move-only resource handles safe without writing move constructors.
- `threading.*` — `namespace threading`: the worker-task pool
  (`init_task_pool()`, `spin_up_threaded_task()`, `spin_down_wait_complete()`,
  `is_threading()`). Use this rather than raw threads; the engine is otherwise
  single-threaded.
- `modular_curves.h` — the template machinery that lets a table field be driven
  by a `curves.tbl` curve (see `code/math/curve.*`); `code/particle/` is the
  main user.
- `HeapAllocator.*` — a pooled allocator.
- `table_viewer.*` — `table_viewer::get_table_entry_text()` and
  `get_complete_table_text()`: pull the original text of a table entry back out
  for display, used by the Lab and debug UI.
- `tuples.h`, `bitarray`-style helpers, and `boost/` — a small vendored subset
  (`hash_combine.h`, `syncboundedqueue.h`).

## Conventions
- Prefer `util::ID` over a bare `int` for any new handle type; it costs nothing
  at runtime and prevents mixing handle spaces.
- Prefer `util::ParsedRandomFloatRange` over a plain float when a table value
  should be allowed to vary.
- Anything under `boost/` is vendored — do not edit it.

## Configuration tables
None.

## See also
- `code/globalincs/` (the always-included foundations; `modules/globalincs.md`),
  `code/math/curve.*` (the curves `modular_curves.h` applies),
  `code/particle/`, `code/gamesnd/`, and `code/sound/` (the biggest `util::ID`
  and `RandomRange` users).
