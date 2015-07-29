/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

/**
 * @file bm_examples.cpp
 * This file contains example code for using bmpman.
 */

#include "bmpman/bmpman.h"
#include "graphics/2d.h"

void bm_create_example() {
	static int test_inited = 0;
	static int test_bmp;
	static uint test_bmp_data[128 * 64];

	if (!test_inited) {
		test_inited = 1;
		// Create the new bitmap and fill in its data.
		// When you're done with it completely, call
		// bm_release to free up the bitmap handle
		test_bmp = bm_create(32, 128, 64, test_bmp_data);
		int i, j;
		for (i = 0; i < 64; i++) {
			for (j = 0; j < 64; j++) {
				uint r = i * 4;
				test_bmp_data[j + i * 128] = r;
			}
		}
	}

	// page out the data, so that the next bm_lock will convert the new data to
	// the correct bpp
	bm_unload(test_bmp);

	// put in new data
	int x, y;
	gr_reset_clip();
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 128; x++) {
			test_bmp_data[y * 128 + x] = 15;
		}
	}

	// Draw the bitmap, the upper left corner is the origin (0, 0)
	gr_set_bitmap(test_bmp);
	gr_bitmap(0, 0);
}
