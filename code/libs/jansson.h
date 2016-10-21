#pragma once

#include "cfile/cfile.h"

#include <jansson.h>

int json_dump_cfile(const json_t *json, CFILE* file, size_t flags);

json_t *json_load_cfile(CFILE* cfile, size_t flags, json_error_t *error);
