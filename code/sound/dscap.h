/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __DSCAP_H__
#define __DSCAP_H__

int	dscap_init();
void	dscap_close();
int	dscap_supported();
int	dscap_create_buffer(int freq, int bits_per_sample, int nchannels, int nseconds);
void	dscap_release_buffer();

int	dscap_start_record();
int	dscap_stop_record();
int	dscap_max_buffersize();
int	dscap_get_raw_data(unsigned char *outbuf, unsigned int max_size);


#endif	// __DSCAP_H__
