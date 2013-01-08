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
	int row, col;
	long miliseconds;
	int ms, sec, min;
	struct passwd *pwentry;
	float temp;
	char suffixes[] = " kmgt";
	extern pmstat memory;

	getmaxyx(stdscr,row,col);

	mvprintw ( 1, 0, "KiB Mem:%10lu total, %10lu used, %10lu free, %10lu buffers", memory->memtotal, memory->memtotal - memory->memfree, memory->memfree, memory->buffers );
	mvprintw ( 2, 0, "KiB Swap:%9d total, %10lu used, %10lu free, %10lu cached", memory->swaptotal, memory->swaptotal - memory->swapfree, memory->swapfree, memory->cached );
	mvprintw ( 3, 0, "JKUtop - horrovac invenit et fecit" );
	attron ( A_REVERSE );
	mvprintw ( 4, 0, "%7s %-8s %2s %3s %5s %4s %1s %6s %4s %9s %-s", "PID", "USER", "PR", "NI", "VIRT", "RES", "S", "%CPU", "%MEM", "TIME+", "COMMAND" );
	for ( i = 67; i < col; i++ )
	{
		printw ( " " );
	}
	printw ( "\n" );
	attroff ( A_REVERSE );

	//c = 0;
	for ( i = 0; i < count; i++ )
	{
		/*
		if ( current->swap[0] )
		*/
		{
			pwentry = getpwuid ( stats_array[i]->uid );
			pwentry->pw_name[8] = '\0';
			printw ( "%7d %8s ", stats_array[i]->pid, pwentry->pw_name );
			if ( stats_array[i]->priority < 0 )
			{
				printw ( "rt %3ld ", stats_array[i]->niceness );
			}
			else
			{
				printw ( "%2ld %3ld ", stats_array[i]->priority, stats_array[i]->niceness );
			}
			temp = stats_array[i]->virt;
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
			temp = stats_array[i]->res * getpagesize();
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
			/*
			printw ( ">>%8Lu %8Lu %8Lu %8Lu %8f<< ", stats_array[i]->utime, stats_array[i]->stime, stats_array[i]->utime_lastpass, stats_array[i]->stime_lastpass, ticks_passed );
			if ( ticks_passed > 0 )
			{
				printw ( "%6.1f ", ( ( ( stats_array[i]->utime + stats_array[i]->stime ) - (stats_array[i]->utime_lastpass + stats_array[i]->stime_lastpass) ) / ticks_passed ) * 100 );
			}
			else
			{
				printw ( "%2d ", 0 );
			}
			*/
			printw ( "%c ", stats_array[i]->state );
			printw ( "%6.1f ", stats_array[i]->cpu_percent );
			printw ( "%4.1f ", ( (float) stats_array[i]->res * getpagesize() * .1024 ) / memory->memtotal );
			/*
			calculate values for time display
			*/
			miliseconds = ( ( stats_array[i]->utime + stats_array[i]->stime ) / (float) sysconf ( _SC_CLK_TCK ) ) * 100;
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
			/*
			reset for the next round
			*/
			min = 0;

			printw ( "%-20s\n", stats_array[i]->name );
			//printw ( "%c %10lu %-20s\n", stats_array[i]->state, ( stats_array[i]->utime + stats_array[i]->stime + stats_array[i]->cutime + stats_array[i]->cstime ) / sysconf ( _SC_CLK_TCK ), stats_array[i]->name );

			//printw ( "%-15s %8d %8d %10d %10.2f %10.2f %5c\n", stats_array[i]->name, stats_array[i]->swap[0], stats_array[i]->pid, stats_array[i]->ppid, (float) stats_array[i]->user / HZ, (float) stats_array[i]->kernel / HZ, stats_array[i]->state );
			//printw ( "%-15s %8d %8d %8d %8d %8d %8d\n", current->name, current->swap[0], current->swap[1], current->swap[2], current->swap[3], current->swap[4], current->swapchange );
		}
		/* limit output to 40 rows */
		if ( i + 7 >= row )
		{
			break;
		}
	}
	printw ( "%d records, %d\n", count, (int) time( NULL ) );
	refresh();
	return ( i + 1 );
}
