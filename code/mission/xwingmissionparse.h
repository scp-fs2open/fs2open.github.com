#ifndef _XWI_PARSE_H
#define _XWI_PARSE_H

struct mission;
class XWingMission;

extern void parse_xwi_mission_info(mission *pm, XWingMission *xwim, bool basic = false);
extern void parse_xwi_mission(mission *pm, XWingMission *xwim);
extern void parse_xwi_briefing(mission* pm, XWingBriefing* xwBrief);

#endif
