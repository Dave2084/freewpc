
#include <freewpc.h>

#define MACHINE_DISPLAY_EFFECTS \
	DECL_DEFF (DEFF_TEST_MENU, D_RUNNING, 5, test_menu_deff) \
	DECL_DEFF (DEFF_AMODE, D_RUNNING, 10, amode_deff) \
	DECL_DEFF (DEFF_BRIAN_IMAGE, D_NORMAL, 20, egg_brian_image_deff) \
	DECL_DEFF (DEFF_SCORES, D_RUNNING, 30, scores_deff) \
	DECL_DEFF (DEFF_COIN_INSERT, D_NORMAL, 110, coin_deff) \
	DECL_DEFF (DEFF_CREDITS, D_NORMAL, 120, credits_deff) \
	DECL_DEFF (DEFF_BONUS, D_RUNNING, 150, bonus_deff) \
	DECL_DEFF (DEFF_TILT_WARNING, D_NORMAL, 200, tilt_warning_deff) \
	DECL_DEFF (DEFF_REPLAY, D_NORMAL, 202, replay_deff) \
	DECL_DEFF (DEFF_TILT, D_RUNNING, 205, tilt_deff) \
	DECL_DEFF (DEFF_SLAM_TILT, D_NORMAL, 210, tilt_deff) \
	DECL_DEFF (DEFF_PRINT_RTC, D_NORMAL, 250, rtc_print_deff) \
	DECL_DEFF (DEFF_FONT_TEST, D_RUNNING, 250, font_test_deff) \
	DECL_DEFF (DEFF_VOLUME_CHANGE, D_RUNNING, 250, volume_deff) \


/* Declare externs for all of the deff functions */
#define DECL_DEFF(num, flags, pri, fn) \
	extern void fn (void);

MACHINE_DISPLAY_EFFECTS


/* Now declare the deff table itself */
#undef DECL_DEFF
#define DECL_DEFF(num, flags, pri, fn) \
	[num] = { flags, pri, fn },

static const deff_t deff_table[] = {
	[DEFF_NULL] = { D_NORMAL, 0, NULL },
	MACHINE_DISPLAY_EFFECTS
};


/** Indicates the id of the deff currently active */
static uint8_t deff_active;

/** Indicates the priority of the deff currently running */
uint8_t deff_prio;

/** Queue of all deffs that have been scheduled to run, even
 * if they are low in priority.  The actively running deff
 * is also in this queue. */
static uint8_t deff_queue[MAX_QUEUED_DEFFS];


uint8_t deff_get_active (void)
{
	return deff_active;
}

static void deff_add_queue (deffnum_t dn)
{
	uint8_t i;

	for (i=0; i < MAX_QUEUED_DEFFS; i++)
		if (deff_queue[i] == dn)
			return;

	for (i=0; i < MAX_QUEUED_DEFFS; i++)
		if (deff_queue[i] == 0)
		{
			deff_queue[i] = dn;
			return;
		}

	fatal (ERR_DEFF_QUEUE_FULL);
}


static void deff_remove_queue (deffnum_t dn)
{
	uint8_t i;
	for (i=0; i < MAX_QUEUED_DEFFS; i++)
		if (deff_queue[i] == dn)
			deff_queue[i] = 0;
}


static deffnum_t deff_get_highest_priority (void)
{
	uint8_t i;
	uint8_t prio = 0;
	uint8_t best = DEFF_NULL;

	for (i=0; i < MAX_QUEUED_DEFFS; i++)
	{
		if (deff_queue[i] != 0)
		{
			const deff_t *deff = &deff_table[deff_queue[i]];
			if (deff->prio > prio)
			{
				prio = deff->prio;
				best = deff_queue[i];
			}
		}
	}

	/* Save the priority of the best deff here */
	deff_prio = prio;

	/* Return the deffnum of the best deff to the caller */
	return best;
}


void deff_start (deffnum_t dn)
{
	const deff_t *deff = &deff_table[dn];

	db_puts ("Deff start request for # "); db_puti (dn); db_putc ('\n');

	if (deff->flags & D_RUNNING)
	{
		db_puts ("Adding running deff to queue\n");
		deff_add_queue (dn);
		if (dn == deff_get_highest_priority ())
		{
			/* This is the new active running deff */
			db_puts ("Requested deff is now highest priority\n");
			deff_active = dn;
			task_recreate_gid (GID_DEFF, deff->fn);
		}
		else
		{
			/* This deff cannot run now, because there is a
			 * higher priority deff running. */
			db_puts ("Can't run because higher priority active\n");
		}
	}
	else
	{
		if (deff->prio > deff_prio)
		{
			db_puts ("Restarting quick deff with high pri\n");
			deff_active = dn;
			task_recreate_gid (GID_DEFF, deff->fn);
		}
		else
		{
			db_puts ("Quick deff lacks pri to run\n");
		}
	}
}


void deff_stop (deffnum_t dn)
{
	const deff_t *deff = &deff_table[dn];

	db_puts ("Stopping deff # "); db_puti (dn); db_putc ('\n');

	if (deff->flags & D_RUNNING)
	{
		db_puts ("Remove running deff from queue\n");
		deff_remove_queue (dn);

		deff_start_highest_priority ();
	}
	else
	{
		fatal (ERR_NOT_IMPLEMENTED_YET);
	}
}


void deff_restart (deffnum_t dn)
{
	if (dn == deff_active)
	{
		const deff_t *deff = &deff_table[dn];
		task_recreate_gid (GID_DEFF, deff->fn);
	}
	else
	{
		deff_start (dn);
	}
}


void deff_default (void)
{
	dmd_alloc_low_clean ();
	dmd_show_low ();
	for (;;)
	{
		task_sleep_sec (10);
	}
}


void deff_start_highest_priority (void)
{
	db_puts ("Restarting highest priority deff\n");

	deff_active = deff_get_highest_priority ();
	if (deff_active != DEFF_NULL)
	{
		const deff_t *deff = &deff_table[deff_active];
		db_puts ("Recreating deff task\n");
		task_recreate_gid (GID_DEFF, deff->fn);
	}
	else
	{
		task_recreate_gid (GID_DEFF, deff_default);
	}
}


__noreturn__ void deff_exit (void)
{
	db_puts ("Exiting deff\n");
	task_setgid (GID_DEFF_EXITING);
	db_puts ("Remove deff_active from queue\n");
	deff_remove_queue (deff_active);
	deff_start_highest_priority ();
	task_exit ();
}


void deff_delay_and_exit (task_ticks_t ticks)
{
	task_sleep (ticks);
	deff_exit ();
}


void deff_swap_low_high (int8_t count, task_ticks_t delay)
{
	while (--count >= 0)
	{
		dmd_show_other ();
		task_sleep (delay);
	}
}


void deff_init (void)
{
	deff_prio = 0;
	deff_active = DEFF_NULL;
	memset (deff_queue, 0, MAX_QUEUED_DEFFS);
}


