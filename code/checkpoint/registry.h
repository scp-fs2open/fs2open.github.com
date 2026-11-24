#pragma once

// Registration of mission state for checkpoints.
//
// Globals holding mission state are registered next to their definition:
//
//     int Mission_end_time;
//     CHECKPOINT_GLOBAL(Mission_end_time);
//
// They are saved with the generic codec (see checkpoint/codec.h) and restored after the mission has been
// reloaded, so everything that is not registered starts from its mission-start value.  Globals that need more than
// that register save and load functions with CHECKPOINT_GLOBAL_FUNCTIONS, and work that must happen once all globals
// are loaded is registered with CHECKPOINT_AFTER_GLOBALS, and work that needs the restored objects with
// CHECKPOINT_AFTER_RESTORE.
//
// Object types register how their instance data is saved and how runtime resources are re-acquired after loading,
// with CHECKPOINT_OBJECT_TYPE.  Object types that are not registered are dropped by a checkpoint.
//
// Any type can be registered: scalars, containers and key types are handled here, and scanned structs, enums and
// types with a rule (see checkpoint/rules.h) are compiled once, in checkpoint/checkpoint_fields.cpp.  A type without a
// rule is reported when linking, as an undefined reference to checkpoint::save_external<T>.

#include "checkpoint/codec.h"

#include <functional>
#include <memory>
#include <optional>

class object;

namespace checkpoint {

struct GlobalHandler {
	const char* name;
	std::function<void(Writer&)> save;
	std::function<void(Reader&)> load;
};

struct ObjectTypeHandler {
	int type;
	// saves the instance data of an object of this type
	void (*save)(Writer& w, const object& objp);
	// loads the instance data of an object whose object struct was just restored, and re-acquires its resources
	void (*restore)(Reader& r, object& objp);
	// runs once after all objects were restored and pointers between them resolved; may be null
	void (*after_restore)(object& objp);
	// whether an object of this type is saved at all; null means always
	bool (*is_saved)(const object& objp);
	// the number of instances of this type there can be, for checking saved data; 0 if not limited
	int max_instances = 0;
};

void register_global(GlobalHandler handler);
void register_after_globals(const char* name, std::function<void()> function);
void register_after_restore(const char* name, std::function<void()> function);
void register_object_type(ObjectTypeHandler handler);

template <typename T>
struct GlobalRegistration {
	GlobalRegistration(const char* name, T& variable)
	{
		register_global({name,
			[&variable](Writer& w) { save(w, variable); },
			[&variable](Reader& r) { load(r, variable); }});
	}
};

struct FunctionRegistration {
	FunctionRegistration(const char* name, std::function<void(Writer&)> save, std::function<void(Reader&)> load)
	{
		register_global({name, std::move(save), std::move(load)});
	}
};

struct AfterGlobalsRegistration {
	AfterGlobalsRegistration(const char* name, std::function<void()> function)
	{
		register_after_globals(name, std::move(function));
	}
};

struct AfterRestoreRegistration {
	AfterRestoreRegistration(const char* name, std::function<void()> function)
	{
		register_after_restore(name, std::move(function));
	}
};

struct ObjectTypeRegistration {
	explicit ObjectTypeRegistration(ObjectTypeHandler handler) { register_object_type(handler); }
};

// When state registered with CHECKPOINT_STATE is applied
enum class Phase {
	AfterGlobals,	// once all globals are restored
	AfterRestore,	// once the objects are restored too
};

template <typename T>
struct StateRegistration {
	StateRegistration(const char* name, T (*capture)(), void (*apply)(const T&), Phase phase)
	{
		auto loaded = std::make_shared<std::optional<T>>();
		register_global({name,
			[capture](Writer& w) { save(w, capture()); },
			[loaded](Reader& r) {
				T value{};
				load(r, value);
				if (!r.failed())
					*loaded = std::move(value);
			}});
		auto run = [loaded, apply]() {
			if (*loaded) {
				apply(**loaded);
				loaded->reset();
			}
		};
		if (phase == Phase::AfterGlobals)
			register_after_globals(name, run);
		else
			register_after_restore(name, run);
	}
};

// A view of the first entries of an array, for tables kept in fixed arrays with a count
template <typename T>
struct ArrayRange {
	T* first;
	T* last;
	T* begin() const { return first; }
	T* end() const { return last; }
};

template <typename T>
ArrayRange<T> first_n(T* array, int count)
{
	return {array, array + count};
}

struct TableFieldRegistration {
	// visit(f) calls f with the field of each table entry, in order
	template <typename Visit>
	TableFieldRegistration(const char* name, Visit visit)
	{
		register_global({name,
			[visit](Writer& w) {
				uint32_t count = 0;
				visit([&count](auto&) { ++count; });
				w.u32(count);
				visit([&w](auto& field) {
					auto at = w.begin_block();
					save(w, field);
					w.end_block(at);
				});
			},
			[visit](Reader& r) {
				auto count = r.u32();
				uint32_t index = 0;
				visit([&](auto& field) {
					if (index < count && !r.failed()) {
						auto block = r.block();
						load(block, field);
						++index;
					}
				});
				// entries the table no longer has
				for (; index < count && !r.failed(); ++index)
					r.block();
			}});
	}
};

} // namespace checkpoint

