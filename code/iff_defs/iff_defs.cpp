/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

/*
 * $Logfile: /Freespace2/code/iff_defs/iff_defs.cpp $
 * $Revision: 1.2 $
 * $Date: 2005-09-29 04:26:08 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/09/27 05:25:18  Goober5000
 * initial commit of basic IFF code
 * --Goober5000
 *
 */


#include "iff_defs/iff_defs.h"
#include "parse/parselo.h"
#include "cfile/cfile.h"


int Num_iffs;
iff_info Iff_info[MAX_IFFS];

int Iff_traitor;

//=============================================================================

// This is the default table
// Please note that the {\n\}s should be removed from the end of each line and
// the {\"}s  should be replaced with {"}s if you intend to use this to format
// your own iff_defs.tbl.

char *default_iff_table = "\
																			\n\
#IFFs																		\n\
																			\n\
;; Every iff_defs.tbl must contain a Traitor entry.  Traitors attack one	\n\
;; another (required by the dogfighting code) but it is up to you to		\n\
;; decide who attacks the traitor or whom else the traitor attacks.			\n\
$Traitor IFF: Traitor														\n\
																			\n\
;------------------------													\n\
; Friendly																	\n\
;------------------------													\n\
$IFF Name: Friendly															\n\
$IFF Color: ( 0, 255, 0 )													\n\
+As Seen By Hostile: ( 255, 0, 0 )											\n\
+As Seen By Neutral: ( 255, 0, 0 )											\n\
+As Seen By Traitor: ( 255, 0, 0 )											\n\
$Attacks: ( \"Hostile\" \"Neutral\" \"Traitor\" )							\n\
$Default Ship Flags: ( \"cargo-known\" )									\n\
																			\n\
;------------------------													\n\
; Hostile																	\n\
;------------------------													\n\
$IFF Name: Hostile															\n\
$IFF Color: ( 255, 0, 0 )													\n\
+As Seen By Hostile: ( 0, 255, 0 )											\n\
+As Seen By Neutral: ( 0, 255, 0 )											\n\
$Attacks: ( \"Friendly\" \"Neutral\" \"Traitor\" )							\n\
																			\n\
;------------------------													\n\
; Neutral																	\n\
;------------------------													\n\
$IFF Name: Neutral															\n\
$IFF Color: ( 255, 0, 0 )													\n\
+As Seen By Neutral: ( 0, 255, 0 )											\n\
$Attacks: ( \"Friendly\" \"Traitor\" )										\n\
																			\n\
;------------------------													\n\
; Unknown																	\n\
;------------------------													\n\
$IFF Name: Unknown															\n\
$IFF Color: ( 255, 0, 255 )													\n\
$Attacks: ( \"Hostile\" )													\n\
$Exempt From All Teams At War												\n\
																			\n\
;------------------------													\n\
; Traitor																	\n\
;------------------------													\n\
$IFF Name: Traitor															\n\
$IFF Color: ( 255, 0, 0 )													\n\
$Attacks: ( \"Friendly\" \"Hostile\" \"Neutral\" \"Traitor\" )				\n\
																			\n\
#End																		\n\
";

//=============================================================================


void iff_init()
{
	char traitor_name[NAME_LENGTH];

	// Goober5000 - condensed check for table file
	CFILE *idt = cfopen("iff_defs.tbl", "rb");
	int table_exists = (idt != NULL);
	if (table_exists)
		cfclose(idt);

	// Goober5000 - if table doesn't exist, use the default table (see above)
	if (table_exists)
		read_file_text("iff_defs.tbl");
	else
		read_file_text_from_array(default_iff_table);

	reset_parse();	

	required_string("#IFFs");

	// get the traitor
	required_string("$Traitor IFF:");
	stuff_string(traitor_name, F_NAME, NULL, NAME_LENGTH);

	// begin reading data
	Num_iffs = 0;
	while (required_string_either("#End","$IFF Name:"))
	{
		iff_info *iff;
		
		// make sure we're under the limit
		if (Num_iffs >= MAX_IFFS)
		{
			Warning(LOCATION, "Too many iffs in iffs_defs.tbl!  Max is %d.\n", MAX_IFFS);
			skip_to_start_of_string("#End", NULL);
			break;
		}

		iff = &Iff_info[Num_iffs];
		Num_iffs++;

		// get the iff name
		required_string("$IFF Name:");
		stuff_string(iff->iff_name, F_NAME, NULL, NAME_LENGTH);


		skip_to_start_of_string("$IFF Name", "#End");

// if (optional_string("$FRED Color:") || optional_string("$FRED Colour:"))


	}
	
	required_string("#End");
}
