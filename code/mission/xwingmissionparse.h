#ifndef _XWI_PARSE_H
#define _XWI_PARSE_H

struct mission;
class XWingMission;

extern void parse_xwi_mission_info(mission *pm, const XWingMission *xwim);
extern void parse_xwi_mission(mission *pm, const XWingMission *xwim);
extern void post_process_xwi_mission(mission *pm, const XWingMission *xwim);
extern void parse_xwi_briefing(mission *pm, const XWingBriefing *xwBrief);

#endif
