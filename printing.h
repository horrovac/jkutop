#include "def.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

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

