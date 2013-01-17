/* Copyright 2013, Faruk Kujundzic

This file is part of jkutop.

jkutop is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

jkutop is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with jkutop.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "def.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct representation
{
	int		active;
	char	format[10];
	char	fieldname[20];
	void	(*printout)(ppstat entry);
}repr, prepr;

char suffixes[] = " kmgt";

void print_pid ( ppstat entry )
{
	printw ( "%7d ", entry->pid );
}

void print_user ( ppstat entry )
{
	struct passwd *pwentry;
	pwentry = getpwuid ( entry->uid );

	/* shorten the name to 8 char max */
	pwentry->pw_name[8] = '\0';
	printw ( "%8s ", pwentry->pw_name );
}

void print_priority ( ppstat entry )
{
	if ( entry->priority < 0 )
	{
		printw ( "rt " );
	}
	else
	{
		printw ( "%2ld ", entry->priority );
	}
}

void print_niceness ( ppstat entry )
{
	printw ( "%3ld ", entry->niceness );
}

void print_virt ( ppstat entry )
{
	float temp;
	int c;
	temp = entry->virt;
	for ( c = 0; temp > 10000; c++ )
	{
		temp /= 1024;
	}
	if ( c > 1 )
	{
		printw ( "%4ld%c ", (long) temp, suffixes[c] );
	}
	else
	{
		printw ( "%5ld ", (long) temp );
	}
}

void print_res ( ppstat entry )
{
	float temp;
	int c;
	temp = entry->virt;
	temp = entry->res * getpagesize();
	for ( c = 0; temp > 1000; c++ )
	{
		temp /= 1024;
	}
	if ( c > 1 )
	{
		printw ( "%3lu%c ", (long unsigned) temp, suffixes[c] );
	}
	else
	{
		printw ( "%4lu ", (long unsigned) temp );
	}
}

void print_status ( ppstat entry )
{
	 printw ( "%c ", entry->state );
}

void print_cpu_percent ( ppstat entry )
{
	printw ( "%6.1f ", entry->cpu_percent );
}

void print_mem_percent ( ppstat entry )
{
	extern pmstat memory;
	printw ( "%4.1f ", ( (float) entry->res * getpagesize() * .1024 ) / memory->memtotal );
}

void print_swap ( ppstat entry )
{
	int temp;
	int c;
	temp = entry->swap[0];
	for ( c = 1; temp > 10000; c++ )
	{
		temp /= 1024;
	}
	if ( c > 1 )
	{
		printw ( "%4d%c ", temp, suffixes[c] );
	}
	else
	{
		printw ( "%5d ", temp );
	}
}


void print_time ( ppstat entry )
{
	long miliseconds;
	int ms, sec, min;
	miliseconds = ( ( entry->utime + entry->stime ) / (float) sysconf ( _SC_CLK_TCK ) ) * 100;
	ms = miliseconds % 100;
	miliseconds /= 100;
	sec = miliseconds % 60;
	min = miliseconds / 60;

	if ( min >= 1000 )
	{
		printw ( "%6d:%02d ", min, sec );
	}
	else
	{
		printw ( "%3d:%02d.%02d ", min, sec, ms );
	}
}

void print_name ( ppstat entry )
{
	printw ( "%-15s ", entry->name );
}

prepr fields[20] =
{
	{ 1, "%7s ", "PID", print_pid },
	{ 1, "%-8s ", "USER", print_user },
	{ 1, "%2s ", "PR", print_priority },
	{ 1, "%3s ", "NI", print_niceness },
	{ 1, "%5s ", "VIRT", print_virt },
	{ 1, "%4s ", "RES", print_res },
	{ 1, "%1s ", "S", print_status },
	{ 1, "%6s ", "%CPU", print_cpu_percent },
	{ 1, "%4s ", "%MEM", print_mem_percent },
	{ 1, "%4s ", "SWAP", print_swap },
	{ 1, "%9s ", "TIME+", print_time },
	{ 1, "%-s ", "COMMAND", print_name },
	{ 0, "", "",  NULL }
};
