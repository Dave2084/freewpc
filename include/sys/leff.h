/*
 * Copyright 2006 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _SYS_LEFF_H
#define _SYS_LEFF_H

typedef U8 leffnum_t;

typedef void (*leff_function_t) (void);

#define L_NORMAL	0x0

/** A running leff is long-lived and continues to be active
 * until it is explicitly stopped. */
#define L_RUNNING 0x1

/** A shared leff is lower priority and can only update a
 * subset of the lamps.  Multiple shared leffs can be running
 * concurrently.  If a shared leff can't get all of the lamps
 * that it wants, it doesn't run, otherwise it will.
 */
#define L_SHARED 0x2


/** A lamp effect descriptor */
typedef struct
{
	/** Miscellaneous flags */
	U8 flags;

	/** Its priority (higher value means more important) */
	U8 prio;

	/** The maximum set of lamps that it will want to control,
	 * expressed as a lampset. */
	U8 lampset;

	/** The maximum GI strings that it will want to control,
	 * expressed as a bitmask of GI string values. */
	U8 gi;

	/** The function implementing the leff */
	leff_function_t fn;
} leff_t;

#define MAX_QUEUED_LEFFS 8

/** Per-leff state variables, stored in the task thread data area */
#define L_PRIV_APPLY_DELAY 0
#define L_PRIV_DATA 1
#define L_PRIV_FLAGS 2
#define L_PRIV_ID 3

#define lampset_apply_delay	task_get_thread_data (task_getpid (), L_PRIV_APPLY_DELAY)
#define lampset_private_data	task_get_thread_data (task_getpid (), L_PRIV_DATA)
#define leff_running_flags		task_get_thread_data (task_getpid (), L_PRIV_FLAGS)
#define leff_self_id				task_get_thread_data (task_getpid (), L_PRIV_ID)

leffnum_t leff_get_active (void);
void leff_start (leffnum_t dn);
void leff_stop (leffnum_t dn);
void leff_restart (leffnum_t dn);
void leff_start_highest_priority (void);
__noreturn__ void leff_exit (void);
void leff_init (void);
void leff_stop_all (void);

#endif /* _SYS_LEFF_H */

