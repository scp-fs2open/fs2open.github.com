//
//

#include <globalincs/pstypes.h>
#include <iff_defs/iff_defs.h>
#include <model/model.h>

// Goober5000
void stuff_special_arrival_anchor_name(char *buf, int iff_index, int restrict_to_players, int retail_format)
{
	const char *iff_name = Iff_info[iff_index].iff_name;

	// stupid retail hack
	if (retail_format && !stricmp(iff_name, "hostile") && !restrict_to_players)
		iff_name = "enemy";

	if (restrict_to_players)
		sprintf(buf, "<any %s player>", iff_name);
	else
		sprintf(buf, "<any %s>", iff_name);

	strlwr(buf);
}
