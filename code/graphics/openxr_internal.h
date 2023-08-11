#pragma once

#include "openxr.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

extern XrInstance xr_instance;
extern XrSystemId xr_system;

PFN_xrVoidFunction openxr_getExtensionFunction(const char* const name);