//
//

#ifndef FS2_OPEN_ADE_API_H_H
#define FS2_OPEN_ADE_API_H_H

#include "globalincs/version.h"

#include "scripting/ade.h"
#include "scripting/ade_args.h"
#include "scripting/ade_doc.h"

class lua_net_exception : public std::exception {
	using std::exception::exception;
};

namespace scripting {

const size_t INVALID_ID = (size_t) -1; // Use -1 to get highest possible unsigned number

/**
 * @ingroup ade_api
 */
class ade_lib_handle {
  protected:
	size_t LibIdx;

  public:
	ade_lib_handle() = default;

	size_t GetIdx() const { return LibIdx; }
};

namespace internal {
	template<typename T> void ade_multi_serialize_fundamental(lua_State* L, const scripting::ade_table_entry& tableEntry, const luacpp::LuaValue& value, ubyte* data, int& packet_size);

	template<typename T> void ade_multi_deserialize_fundamental(lua_State* L, const scripting::ade_table_entry& tableEntry, char* data_ptr, ubyte* data, int& offset);

	inline void ade_multi_serialize_unsupported(lua_State* L, const scripting::ade_table_entry& tableEntry, const luacpp::LuaValue& /*value*/, ubyte* /*data*/, int& /*packet_size*/) {
		LuaError(L, "Cannot serialize data of type %s for sending over network!", tableEntry.GetName());
		throw lua_net_exception("Cannot serialize data of given userdata type to network");
	}

	inline void ade_multi_deserialize_unsupported(lua_State* L, const scripting::ade_table_entry& tableEntry, char* /*data_ptr*/, ubyte* /*data*/, int& /*offset*/) {
		LuaError(L, "Cannot deserialize data of type %s from network! Make sure all players are running the same version!", tableEntry.GetName());
		throw lua_net_exception("Cannot deserialize data of recieved userdata type from network");
	}

	enum class ade_multi_serialize_mode : size_t { NATIVE, FUNDAMENTAL, UNSUPPORTED };

	template <typename T, typename = int>
	struct ade_serializable : std::false_type { };

	template <typename T>
	struct ade_serializable <T, decltype((void)T::serialize, 0)> : std::true_type { };

	template<typename T, ade_multi_serialize_mode mode>
	struct ade_multi_serialize_dispatcher {};

	template<typename T>
	struct ade_multi_serialize_dispatcher<T, ade_multi_serialize_mode::NATIVE> {
		static constexpr ade_serialize_func serialize = &T::serialize;
		static constexpr ade_deserialize_func deserialize = &T::deserialize;
	};

	template<typename T>
	struct ade_multi_serialize_dispatcher<T, ade_multi_serialize_mode::FUNDAMENTAL> {
		static constexpr ade_serialize_func serialize = &ade_multi_serialize_fundamental<T>;
		static constexpr ade_deserialize_func deserialize = &ade_multi_deserialize_fundamental<T>;
	};

	template<typename T>
	struct ade_multi_serialize_dispatcher<T, ade_multi_serialize_mode::UNSUPPORTED> {
		static constexpr ade_serialize_func serialize = &ade_multi_serialize_unsupported;
		static constexpr ade_deserialize_func deserialize = &ade_multi_deserialize_unsupported;
	};
}

template<typename T>
struct ade_multi_serializer : public internal::ade_multi_serialize_dispatcher<T,
	internal::ade_serializable<T>::value ? internal::ade_multi_serialize_mode::NATIVE :
	(std::is_fundamental<T>::value ? internal::ade_multi_serialize_mode::FUNDAMENTAL : internal::ade_multi_serialize_mode::UNSUPPORTED)> {};

/**
 * @ingroup ade_api
 */
template <class StoreType>
class ade_obj : public ade_lib_handle {
	static int lua_destructor(lua_State* L)
	{
		auto* obj =
		    static_cast<ade_obj<StoreType>*>(lua_touserdata(L, lua_upvalueindex(ADE_DESTRUCTOR_OBJ_UPVALUE_INDEX)));
		StoreType* value = nullptr;
		if (!ade_get_args(L, "o", obj->GetPtr(&value))) {
			return 0;
		}

		if (value == nullptr) {
			return 0;
		}

		value->~StoreType();
		return 0;
	}

