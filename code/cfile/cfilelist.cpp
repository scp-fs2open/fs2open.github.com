/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <winbase.h>		/* needed for memory mapping of file functions */
#endif

#include "cfile/cfile.h"
#include "globalincs/pstypes.h"


void cf_sort_filenames( SCP_vector<SCP_string> &list, int sort, SCP_vector<file_list_info> *info )
{
	// NOTE: This really needs to be updated to C++ style sorting at some point

	int i, j, incr;
	SCP_string t;
	file_list_info tt;

	int n = (int)list.size();

	if (sort == CF_SORT_NAME) {
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if (stricmp(list[j].c_str(), list[j + incr].c_str()) > 0) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						if (info) {
							tt = (*info)[j];
							(*info)[j] = (*info)[j + incr];
							(*info)[j + incr] = tt;
						}

						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_TIME) {
		Assert(info);
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if ( (*info)[j].write_time < (*info)[j + incr].write_time ) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						tt = (*info)[j];
						(*info)[j] = (*info)[j + incr];
						(*info)[j + incr] = tt;
						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_REVERSE) {
		std::reverse( list.begin(), list.end() );

		if (info) {
			std::reverse( info->begin(), info->end() );
		}

		return;
	}

	nprintf(("Error", "Unknown sorting method %d passed to cf_sort_filenames()\n", sort));
}

// Sorts a list of filenames using the specified sorting method (CF_SORT_*).
//   n = number of filenames in list to sort
//   list = list of filenames to be sorted
//   sort = sorting method to use (one of the CF_SORT_* defines)
//   info = extra info for each file.  Only required if sorting by time, however if you
//          have extra file info, you should pass it as well to get it sorted too (so an
//          index into list is the same index for info for that file
void cf_sort_filenames( int n, char **list, int sort, file_list_info *info )
{
	int i, j, incr;
	char *t;
	file_list_info tt;

	if (sort == CF_SORT_NAME) {
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if (stricmp(list[j], list[j + incr]) > 0) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						if (info) {
							tt = info[j];
							info[j] = info[j + incr];
							info[j + incr] = tt;
						}

						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_TIME) {
		Assert(info);
		incr = n / 2;
		while (incr > 0) {
			for (i=incr; i<n; i++) {
				j = i - incr;
				while (j >= 0) {
					if (info[j].write_time < info[j + incr].write_time) {
						t = list[j];
						list[j] = list[j + incr];
						list[j + incr] = t;

						tt = info[j];
						info[j] = info[j + incr];
						info[j + incr] = tt;
						j -= incr;

					} else
						break;
				}
			}

			incr /= 2;
		}

		return;

	} else if (sort == CF_SORT_REVERSE) {
		incr = n / 2;
		char buffer[MAX_FILENAME_LEN];
		file_list_info tt_tmp;

		for (i = 0; i < incr; i++) {
			t = list[n - 1 - i];

			if (list[i] != t) {
				strcpy_s(buffer, list[i]);
				strcpy(list[i], t);
				strcpy(t, buffer);

				if (info) {
					tt = info[n - 1 - i];
					tt_tmp = info[i];
					info[i] = tt;
					tt = tt_tmp;
				}
			}
		}

		return;
	}

	nprintf(("Error", "Unknown sorting method %d passed to cf_sort_filenames()\n", sort));
}