#define CHECKPOINT_CONCAT_INNER(a, b) a##b
#define CHECKPOINT_CONCAT(a, b) CHECKPOINT_CONCAT_INNER(a, b)

// Saves and restores a global with the generic codec.
#define CHECKPOINT_GLOBAL(variable)                                                                                    \
	static ::checkpoint::GlobalRegistration<std::remove_reference_t<decltype(variable)>> CHECKPOINT_CONCAT(              \
		checkpoint_global_, __LINE__)(#variable, variable)

// Saves and restores state with dedicated functions: void save(checkpoint::Writer&), void load(checkpoint::Reader&).
#define CHECKPOINT_GLOBAL_FUNCTIONS(save_function, load_function)                                                      \
	static ::checkpoint::FunctionRegistration CHECKPOINT_CONCAT(checkpoint_functions_, __LINE__)(#save_function,       \
		save_function, load_function)

// Runs a function after all globals have been restored, e.g. to rebuild lookup tables from them.
#define CHECKPOINT_AFTER_GLOBALS(function)                                                                             \
	static ::checkpoint::AfterGlobalsRegistration CHECKPOINT_CONCAT(checkpoint_after_globals_, __LINE__)(#function, function)

// Runs a function once everything is restored, e.g. to rebuild state derived from objects.
#define CHECKPOINT_AFTER_RESTORE(function)                                                                             \
	static ::checkpoint::AfterRestoreRegistration CHECKPOINT_CONCAT(checkpoint_after_restore_, __LINE__)(#function, function)

// Saves state that is applied at a later point of the restore than the globals are loaded, e.g. because applying it
// needs the restored objects.  capture is T(), apply is void(const T&), phase is AfterGlobals or AfterRestore.  If a
// checkpoint has no saved state of this name, apply isn't called.
#define CHECKPOINT_STATE(capture, apply, phase)                                                                        \
	static ::checkpoint::StateRegistration<decltype(capture())> CHECKPOINT_CONCAT(checkpoint_state_, __LINE__)(       \
		#capture, capture, apply, ::checkpoint::Phase::phase)

// Saves a field of every entry of a table, for fields missions change at runtime: tables aren't reset when a mission
// loads.  The entries are saved in order, e.g. CHECKPOINT_TABLE_FIELD(Weapon_info, damage_type_idx).  Tables in fixed
// arrays are given as checkpoint::first_n(array, count).
#define CHECKPOINT_TABLE_FIELD(table, member)                                                                          \
	static ::checkpoint::TableFieldRegistration CHECKPOINT_CONCAT(checkpoint_table_field_, __LINE__)(                 \
		#table "." #member, [](auto&& visit) {                                                                                       \
			for (auto& entry : table)                                                                                  \
				visit(entry.member);                                                                                   \
		})

#define CHECKPOINT_OBJECT_TYPE(...)                                                                                    \
	static ::checkpoint::ObjectTypeRegistration CHECKPOINT_CONCAT(checkpoint_object_type_, __LINE__)(                    \
		::checkpoint::ObjectTypeHandler{__VA_ARGS__})