  public:
	ade_obj(const char* in_name, const char* in_desc, const ade_lib_handle* in_deriv = nullptr, size_t size = 0, ade_serialize_func serializer = nullptr, ade_deserialize_func deserializer = nullptr)
	{
		ade_table_entry ate;

		// WMC - object metadata are uninstanced library types
		ate.Name = in_name;
		if (in_deriv != nullptr) {
			ate.DerivatorIdx = in_deriv->GetIdx();
		}
		ate.Type        = 'o';
		ate.Description = in_desc;
		ate.Size = size;
		ate.Serializer = serializer;
		ate.Deserializer = deserializer;

		if (!std::is_trivially_destructible<StoreType>::value) {
			// If this type is not trivial then we need to have a destructor
			// This is mildly dangerous since "this" will only remain valid if it was constructed statically. Since this
			// value is only used once at program startup it should be relatively safe
			ate.Destructor_upvalue = static_cast<void*>(this);
			ate.Destructor         = lua_destructor;
		}

		LibIdx = ade_manager::getInstance()->addTableEntry(ate);
	}

	// WMC - Use this to store object data for return, or for setting as a global
	ade_odata_setter<StoreType> Set(StoreType&& obj) const
	{
		return ade_odata_setter<StoreType>(LibIdx, std::move(obj));
	}
	ade_odata_setter<StoreType> Set(const StoreType& obj) const
	{
		return ade_odata_setter<StoreType>(LibIdx, obj);
	}

	// WMC - Use this to copy object data, for modification or whatever
	ade_odata_getter<StoreType> Get(StoreType* ptr) const { return ade_odata_getter<StoreType>(LibIdx, ptr); }

	// WMC - Use this to get a pointer to Lua object data.
	// Use >ONLY< when:
	// 1 - You are setting the data of an object (ie 'x' component of vector)
	// 2 - To speed up read-only calcs (ie computing dot product of vectors)
	// 3 - To get a reference to a move-only type stored in Lua memory
	ade_odata_ptr_getter<StoreType> GetPtr(StoreType** ptr) const
	{
		return ade_odata_ptr_getter<StoreType>(LibIdx, ptr);
	}
};

/**
 * @warning Utility macro. DO NOT USE!
 */
#define ADE_OBJ_DERIV_IMPL(field, type, name, desc, deriv, ...)                                                 \
	const ::scripting::ade_obj<type>& SCP_TOKEN_CONCAT(get_, field)()                                                  \
	{                                                                                                                  \
		static ::scripting::ade_obj<type> obj(name, desc, deriv, sizeof(type), __VA_ARGS__::serialize, __VA_ARGS__::deserialize);\
		return obj;                                                                                                    \
	}                                                                                                                  \
	const ::scripting::ade_obj<type>& field = SCP_TOKEN_CONCAT(get_, field)()

/**
 * @brief Define an API object
 *
 * An object is similar to a C++ class. Use this if you want to return a special type from a function that should be
 * able to do more on its own.
 *
 * @param field The name of the field by which the class should be accessible
 * @param type The type of the data the class contains
 * @param name The name the class should have in the documentation
 * @param desc Documentation about what this class is
 *
 * @ingroup ade_api
 */
#define ADE_OBJ(field, type, name, desc) ADE_OBJ_DERIV_IMPL(field, type, name, desc, nullptr, ade_multi_serializer<type>)

 /**
  * @brief Define an API object that cannot be serialized for multi, despite a serialization handler existing
  *
  * An object is similar to a C++ class. Use this if you want to return a special type from a function that should be
  * able to do more on its own.
  *
  * @param field The name of the field by which the class should be accessible
  * @param type The type of the data the class contains
  * @param name The name the class should have in the documentation
  * @param desc Documentation about what this class is
  *
  * @ingroup ade_api
  */
#define ADE_OBJ_NO_MULTI(field, type, name, desc) ADE_OBJ_DERIV_IMPL(field, type, name, desc, nullptr, internal::ade_multi_serialize_dispatcher<type, internal::ade_multi_serialize_mode::UNSUPPORTED>)


/**
 * @brief Define an API object that derives from another
 *
 * This is the same as ADE_OBJ but this allows to derive this class from another
 *
 * @param field The name of the field by which the class should be accessible
 * @param type The type of the data the class contains
 * @param name The name the class should have in the documentation
 * @param desc Documentation about what this class is
 * @param deriv The class to derive from. This should be the name of the class field (e.g. l_Object)
 *
 * @ingroup ade_api
 */
#define ADE_OBJ_DERIV(field, type, name, desc, deriv)                                                                  \
	ADE_OBJ_DERIV_IMPL(field, type, name, desc, &SCP_TOKEN_CONCAT(get_, deriv)(), ade_multi_serializer<type>)

/**
 * @brief Declare an API object but don't define it
 *
 * You should use this in headers if the class should be able to be used by other files
 *
 * @param field The name of the field
 * @param type The type of the contained value
 *
 * @ingroup ade_api
 */
#define DECLARE_ADE_OBJ(field, type)                                                                                   \
	extern const ::scripting::ade_obj<type>& SCP_TOKEN_CONCAT(get_, field)();                                          \
	extern const ::scripting::ade_obj<type>& field

/**
 * Library class
 * This is what you define a variable of to make new libraries
 *
 * @ingroup ade_api
 */
class ade_lib : public ade_lib_handle {
  public:
	explicit ade_lib(const char* in_name, const ade_lib_handle* parent = nullptr, const char* in_shortname = nullptr,
	                 const char* in_desc = nullptr);

