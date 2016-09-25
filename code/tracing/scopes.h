
#ifndef FS2_OPEN_SCOPES_H
#define FS2_OPEN_SCOPES_H
#pragma once

#include "globalincs/pstypes.h"


/** @file
 *  @ingroup tracing
 *
 *  This file contains the tracing scopes that exist. In order to add a new one you need to create the instance in scopes.cpp,
 *  declare the @c extern reference here and then use it wherever appropriate.
 */

namespace tracing {

/**
 * @brief A tracing scope
 *
 * A scope is similar to a category but should be used for events that take a longer time or that can happen asynchronously.
 */
class Scope {
	const char* _name;

 public:
	explicit Scope(const char* name);
	~Scope();

	const char* getName() const { return _name; }
};

extern Scope MainFrameScope;

}

#endif //FS2_OPEN_SCOPES_H
