/*
 *
 * Copyright (C) 2014 Impex-Sat Gmbh & Co.KG
 * Written by Sandro Cavazzoni <sandro@skanetwork.com>
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include "nfr4x_common.h"
#include "nfr4x_log.h"
#include "nfr4x_input.h"

#define nfr4x_input_max_fds 10

static int nfr4x_input_num_fds = 0;
static int nfr4x_input_fd[nfr4x_input_max_fds];
static int nfr4x_input_last_event_code = -1;
static int nfr4x_input_last_event_count = 0;

int nfr4x_input_open()
{
	while (nfr4x_input_num_fds < nfr4x_input_max_fds)
	{
		char filename[32];
		sprintf(filename, "/dev/input/event%d", nfr4x_input_num_fds);
		if ((nfr4x_input_fd[nfr4x_input_num_fds] = open(filename, O_RDONLY | O_NONBLOCK)) == -1)
			break;
		nfr4x_input_num_fds++;
	}

	if (nfr4x_input_num_fds == 0)
	{
		nfr4x_log(LOG_ERROR, "%-33s: cannot open input device", __FUNCTION__);
		return nfr4x_ERROR;
	}

	nfr4x_log(LOG_DEBUG, "%-33s: input device opened", __FUNCTION__);
	
	return nfr4x_SUCCESS;
}

int nfr4x_input_get_code()
{
	struct input_event event;
	int i = 0;
	while (i < nfr4x_input_num_fds)
	{
		if (read(nfr4x_input_fd[i], &event, sizeof(event)) == sizeof(event))
		{
			if (!event.code)
				continue;

			if (event.type == EV_KEY && (event.value == 0 || event.value == 2))
				return event.code;
		}
		i++;
	}

	return -1;
}

void nfr4x_input_close()
{
	int i = 0;
	for (; i < nfr4x_input_num_fds; i++)
		close(nfr4x_input_fd[i]);
}