	const char* GetName() const;
};
/**
 * @warning Utility macro. DO NOT USE!
 */
#define ADE_LIB_IMPL(field, name, short_name, desc, parent)                                                            \
	const ::scripting::ade_lib& SCP_TOKEN_CONCAT(get_, field)()                                                        \
	{                                                                                                                  \
		static ::scripting::ade_lib lib(name, parent, short_name, desc);                                               \
		return lib;                                                                                                    \
	}                                                                                                                  \
	const ::scripting::ade_lib& field = SCP_TOKEN_CONCAT(get_, field)()

/**
 * @brief Define an API library
 *
 * A library is similar to a C++ namespace or a class with static functions. It can be used to group multiple functions
 * together that serve a similar pupose
 *
 * @param field The name of the field by which the class should be ac
 * @param name The name the class should have in the documentationcessible
 * @param short_name The short name of the library, makes writing scripts easier
 * @param desc Documentation about what this class is
 *
 * @ingroup ade_api
 */
#define ADE_LIB(field, name, short_name, desc) ADE_LIB_IMPL(field, name, short_name, desc, nullptr)

/**
 * @brief Define an API library which is the child of another library
 *
 * A library is similar to a C++ namespace or a class with static functions. It can be used to group multiple functions
 * together that serve a similar pupose.
 *
 * A sublibrary is basically a nested namespace
 *
 * @param field The name of the field by which the class should be ac
 * @param name The name the class should have in the documentationcessible
 * @param short_name The short name of the library, makes writing scripts easier
 * @param desc Documentation about what this class is
 * @param parent The parent library, this should be the field name, e.g. l_Base
 *
 * @ingroup ade_api
 */
#define ADE_LIB_DERIV(field, name, short_name, desc, parent)                                                           \
	ADE_LIB_IMPL(field, name, short_name, desc, &SCP_TOKEN_CONCAT(get_, parent)())

/**
 * @brief Declare an API library but don't define it
 *
 * You should use this in headers if the library should be able to be used by other files
 *
 * @param field The name of the field, must match the name in the source file
 *
 * @ingroup ade_api
 */
#define DECLARE_ADE_LIB(field)                                                                                         \
	extern const ::scripting::ade_lib& SCP_TOKEN_CONCAT(get_, field)();                                                \
	extern const ::scripting::ade_lib& field;                                                                          \
	static const ::scripting::ade_lib* SCP_TOKEN_CONCAT(field, reference_dummy) USED_VARIABLE = &(field)

/**
 * @ingroup ade_api
 */
class ade_func : public ade_lib_handle {
  public:
	ade_func(const char* name,
		lua_CFunction func,
		const ade_lib_handle& parent,
		ade_overload_list args,
		const char* desc,
		const char* ret_type,
		const char* ret_desc,
		const gameversion::version& deprecation_version,
		const char* deprecation_message);
};

/**
 * @brief Declare an API function
 *
 * Immediately after this macro the function body should follow.
 *
 * @param name The name of the function, this may not be a string
 * @param parent The library or object containing this function
 * @param args Documentation for parameters of the function
 * @param desc Description of what the function does
 * @param ret_type The type of the returned value
 * @param ret_desc Documentation for the returned value
 *
 * @ingroup ade_api
 */
#define ADE_FUNC(name, parent, args, desc, ret_type, ret_desc)                                                         \
	static int parent##_##name##_f(lua_State* L);                                                                      \
	::scripting::ade_func parent##_##name(#name, parent##_##name##_f, parent, args, desc, ret_type, ret_desc,          \
	                                      ::gameversion::version(), nullptr);                                          \
	static int parent##_##name##_f(lua_State* L)

/**
 * @brief Declare a deprecated API function
 *
 * Immediately after this macro the function body should follow. This function is marked as deprecated and will be
 * handled specially if the targeted engine version is higher than the specified version.
 *
 * @param name The name of the function, this may not be a string
 * @param parent The library or object containing this function
 * @param args Documentation for parameters of the function
 * @param desc Description of what the function does
 * @param ret_type The type of the returned value
 * @param ret_desc Documentation for the returned value
 * @param deprecate_version Version starting from which this function is deprecated.
 * @param deprecated_msg Message for the deprecation notice. May be nullptr.
 *
 * @ingroup ade_api
 */
