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
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include "printing.h"

int print_it ( ppstat *stats_array, int count )
{
	int i, j;
	extern pmstat memory;

	mvprintw ( 1, 0, "KiB Mem:%10lu total, %10lu used, %10lu free, %10lu buffers", memory->memtotal, memory->memtotal - memory->memfree, memory->memfree, memory->buffers );
	mvprintw ( 2, 0, "KiB Swap:%10lu total, %10lu used, %10lu free, %10lu cached", memory->swaptotal, memory->swaptotal - memory->swapfree, memory->swapfree, memory->cached );
	mvprintw ( 3, 0, "JKUtop - horrovac invenit et fecit" );
	attron ( A_REVERSE );
	/*
	print the header
	mvprintw ( 4, 0, "%7s %-8s %2s %3s %5s %4s %1s %6s %4s %9s %-s", "PID", "USER", "PR", "NI", "VIRT", "RES", "S", "%CPU", "%MEM", "TIME+", "COMMAND" );
	*/
	move ( 4, 0 );
	for ( j = 0; j < 20; j++ )
	{
		if ( fields[j].active == 0 )
		{
			break;
		}
		printw ( fields[j].format, fields[j].fieldname );
	}
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
		move ( i + 5, 0 );
		{
			if ( stats_array[i]->state == 'R' )
			{
			attron ( A_BOLD );
			}
			else
			{
				attroff ( A_BOLD );
			}
			for ( j = 0; j < 20; j++ )
			{
				if ( fields[j].active == 0 )
				{
					break;
				}
				fields[j].printout( stats_array[i] );
			}
			/*
			print_pid ( stats_array[i] );
			print_user ( stats_array[i] );
			print_priority ( stats_array[i] );
			print_niceness ( stats_array[i] );
			print_virt ( stats_array[i] );
			print_res ( stats_array[i] );
			print_status ( stats_array[i] );
			print_cpu_percent ( stats_array[i] );
			print_mem_percent ( stats_array[i] );
			print_time ( stats_array[i] );
			print_name ( stats_array[i] );
			*/
			printw ( "\n" );

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
