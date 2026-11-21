# Module: localization — `code/localization/`

## Purpose
**Translation and localized text.** Manages the active language, loads translated
string tables (`strings.tbl`/`tstrings.tbl`), and resolves `XSTR("...", id)`
lookups used throughout the UI and game text. Also handles language
auto-detection and the special-character font offsets per language.

## Key files
- `localize.cpp` / `localize.h` — language state, table parsing, `XSTR` resolution.
- `fhash.cpp` / `fhash.h` — string hashing used for fast lookup.

## Core data structures / globals
- `lang_info` + `Lcl_languages` (`SCP_vector`) and `Lcl_builtin_languages[]` —
  the supported-language table (name, on-disk extension, special chars, checksum).
- `int Lcl_current_lang` — index of the active language.
- `Lcl_special_chars`, `Lcl_en/fr/gr/pl` — font offsets / language flags.

## Major constants
- Language ids: `LCL_ENGLISH` (0), `LCL_GERMAN` (1), `LCL_FRENCH` (2),
  `LCL_POLISH` (3); `NUM_BUILTIN_LANGUAGES` (4).
- `LCL_DEFAULT` (0), `LCL_UNTRANSLATED` (100), `LCL_RETAIL_HYBRID` (101).
- `LCL_LANG_NAME_LEN` (32), `LCL_MIN_FONTS` (3).

## Translation lookup pattern
- Engine code wraps translatable literals in `XSTR("English text", id)`; at runtime
  the id is looked up in the active language's string table, falling back to the
  literal when untranslated.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `strings.tbl` | `parse_stringstbl()` | Built-in (engine) `XSTR` translations |
| `tstrings.tbl` | `parse_tstringstbl()` | Mod/mission (`+Tstrings`) translations |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/graphics/software/font.cpp` (`fonts.tbl`, special-character fonts),
  `code/parse/parselo.*` (table parsing), most UI modules (consumers of `XSTR`).
