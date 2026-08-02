---
name: fso-add-sexp
description: >-
  Add a new SEXP operator (mission-scripting function) to FSO. Use when creating
  a new SEXP, adding a mission event/goal operator, registering an OP_ identifier,
  or wiring a sexp into eval_sexp. Covers sexp.h enum, the Operators table,
  argument typing, evaluation, and help text in code/parse/sexp.cpp.
---

# FSO: Add a SEXP Operator

SEXPs are the mission designer's scripting language. Adding one touches a fixed
set of locations in `code/parse/sexp.h` and `code/parse/sexp.cpp`. Search an
existing similar operator name across both files to find every spot to edit.

## Checklist

```
- [ ] 1. Declare OP_* identifier (sexp.h)
- [ ] 2. Register in the Operators table (sexp.cpp ~line 146)
- [ ] 3. Add argument type-checking in query_operator_argument_type() (sexp.cpp ~line 32126)
- [ ] 4. Implement the handler + dispatch in eval_sexp() (sexp.cpp ~line 28169)
- [ ] 5. Add help text to Sexp_help (sexp.cpp ~line 38004)
- [ ] 6. Add the FRED submenu category in get_category()/get_subcategory() (sexp.cpp ~line 36656)
- [ ] 7. (If multiplayer-relevant) handle packing in network/multi_sexp.cpp
```

## Steps

1. **OP_ identifier** — add to the operator enum in `code/parse/sexp.h`
   (the block starting `OP_PLUS = FIRST_OP, ...`). Do not reuse a value.

2. **Register the operator** in the `Operators` vector (`sexp.cpp`, ~line 146).
   Entry format is `{ text, OP_id, min_args, max_args, type }`:

```cpp
{ "my-operator", OP_MY_OPERATOR, 1, 2, SEXP_ACTION_OPERATOR, },
```
   The 5th field (`sexp_oper_type`, the `SEXP_*_OPERATOR` macros defined near
   `sexp.cpp` ~line 126) classifies the operator's *kind*
   (action/conditional/arithmetic/boolean/…) — it does **not** control the FRED
   submenu; that's step 6.

3. **Argument typing** — add a `case OP_MY_OPERATOR:` to the big switch in
   `query_operator_argument_type()` (`sexp.cpp`, ~line 32126), declaring what
   each argument slot expects using the `OPF_*` enums (`OPF_NUMBER`, `OPF_SHIP`, …).

4. **Implement + dispatch** — write a handler function, then add a `case OP_MY_OPERATOR:`
   in `eval_sexp()` (`sexp.cpp`, ~line 28169) that calls it and returns a SEXP
   result (`SEXP_TRUE`/`SEXP_FALSE`/`SEXP_KNOWN_*`, or a number for arithmetic ops).
   Read arguments via the `CDR`/`CADR` node walk like neighbouring cases.

5. **Help text** — add an entry to the `Sexp_help` vector (`sexp.cpp`, ~line 38004):

```cpp
{ OP_MY_OPERATOR, "my-operator\r\n"
    "\tWhat it does.\r\n\r\n"
    "Takes 1 or 2 arguments...\r\n"
    "\t1:\tFirst argument.\r\n"
    "\t2:\t(optional) Second argument." },
```

6. **FRED submenu category** — add a `case OP_MY_OPERATOR:` to `get_category()`
   (`sexp.cpp`, ~line 36656) so the operator appears under the right FRED
   category; add it to `get_subcategory()` (~line 37380) too if the category
   has subcategories.

7. **Multiplayer** — if the operator changes game state on the server, ensure it
   packs/sends correctly via `code/network/multi_sexp.*`.

## Conventions

- Operator names are lowercase-hyphenated and must be ≤ `OPERATOR_LENGTH` (30).
- Prefer adding self-contained logic; large new systems may live under `code/parse/sexp/`.
- Keep warning-clean (`FSO_FATAL_WARNINGS=ON`).

## Verify

- Build (see `fso-build-and-test`), open FRED/qtFRED, confirm the operator appears
  in the right category with correct arg constraints, and test it in a mission.

## Reference

- `code/parse/sexp.h` — `OP_*` enum, `OPF_*` arg types, `OP_CATEGORY_*`, return codes.
- `documentation/modules/parse.md`.
- SEXP list: https://wiki.hard-light.net/index.php/SEXP
