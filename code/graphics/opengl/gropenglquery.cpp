
#include "graphics/opengl/gropenglquery.h"
#include "graphics/opengl/gropengl.h"

namespace {

struct query_object_slot {
	bool used = false;
	GLuint name = 0;
};

SCP_vector<query_object_slot> query_objects;

int get_new_query_slot() {
	auto end = query_objects.end();
	for (auto iter = query_objects.begin(); iter != end; ++iter) {
		if (!iter->used) {
			return (int) std::distance(query_objects.begin(), iter);
		}
	}

	query_objects.emplace_back();
	return (int) (query_objects.size() - 1);
}

query_object_slot& get_query_slot(int handle) {
	Assertion(handle >= 0 && handle < (int)query_objects.size(), "Query object index %d is invalid!", handle);
	return query_objects[handle];
}

}

int gr_opengl_create_query_object() {
	auto idx = get_new_query_slot();

	auto& slot = query_objects[idx];
	slot.used = true;

	glGenQueries(1, &slot.name);

	return idx;
}

void gr_opengl_query_value(int obj, QueryType type) {
	auto& slot = get_query_slot(obj);

	switch(type) {
		case QueryType::Timestamp:
			Assertion(GLAD_GL_ARB_timer_query, "Timestamp queries are not available! Availability must be checked before calling this function!");
			glQueryCounter(slot.name, GL_TIMESTAMP);
			break;
		default:
			UNREACHABLE("Unhandled enum value!");
			break;
	}
}

bool gr_opengl_query_value_available(int obj) {
	auto& slot = get_query_slot(obj);

	GLuint available;
	glGetQueryObjectuiv(slot.name, GL_QUERY_RESULT_AVAILABLE, &available);

	return available == GL_TRUE;
}

std::uint64_t gr_opengl_get_query_value(int obj) {
	auto& slot = get_query_slot(obj);

	GLuint64 available;
	glGetQueryObjectui64v(slot.name, GL_QUERY_RESULT, &available);

	return available;
}

void gr_opengl_delete_query_object(int obj) {
	auto& slot = get_query_slot(obj);
	glDeleteQueries(1, &slot.name);

	slot.name = 0;
	slot.used = false;
}
