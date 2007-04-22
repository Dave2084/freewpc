/*
 * Copyright 2006, 2007 by Brian Dominy <brian@oddchange.com>
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

#ifndef _SYS_SWITCH_H
#define _SYS_SWITCH_H

/** Small integer typedef for a switch number */
typedef U8 switchnum_t;

/** Switch flags */
#define SW_OPTICAL	0x01 /* Switch is active when it is _open_ */
#define SW_EDGE		0x02 /* Switch is handled on any edge */
#define SW_IN_GAME	0x08 /* Only service switch during a game */
#define SW_PLAYFIELD	0x10 /* Declares that switch is 'on the playfield' */
#define SW_NOPLAY		0x20 /* Switch does not mark a ball as 'in play' */
#define SW_IN_TEST	0x40 /* Service switch in test mode; default is no */

/** Switch handler prototype form */
typedef void (*switch_handler_t) (void);

/** A switch descriptor.  Contains all of the static information
 * about a particular switch */
typedef struct
{
	/** A function to call when the switch produces an event.
	 * All functions are assumed to be callsets, and located in the
	 * callset page of the ROM. */
	switch_handler_t fn;

	/** A set of flags that control when switch events are produced */
	U8 flags;

	/** If nonzero, indicates a lamp that is associated with the switch */
	U8 lamp;

	/** If nonzero, indicates a sound to be made on any switch event */
	sound_code_t sound;

	/** Indicates how long the switch must be physically active
	 * before an event is generated */
	task_ticks_t active_time;

	/** Indicates how long the switch must be physically inactive
	 * before the next switch event can be considered */
	task_ticks_t inactive_time;

	/** If nonzero, indicates the device driver associated with this
	 *switch. */
	U8 devno;
} switch_info_t;


extern const U8 mach_opto_mask[];
extern const U8 mach_edge_switches[];


#define SW_DEVICE_DECL(real_devno)	((real_devno) + 1)

#define SW_HAS_DEVICE(sw)	(sw->devno != 0)

#define SW_GET_DEVICE(sw)	(sw->devno - 1)


#define NUM_PF_SWITCHES 64
#define NUM_DEDICATED_SWITCHES 8
#define NUM_FLIPPER_SWITCHES 8

#define NUM_SWITCHES (NUM_PF_SWITCHES + NUM_DEDICATED_SWITCHES + NUM_FLIPPER_SWITCHES)

#define SWITCH_BITS_SIZE	(NUM_SWITCHES / 8)

#define SW_COL(x)			((x) >> 3)
#define SW_ROW(x)			((x) & 0x07)
#define SW_ROWMASK(x)	(1 << SW_ROW(x))

#define MAKE_SWITCH(col,row)	((col * 8) + row - 1)
#define MAKE_SW(row,col)  MAKE_SWITCH(row,col)

/* Array types. */
#define AR_RAW			0
#define AR_CHANGED 	1
#define AR_PENDING 	2
#define AR_QUEUED 	3
#define AR_RUNNING   4
#define NUM_SWITCH_ARRAYS 	5

extern U8 switch_bits[NUM_SWITCH_ARRAYS][SWITCH_BITS_SIZE];


extern inline U8 rt_switch_poll (const switchnum_t sw_num)
{
	return switch_bits[0][(sw_num / 8)] & (1 << (sw_num % 8));
}


/** Declare that another switch can follow the one that just closed. */
#define switch_can_follow(first,second,timeout) \
	timer_restart_free (GID_ ## first ## _FOLLOWED_BY_ ## second, timeout)

/** Declare that a 'device switch' can follow the one that just closed.
 * This is equivalent to 'switch_can_follow', but it also freezes all
 * timers temporarily, since the ball is effectively not in play and
 * is eventually to a device, where timers will stop anyway. */
#define device_switch_can_follow(first, second, timeout) \
	do { \
		switch_can_follow (first, second, timeout); \
		timer_restart_free (GID_DEVICE_SWITCH_WILL_FOLLOW, timeout); \
	} while (0)

/** Indicate that the second switch did indeed follow.
 * If this returns TRUE, it means it happened within the timeout after the
 * first switch closure.  This will always return FALSE during a
 * multiball, since multiple closures cannot assume that they are from
 * the same ball. */
#define switch_did_follow(first,second) \
	((live_balls == 1) \
		&& timer_kill_gid (GID_ ## first ## _FOLLOWED_BY_ ## second))

/* The above macros are generic enough to apply to any event, not
 * just switch events */

#define event_should_follow(f,s,t) switch_can_follow(f,s,t)
#define event_can_follow(f,s,t)	switch_can_follow(f,s,t)
#define event_did_follow(f,s)		switch_did_follow(f,s)

/** A macro for button event handlers, which need to be behave
differently when the button is kept pressed. */
#define callset_invoke_held(sw,delay,repeat,event) \
	do { \
		U8 i; \
		callset_invoke (event); \
		for (i=0; i < 8; i++) \
			if (!switch_poll (sw)) \
				goto done; \
			else \
				task_sleep (delay / 8); \
	 \
		while (switch_poll (sw)) \
		{ \
			callset_invoke (event); \
			task_sleep (repeat); \
		} \
	done:; \
	} while (0)


void switch_init (void);
void switch_rtt (void);
void switch_sched (void);
void switch_idle_task (void);
bool switch_poll (const switchnum_t sw);
bool switch_is_opto (const switchnum_t sw);
bool switch_poll_logical (const switchnum_t sw);
U8 switch_lookup_lamp (const switchnum_t sw);

#endif /* _SYS_SWITCH_H */
