#pragma once

// swresample typedefs and macros

// typedefs
typedef AVAudioResampleContext SwrContext;

// Function compatibility
#define swr_alloc avresample_alloc_context
#define swr_init avresample_open
#define swr_free avresample_free
#define swr_convert_frame avresample_convert_frame
