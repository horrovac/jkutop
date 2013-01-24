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
#include "jkutop.h"

extern char suffixes[];
extern repr fields[];

prepr select_field ( int y, int x, prepr current )
{
	WINDOW *menu_win;
	prepr retval = current;
	int i;
	int c;
	int highlight = 0;

	cbreak();
	menu_win = newwin ( FIELDS_AVAILABLE + 2, 16, y, x );
	keypad ( menu_win, TRUE );
	wborder ( menu_win, 0, 0, 0, 0, 0, 0, 0, 0 );

	while ( 1 )
	{
		for ( i = 0; i < FIELDS_AVAILABLE; i++ )
		{
			wattroff ( menu_win, A_REVERSE );
			if ( i == highlight )
			{
				wattron ( menu_win, A_REVERSE );
			}
			mvwprintw ( menu_win, i+1, 2, "%12s", fields[i].fieldname );
			if ( current->identifier == fields[i].identifier )
			{
				wprintw ( menu_win, "*" );
			}
		}
		wrefresh( menu_win );
		c = getch();
		switch ( c )
		{
			case 'j':
			case KEY_DOWN:
				if ( highlight < FIELDS_AVAILABLE - 1 )
				{
					highlight++;
				}
				break;
			case 'k':
			case KEY_UP:
				if ( highlight > 0 )
				{
					highlight--;
				}
				break;
			case 's':
			case KEY_ENTER:
			case ' ':
				retval = &fields[highlight];
				goto end;
				break;
			case 'q':
				goto end;
				break;
			default:
				break;
		}
	}
	end: ;
	keypad ( menu_win, FALSE );
	delwin ( menu_win );
	return ( retval );
}

void modify_display ( void )
{
	int i;
	int focus = 0;
	int c;
	int x,y;
	int ll_focus_y = 0;
	int ll_focus_x = 0;
	WINDOW *mod_win;

	getmaxyx ( win, y, x );
	mod_win = newwin ( 6, x, 0, 0 );
	cbreak();
	raw();
	nonl();
	keypad ( mod_win, TRUE );
	while ( 1 )
	{
		werase( mod_win );
		wrefresh( mod_win );
		mvwprintw ( mod_win,  0, 0, "Use arrow keys (or vim cursor keys) to navigate;" );
		mvwprintw ( mod_win,  1, 0, "Set sort field (highlighted) with 's', order %s (change with 'r')", parametres.reversesort ? "ascending" : "descending" );
		mvwprintw ( mod_win,  2, 0, "go down or press enter to change a field, select with space key");
		wmove ( mod_win, 4, 0 );
		for ( i = 0; i < 20; i++ )
		{
			wattron ( mod_win, A_REVERSE );
			if ( display_fields[i] == NULL )
			{
				break;
			}
			if ( display_fields[i]->identifier == parametres.sortby )
			{
				wattroff ( mod_win, A_REVERSE );
			}
			wprintw ( mod_win, display_fields[i]->format, display_fields[i]->fieldname );
			if ( i == focus )
			{
				wattroff ( mod_win, A_REVERSE );
				getyx ( mod_win, y, x );
				ll_focus_y = y + 1;
				ll_focus_x = x - display_fields[i]->field_length;
				mvwaddch( mod_win, y - 1, x - display_fields[i]->field_length - 1 , ACS_ULCORNER);
				mvwaddch( mod_win, y + 1, x - display_fields[i]->field_length - 1 , ACS_LLCORNER);
				mvwaddch( mod_win, y + 1, x -1, ACS_LRCORNER);
				mvwaddch( mod_win, y - 1, x - 1 , ACS_URCORNER);
				mvwvline( mod_win, y , x - display_fields[i]->field_length - 1 , ACS_VLINE, 1 );
				mvwvline( mod_win, y , x - 1 , ACS_VLINE, 1 );
				mvwhline( mod_win, y - 1, x - display_fields[i]->field_length, ACS_HLINE, display_fields[i]->field_length - 1 );
				mvwhline( mod_win, y + 1, x - display_fields[i]->field_length, ACS_HLINE, display_fields[i]->field_length - 1 );
				wmove ( mod_win, y, x );
			}
		}
		wrefresh( mod_win );
		c = getch();
		switch ( c )
		{
			case 'h':
			case KEY_LEFT:
				if ( focus > 0 )
				{
					focus--;
				}
				break;
			case 'l':
			case KEY_RIGHT:
				if ( display_fields[focus + 1] != NULL )
				{
					focus++;
				}
				break;
			case 's':
			case 'S':
				parametres.sortby = display_fields[focus]->identifier;
				break;
			case 'r':
			case 'R':
				parametres.reversesort ^= 1;
				break;
			case 'j':
			case KEY_DOWN:
			case KEY_ENTER:
				display_fields[focus] = select_field ( ll_focus_y, ll_focus_x, display_fields[focus] );
				goto end;
				break;
			case 'q':
				goto end;
				break;
				/* blah */
				break;
			default:
				break;
		}
		wattroff ( mod_win, A_REVERSE );
	}
	end: ;
	keypad ( mod_win, FALSE );
	halfdelay ( 30 );
	werase( mod_win );
	delwin( mod_win );
}

int print_it ( ppstat *stats_array, int count )
{
	char input;
	int i, j;
	extern pmstat memory;
	keypad ( win, TRUE );
	erase();

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
		if ( display_fields[j] == NULL )
		{
			break;
		}
		printw ( display_fields[j]->format, display_fields[j]->fieldname );
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
				if ( display_fields[j] == NULL )
				{
					break;
				}
				display_fields[j]->printout( stats_array[i] );
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
	if ( ( input = getch () ) != ERR )
	{
		switch ( input )
		{
			case 'q':
				endwin();
				exit ( 0 );
				break;
			case 'o':
			case 'O':
			case 's':
			case 'S':
			case 'f':
			case 'F':
				modify_display();
				break;
			default:
				break;
		}
	}
	return ( i + 1 );
}

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


