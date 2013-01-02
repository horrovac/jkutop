#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>
#include "def.h"

int print_it ( ppstat *stats_array, int count )
{
	int i;
	int c;
	long miliseconds;
	int ms, sec, min, h;
	struct passwd *pwentry;
	float temp;
	char suffixes[] = " kmgt";
	printf ( "%-15s %8s %8s %10s %10s %10s %5s\n", "CMD", "SWAP (kB)", "PID", "PPID", "USER", "SYSTEM", "STATE" );

	//c = 0;
	for ( i = 0; i < count; i++ )
	{
		/*
		current = stats[i];
		while ( current != NULL )
		{
			if ( current->swap[0] )
			*/
			{
				pwentry = getpwuid ( stats_array[i]->uid );
				printf ( "%6d %8s ", stats_array[i]->pid, pwentry->pw_name );
				if ( stats_array[i]->priority < 0 )
				{
					printf ( "rt %3ld ", stats_array[i]->niceness );
				}
				else
				{
					printf ( "%2ld %3ld ", stats_array[i]->priority, stats_array[i]->niceness );
				}
				temp = stats_array[i]->virt;
				for ( c = 0; temp > 10000; c++ )
				{
					temp /= 1024;
				}
				if ( c > 1 )
				{
					printf ( "%4ld%c ", (long) temp, suffixes[c] );
				}
				else
				{
					printf ( "%5ld ", (long) temp );
				}
				temp = stats_array[i]->res;
				for ( c = 0; temp > 1000; c++ )
				{
					temp /= 1024;
				}
				if ( c > 1 )
				{
					printf ( "%3lu%c ", (long unsigned) temp, suffixes[c] );
				}
				else
				{
					printf ( "%4lu ", (long unsigned) temp );
				}
				printf ( "%c ", stats_array[i]->state );
				/*
				calculate values for time display
				*/
				miliseconds = ( ( stats_array[i]->utime + stats_array[i]->stime + stats_array[i]->cutime + stats_array[i]->cstime ) / (float) sysconf ( _SC_CLK_TCK ) ) * 100;
				printf ( "%10ld ", miliseconds );
				ms = miliseconds % 100;
				miliseconds /= 100;
				sec = miliseconds % 60;
				min = miliseconds / 60;

				if ( min >= 1000 )
				{
					printf ( "%6d:%02d ", min, sec );
				}
				else
				{
					printf ( "%3d:%02d.%02d ", min, sec, ms );
				}
				/*
				reset for the next round
				*/
				h = 0;

				printf ( "%-20s\n", stats_array[i]->name );
				//printf ( "%c %10lu %-20s\n", stats_array[i]->state, ( stats_array[i]->utime + stats_array[i]->stime + stats_array[i]->cutime + stats_array[i]->cstime ) / sysconf ( _SC_CLK_TCK ), stats_array[i]->name );

				//printf ( "%-15s %8d %8d %10d %10.2f %10.2f %5c\n", stats_array[i]->name, stats_array[i]->swap[0], stats_array[i]->pid, stats_array[i]->ppid, (float) stats_array[i]->user / HZ, (float) stats_array[i]->kernel / HZ, stats_array[i]->state );
				//printf ( "%-15s %8d %8d %8d %8d %8d %8d\n", current->name, current->swap[0], current->swap[1], current->swap[2], current->swap[3], current->swap[4], current->swapchange );
			}
			//c++;
			//current = current->next;
		//}
	}
	printf ( "%d records, %d\n", count, (int) time( NULL ) );
	return ( i + 1 );
}
