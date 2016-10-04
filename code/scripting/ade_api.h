//
//

#ifndef FS2_OPEN_ADE_API_H_H
#define FS2_OPEN_ADE_API_H_H

#include "scripting/ade.h"

namespace scripting {

const size_t INVALID_ID = (size_t) -1; // Use -1 to get highest possible unsigned number

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
#define ADE_FUNC(name, parent, args, desc, ret_type, ret_desc)	\
	static int parent##_##name##_f(lua_State *L);	\
	::scripting::ade_func parent##_##name(#name, parent##_##name##_f, parent, args, desc, ret_type, ret_desc);	\
	static int parent##_##name##_f(lua_State *L)

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
#define ADE_VIRTVAR(name, parent, args, desc, ret_type, ret_desc)			\
	static int parent##_##name##_f(lua_State *L);	\
	::scripting::ade_virtvar parent##_##name(#name, parent##_##name##_f, parent, args, desc, ret_type, ret_desc);	\
	static int parent##_##name##_f(lua_State *L)

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
#define ADE_INDEXER(parent, args, desc, ret_type, ret_desc)			\
	static int parent##___indexer_f(lua_State *L);		\
	::scripting::ade_indexer parent##___indexer(parent##___indexer_f, parent, args, desc, ret_type, ret_desc);	\
	static int parent##___indexer_f(lua_State *L)

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
#define ADE_OBJ(field, type, name, desc) \
const ::scripting::ade_obj<type>& SCP_TOKEN_CONCAT(get_, field)() { \
	static ::scripting::ade_obj<type> obj(name, desc, nullptr);\
	return obj;\
} \
const ::scripting::ade_obj<type>& field = SCP_TOKEN_CONCAT(get_, field)();

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
#define DECLARE_ADE_OBJ(field, type) \
extern const ::scripting::ade_obj<type>& SCP_TOKEN_CONCAT(get_, field)(); \
extern const ::scripting::ade_obj<type>& field;

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
#define ADE_LIB(field, name, short_name, desc) \
const ::scripting::ade_lib& SCP_TOKEN_CONCAT(get_, field)() { \
	static ::scripting::ade_lib lib(name, nullptr, short_name, desc);\
	return lib;\
} \
const ::scripting::ade_lib& field = SCP_TOKEN_CONCAT(get_, field)();

/**
 * @brief Declare an API library but don't define it
 *
 * You should use this in headers if the library should be able to be used by other files
 *
 * @param field The name of the field, must match the name in the source file
 *
 * @ingroup ade_api
 */
#define DECLARE_ADE_LIB(field) \
extern const ::scripting::ade_lib& SCP_TOKEN_CONCAT(get_, field)(); \
extern const ::scripting::ade_lib& field; \
static const ::scripting::ade_lib* SCP_TOKEN_CONCAT(field, reference_dummy) USED_VARIABLE = &(field);


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

/**
 * @brief Return a value in error
 *
 * Should be used if the value is not valid
 *
 * @ingroup ade_api
 */
#define ade_set_error				ade_set_args


}

#endif //FS2_OPEN_ADE_API_H_H
