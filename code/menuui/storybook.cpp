//This holds all the code for the WCS "Fiction Viewer"
//I like to call it...the storybook. :) -WMC

#include "gamesequence/gamesequence.h"
#include "lab/wmcgui.h"

static GUIScreen *SB_screen = NULL;

void storybook_command_briefing(Button *caller)
{
	gameseq_post_event(GS_EVENT_CMD_BRIEF);
}

void storybook_help(Button *caller)
{
	//do nothing yet
	//TODO: Do something
}

void storybook_options(Button *caller)
{
	gameseq_post_event(GS_EVENT_OPTIONS_MENU);
}

void storybook_init()
{
	if(SB_screen != NULL)
	{
		GUI_system->PushScreen(SB_screen);
		return;
	}

	SB_screen = GUI_system->PushScreen(new GUIScreen("Storybook"));

	SB_screen->Add(new Button("Continue", 50, 50, storybook_command_briefing));
	SB_screen->Add(new Button("Help", 50, 100, storybook_help));
	SB_screen->Add(new Button("Options", 50, 150, storybook_options));
	SB_screen->Add(new ImageAnim("Image", "boom", 90, 100));

	char* string = "Altair\n\n\tWith a final blast from the landing rockets, the modified Omega transport settled down into the earth. There was a moment of relative peace, then the airlock hummed, made a large clunk, and with the hiss of pressurized air escaping, it swung downward and landed on the ground. Twelve heavily armed young men ran out, quickly forming a perimeter around the open door. All of them scanned the surrounding trees for a moment, before saying anything.\n\n\t\"Clear!\" the Sergeant in charge reported. \"Marines, fan out! Team C, investigate that building!\" He jerked his head in the direction of the large grey-green tower some 250 meters away.\n\n\t\"Yes, sir,\" one of the men said over the comm system, as they and four of the twelve soldiers moved off towards the structure, their eyes alert and their fingers ready on the triggers of their guns.\n\n\tBack at the Omega, Captain Roemig and five professional-looking specialists emerged from the transport. None of them looked particularly at ease; whatever had destroyed the Triton and its escort fighters had still not been found. The vessels had been identified as part of a mining operation, and they'd confirmed the identity of the landing craft, Darwin's Folly. All nine members of the expedition - seven on board the Triton, and two mercs driving the Perseuses - were found dead, killed long before the Trinity had arrived.\n\n\tAs the four soldiers moved across the terrain separating the tower from the transport, the Head of Security for the Trinity scoffed. \"They'd better hope they're not moving into a minefield. With their proximity to one another, they'd all be killed by a single well-placed Golan M-47.\"";
	Window *cwp = (Window*)SB_screen->Add(new Window("Fiction viewer", 50, 50, -1, -1, WS_NOTITLEBAR | WS_NONMOVEABLE));
	cwp->AddChild(new Text("Content", string, 0, 0));
}

void storybook_do_frame(float frametime)
{
	GUI_system->OnFrame(frametime, true, true);
}

void storybook_close()
{
	GUI_system->PullScreen(SB_screen);
}