#define ADE_FUNC_DEPRECATED(name, parent, args, desc, ret_type, ret_desc, deprecate_version, deprecated_msg)           \
	static int parent##_##name##_f(lua_State* L);                                                                      \
	::scripting::ade_func parent##_##name(#name, parent##_##name##_f, parent, args, desc, ret_type, ret_desc,          \
	                                      deprecate_version, deprecated_msg);                                          \
	static int parent##_##name##_f(lua_State* L)

/**
 * @ingroup ade_api
 */
class ade_virtvar : public ade_lib_handle {
  public:
	ade_virtvar(const char* name,
		lua_CFunction func,
		const ade_lib_handle& parent,
		const char* args,
		const char* desc,
		const char* ret_type,
		const char* ret_desc,
		const gameversion::version& deprecation_version,
		const char* deprecation_message);
};

/**
 * @brief Declare an API variable
 *
 * Use this to handle forms of type vec.x and vec['x']. Basically an indexer for a specific variable. Format string
 * should be "o*%", where * is indexing value, and % is the value to set to when LUA_SETTTING_VAR is set
 *
 * @param name The name of the variable, this may not be a string
 * @param parent The library or object containing this field
 * @param args Documentation for the type of the value that may be assigned
 * @param desc Description of what the variable does
 * @param ret_type The type of the returned value
 * @param ret_desc Documentation for the returned value
 *
 * @ingroup ade_api
 */
#define ADE_VIRTVAR(name, parent, args, desc, ret_type, ret_desc)                                                      \
	static int parent##_##name##_f(lua_State* L);                                                                      \
	::scripting::ade_virtvar parent##_##name(#name, parent##_##name##_f, parent, args, desc, ret_type, ret_desc,       \
	                                         ::gameversion::version(), nullptr);                                       \
	static int parent##_##name##_f(lua_State* L)

/**
 * @brief Declare a deprecated API variable
 *
 * Use this to handle forms of type vec.x and vec['x']. Basically an indexer for a specific variable. Format string
 * should be "o*%", where * is indexing value, and % is the value to set to when LUA_SETTTING_VAR is set
 *
 * @param name The name of the variable, this may not be a string
 * @param parent The library or object containing this field
 * @param args Documentation for the type of the value that may be assigned
 * @param desc Description of what the variable does
 * @param ret_type The type of the returned value
 * @param ret_desc Documentation for the returned value
 * @param deprecate_version Version starting from which this function is deprecated.
 * @param deprecated_msg Message for the deprecation notice. May be nullptr.
 *
 * @ingroup ade_api
 */
#define ADE_VIRTVAR_DEPRECATED(name, parent, args, desc, ret_type, ret_desc, deprecate_version, deprecated_msg)        \
	static int parent##_##name##_f(lua_State* L);                                                                      \
	::scripting::ade_virtvar parent##_##name(#name, parent##_##name##_f, parent, args, desc, ret_type, ret_desc,       \
	                                         deprecate_version, deprecated_msg);                                       \
	static int parent##_##name##_f(lua_State* L)

/**
 * @ingroup ade_api
 */
class ade_indexer : public ade_lib_handle {
  public:
	ade_indexer(lua_CFunction func,
		const ade_lib_handle& parent,
		ade_overload_list overloads,
		const char* desc,
		const char* ret_type,
		const char* ret_desc);
};

/**
 * @brief Declare an indexer of an object
 *
 * Use this with objects to deal with forms such as vec.x, vec['x'], vec[0]. Format string should be "o*%", where * is
 * indexing value, and % is the value to set to when LUA_SETTTING_VAR is set
 *
 * @param parent The library or object containing the indexer
 * @param args Documentation for the type of the value that may be assigned
 * @param desc Description of what the variable does
 * @param ret_type The type of the returned value
 * @param ret_desc Documentation for the returned value
 *
 * @ingroup ade_api
 */
#define ADE_INDEXER(parent, args, desc, ret_type, ret_desc)                                                            \
	static int parent##___indexer_f(lua_State* L);                                                                     \
	::scripting::ade_indexer parent##___indexer(parent##___indexer_f, parent, args, desc, ret_type, ret_desc);         \
	static int parent##___indexer_f(lua_State* L)

//*************************Lua return values*************************
/**
 * @brief Return the Lua @c nil value
 *
 * @ingroup ade_api
 */
#define ADE_RETURN_NIL				0

/**
 * @brief Return @c true from an API function
 *
 * @ingroup ade_api
 */
#define ADE_RETURN_TRUE				ade_set_args(L, "b", true)

/**
 * @brief Return @c false from an API function
 *
 * @ingroup ade_api
 */
#define ADE_RETURN_FALSE			ade_set_args(L, "b", false)

}

#endif //FS2_OPEN_ADE_API_H_H
