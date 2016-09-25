
#include "tracing/scopes.h"

namespace tracing {

Scope::Scope(const char* name) : _name(name) {
}
Scope::~Scope() {
}

Scope MainFrameScope("main_frame");

}
