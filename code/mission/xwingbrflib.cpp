#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <assert.h>
#include "xwingbrflib.h"



XWingBriefing::XWingBriefing()
{
}

XWingBriefing::~XWingBriefing()
{
}


XWingBriefing *XWingBriefing::load(const char *data)
{
	// parse header
	struct xwi_brf_header *h = (struct xwi_brf_header *)data;
	if (h->version != 2)
		return NULL;

	XWingBriefing *b = new XWingBriefing();

	// h->icon_count == numShips
	b->ships = *new std::vector<xwi_brf_ship>(h->icon_count);

	return b;
}

