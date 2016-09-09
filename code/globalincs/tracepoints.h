/*
 * Copyright (C) Freespace Open 2016.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include <SDL_platform.h>

#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER fs2open

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./globalincs/tracepoints.h"

#if !defined(_TRACEPOINTS_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINTS_H

#ifdef __LINUX__
#include <lttng/tracepoint.h>

/**
 * NAMING CONVENTION:
 *
 * 1 For entry/exits, please end the common name with "__begin" or "__end"
 * 2 for send receives, please ent the common name with "__send" or "__recv"
 */

TRACEPOINT_EVENT(
		fs2open,
		draw_list_render_all__begin,
		TP_ARGS(
				int, zbuffer_type
		),
		TP_FIELDS(
				ctf_integer(int, zbuffer_type, zbuffer_type)
		)
)

TRACEPOINT_EVENT(
		fs2open,
		draw_list_render_all__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		game_frame__begin,
		TP_ARGS(
				char, paused
		),
		TP_FIELDS(
				ctf_integer(char, paused, paused)
		)
)

TRACEPOINT_EVENT(
		fs2open,
		game_frame__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		game_render_frame__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		game_render_frame__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		game_simulation_frame__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		game_simulation_frame__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		gr_flip__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		gr_flip__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		mission_eval_goals__begin,
		TP_ARGS(
				int, objective
		),
		TP_FIELDS(
				ctf_integer(int, objective, objective)
		)
)

TRACEPOINT_EVENT(
		fs2open,
		mission_eval_goals__end,
		TP_ARGS(
				int, objective
		),
		TP_FIELDS(
				ctf_integer(int, objective, objective)
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_move_all__begin,
		TP_ARGS(
				float, arg
		),
		TP_FIELDS(
				ctf_float(int, sequence, arg)
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_move_all__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_move_all_pre__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_move_all_pre__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_render_queue_all__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_render_queue_all__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_sort_and_collide__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		obj_sort_and_collide__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		particle_render_all__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		particle_render_all__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		particle_render_set_up__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		particle_render_set_up__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		particle_render_batch_render__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		particle_render_batch_render__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		trail_render__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		LUA_On_Frame__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		LUA_On_Frame__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		draw_list__render_all__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		draw_list__render_all__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		sort_colliders_phase_1__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		sort_colliders_phase_1__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		sort_colliders_phase_2__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		sort_colliders_phase_2__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		sort_colliders_phase_3__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		sort_colliders_phase_3__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		trail_render_draw__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		trail_render_draw__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		queue_render__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		queue_render__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		page_flip__begin,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

TRACEPOINT_EVENT(
		fs2open,
		page_flip__end,
		TP_ARGS(
		),
		TP_FIELDS(
		)
)

#include <lttng/tracepoint-event.h>

#else

#define tracepoint( ... )

#endif /* LINUX */

#endif /* _tracepoints */
