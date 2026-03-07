// Copyright 2015 The Shaderc Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SHADERC_SHADERC_H_
#define SHADERC_SHADERC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shaderc/env.h"
#include "shaderc/status.h"
#include "shaderc/visibility.h"

// Source language kind.
typedef enum {
  shaderc_source_language_glsl,
  shaderc_source_language_hlsl,
} shaderc_source_language;

typedef enum {
  shaderc_vertex_shader,
  shaderc_fragment_shader,
  shaderc_compute_shader,
  shaderc_geometry_shader,
  shaderc_tess_control_shader,
  shaderc_tess_evaluation_shader,

  shaderc_glsl_vertex_shader = shaderc_vertex_shader,
  shaderc_glsl_fragment_shader = shaderc_fragment_shader,
  shaderc_glsl_compute_shader = shaderc_compute_shader,
  shaderc_glsl_geometry_shader = shaderc_geometry_shader,
  shaderc_glsl_tess_control_shader = shaderc_tess_control_shader,
  shaderc_glsl_tess_evaluation_shader = shaderc_tess_evaluation_shader,

  shaderc_glsl_infer_from_source,
  shaderc_glsl_default_vertex_shader,
  shaderc_glsl_default_fragment_shader,
  shaderc_glsl_default_compute_shader,
  shaderc_glsl_default_geometry_shader,
  shaderc_glsl_default_tess_control_shader,
  shaderc_glsl_default_tess_evaluation_shader,
  shaderc_spirv_assembly,
  shaderc_raygen_shader,
  shaderc_anyhit_shader,
  shaderc_closesthit_shader,
  shaderc_miss_shader,
  shaderc_intersection_shader,
  shaderc_callable_shader,
  shaderc_glsl_raygen_shader = shaderc_raygen_shader,
  shaderc_glsl_anyhit_shader = shaderc_anyhit_shader,
  shaderc_glsl_closesthit_shader = shaderc_closesthit_shader,
  shaderc_glsl_miss_shader = shaderc_miss_shader,
  shaderc_glsl_intersection_shader = shaderc_intersection_shader,
  shaderc_glsl_callable_shader = shaderc_callable_shader,
  shaderc_glsl_default_raygen_shader,
  shaderc_glsl_default_anyhit_shader,
  shaderc_glsl_default_closesthit_shader,
  shaderc_glsl_default_miss_shader,
  shaderc_glsl_default_intersection_shader,
  shaderc_glsl_default_callable_shader,
  shaderc_task_shader,
  shaderc_mesh_shader,
  shaderc_glsl_task_shader = shaderc_task_shader,
  shaderc_glsl_mesh_shader = shaderc_mesh_shader,
  shaderc_glsl_default_task_shader,
  shaderc_glsl_default_mesh_shader,
} shaderc_shader_kind;

typedef enum {
  shaderc_profile_none,
  shaderc_profile_core,
  shaderc_profile_compatibility,
  shaderc_profile_es,
} shaderc_profile;

// Optimization level.
typedef enum {
  shaderc_optimization_level_zero,         // no optimization
  shaderc_optimization_level_size,         // optimize towards reducing code size
  shaderc_optimization_level_performance,  // optimize towards performance
} shaderc_optimization_level;

// An opaque handle to an object that manages all compiler state.
typedef struct shaderc_compiler* shaderc_compiler_t;

// Returns a shaderc_compiler_t that can be used to compile modules.
// A return of NULL indicates that there was an error initializing the compiler.
SHADERC_EXPORT shaderc_compiler_t shaderc_compiler_initialize(void);

// Releases the resources held by the shaderc_compiler_t.
SHADERC_EXPORT void shaderc_compiler_release(shaderc_compiler_t);

// An opaque handle to an object that manages options to a single compilation
// result.
typedef struct shaderc_compile_options* shaderc_compile_options_t;

// Returns a default-initialized shaderc_compile_options_t.
SHADERC_EXPORT shaderc_compile_options_t
    shaderc_compile_options_initialize(void);

// Releases the compilation options.
SHADERC_EXPORT void shaderc_compile_options_release(
    shaderc_compile_options_t options);

// Sets the compiler mode to generate debug information in the output.
SHADERC_EXPORT void shaderc_compile_options_set_generate_debug_info(
    shaderc_compile_options_t options);

// Sets the compiler optimization level.
SHADERC_EXPORT void shaderc_compile_options_set_optimization_level(
    shaderc_compile_options_t options, shaderc_optimization_level level);

// Sets the target shader environment.
SHADERC_EXPORT void shaderc_compile_options_set_target_env(
    shaderc_compile_options_t options,
    shaderc_target_env target,
    uint32_t version);

// An include result.
typedef struct shaderc_include_result {
  const char* source_name;
  size_t source_name_length;
  const char* content;
  size_t content_length;
  void* user_data;
} shaderc_include_result;

// The kinds of include requests.
enum shaderc_include_type {
  shaderc_include_type_relative,  // E.g. #include "source"
  shaderc_include_type_standard   // E.g. #include <source>
};

// An includer callback type for mapping an #include request to an include
// result.
typedef shaderc_include_result* (*shaderc_include_resolve_fn)(
    void* user_data, const char* requested_source, int type,
    const char* requesting_source, size_t include_depth);

// An includer callback type for destroying an include result.
typedef void (*shaderc_include_result_release_fn)(
    void* user_data, shaderc_include_result* include_result);

// Sets includer callback functions.
SHADERC_EXPORT void shaderc_compile_options_set_include_callbacks(
    shaderc_compile_options_t options, shaderc_include_resolve_fn resolver,
    shaderc_include_result_release_fn result_releaser, void* user_data);

// An opaque handle to the results of a call to any shaderc_compile_into_*()
// function.
typedef struct shaderc_compilation_result* shaderc_compilation_result_t;

// Compiles GLSL source to SPIR-V binary.
SHADERC_EXPORT shaderc_compilation_result_t shaderc_compile_into_spv(
    const shaderc_compiler_t compiler, const char* source_text,
    size_t source_text_size, shaderc_shader_kind shader_kind,
    const char* input_file_name, const char* entry_point_name,
    const shaderc_compile_options_t additional_options);

// Releases the resources held by the result object.
SHADERC_EXPORT void shaderc_result_release(shaderc_compilation_result_t result);

// Returns the number of bytes of the compilation output data.
SHADERC_EXPORT size_t shaderc_result_get_length(const shaderc_compilation_result_t result);

// Returns the number of warnings generated during the compilation.
SHADERC_EXPORT size_t shaderc_result_get_num_warnings(
    const shaderc_compilation_result_t result);

// Returns the compilation status.
SHADERC_EXPORT shaderc_compilation_status shaderc_result_get_compilation_status(
    const shaderc_compilation_result_t);

// Returns a pointer to the start of the compilation output data bytes.
// When compiled to SPIR-V binary, this is guaranteed to be castable to uint32_t*.
SHADERC_EXPORT const char* shaderc_result_get_bytes(const shaderc_compilation_result_t result);

// Returns a null-terminated string that contains any error messages.
SHADERC_EXPORT const char* shaderc_result_get_error_message(
    const shaderc_compilation_result_t result);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // SHADERC_SHADERC_H_
