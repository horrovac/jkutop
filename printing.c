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

#define MENU_WIDTH 16

extern char suffixes[];
extern repr fields[];


int print_it ( ppstat *stats_array, int count )
{
	char input;
	int i, j;
	int hertz;
	extern pmstat memory;
	extern double ticks_passed;
	double	total_ticks;
	extern cstats cpu_stats[];

	hertz = sysconf ( _SC_CLK_TCK );
	total_ticks = ticks_passed * sysconf ( _SC_NPROCESSORS_ONLN );
	keypad ( win, TRUE );
	erase();

	if ( cpu_stats[1].user > 0 )
	{
		mvprintw ( 2, 0, "Cpu(s):%5.1f us,%5.1f sy,%5.1f ni,%5.1f id,%5.1f wa,%5.1f hi,%5.1f si,%5.1f st",
		((cpu_stats[0].user - cpu_stats[1].user)/total_ticks) * 100,
	 	((cpu_stats[0].system - cpu_stats[1].system)/total_ticks) * 100,
		((cpu_stats[0].nice - cpu_stats[1].nice)/total_ticks) * 100 ,
		((cpu_stats[0].idle - cpu_stats[1].idle)/total_ticks) * 100 ,
		((cpu_stats[0].iowait - cpu_stats[1].iowait)/total_ticks) * 100,
		((cpu_stats[0].irq - cpu_stats[1].irq)/total_ticks) * 100,
		((cpu_stats[0].softirq - cpu_stats[1].softirq)/total_ticks) * 100,
		((cpu_stats[0].steal - cpu_stats[1].steal)/total_ticks) * 100 );
	}
	else
	{
		mvprintw ( 2, 0, "Cpu(s):%5.1f us,%5.1f sy,%5.1f ni,%5.1f id,%5.1f wa,%5.1f hi,%5.1f si,%5.1f st", 0, 0, 0, 0, 0, 0, 0, 0 );
	}
	mvprintw ( 3, 0, "KiB Mem:%10lu total, %10lu used, %10lu free, %10lu buffers", memory->memtotal, memory->memtotal - memory->memfree, memory->memfree, memory->buffers );
	mvprintw ( 4, 0, "KiB Swap:%10lu total, %10lu used, %10lu free, %10lu cached", memory->swaptotal, memory->swaptotal - memory->swapfree, memory->swapfree, memory->cached );
	mvprintw ( 5, 0, "JKUtop - horrovac invenit et fecit" );
	attron ( A_REVERSE );
	/*
	print the header
	*/
	move ( 6, 0 );
	for ( j = 0; j < FIELDS_AVAILABLE; j++ )
	{
		if ( display_fields[j] == NULL )
		{
			break;
		}
		printw ( display_fields[j]->header_format, display_fields[j]->fieldname );
	}
	for ( i = 67; i < col; i++ )
	{
		printw ( " " );
	}
	printw ( "\n" );
	attroff ( A_REVERSE );

	/*
	print the data fields
	*/
	for ( i = 0; i < count; i++ )
	{
		move ( i + 7, 0 );
		{
			if ( stats_array[i]->state == 'R' )
			{
			attron ( A_BOLD );
			}
			else
			{
				attroff ( A_BOLD );
			}
			for ( j = 0; j < FIELDS_AVAILABLE; j++ )
			{
				if ( display_fields[j] == NULL )
				{
					break;
				}
				display_fields[j]->printout( stats_array[i], display_fields[j]->identifier );
			}
			printw ( "\n" );

		}
		/* limit output to visible rows */
		if ( i + 9 >= row )
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

void modify_display ( void )
{
	int i;
	int focus = 0;
	int c;
	int x,y;
	int maxx,maxy;
	int ll_focus_y = 0;
	int ll_focus_x = 0;
	WINDOW *mod_win;

	getmaxyx ( win, maxy, maxx );
	mod_win = newwin ( 8, maxx, 0, 0 );
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
		wmove ( mod_win, 6, 0 );
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
			wprintw ( mod_win, display_fields[i]->header_format, display_fields[i]->fieldname );
			if ( i == focus )
			{
				wattroff ( mod_win, A_REVERSE );
				/* find out where to print the menu in case we need one */
				getyx ( mod_win, y, x );
				ll_focus_y = y + 1;
				ll_focus_x = x - display_fields[i]->field_length;
				/*
				  keep menu inside the current window - in case it would "fall
				  off" off the edge, move it inwards by the width of the menu
				*/
				if ( maxx < ( x + MENU_WIDTH ) )
				{
					ll_focus_x = maxx - MENU_WIDTH;
				}

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
			case 'i':		/* insert a field */
				for ( i = focus + 1; display_fields[i] != 0; i++ )
				{
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
			case 27:	/* ESC key */
				goto end;
				break;
			default:
				printf ( "\a" );
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


prepr select_field ( int y, int x, prepr current )
{
	WINDOW *menu_win;
	prepr retval = current;
	int i;
	int c;
	int highlight = 0;

	cbreak();
	menu_win = newwin ( FIELDS_AVAILABLE + 2, MENU_WIDTH, y, x );
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
			case 27:	/* ESC key */
				goto end;
				break;
			default:
				printf ( "\a" );
				break;
		}
	}
	end: ;
	keypad ( menu_win, FALSE );
	delwin ( menu_win );
	return ( retval );
}

void print_pid ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->pid );
}

void print_ppid ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->ppid );
}

void print_user ( ppstat entry, int identifier )
{
	struct passwd *pwentry;
	pwentry = getpwuid ( entry->uid );

	/* shorten the name to 8 char max */
	pwentry->pw_name[8] = '\0';
	printw ( fields[identifier].format, pwentry->pw_name );
}

void print_uid ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->uid );
}

void print_priority ( ppstat entry, int identifier )
{
	if ( entry->priority < 0 )
	{
		printw ( fields[identifier].format, "rt" );
	}
	else
	{
		printw ( fields[identifier].format_alt, entry->priority );
	}
}

void print_niceness ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->niceness );
}

void print_virt ( ppstat entry, int identifier )
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
		printw ( fields[identifier].format, (long) temp, suffixes[c] );
	}
	else
	{
		printw ( fields[identifier].format_alt, (long) temp );
	}
}

void print_res ( ppstat entry, int identifier )
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
		printw ( fields[identifier].format, (long unsigned) temp, suffixes[c] );
	}
	else
	{
		printw ( fields[identifier].format_alt, (long unsigned) temp );
	}
}

void print_status ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->state );
}

void print_cpu_percent ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->cpu_percent );
}

void print_mem_percent ( ppstat entry, int identifier )
{
	extern pmstat memory;
	printw ( fields[identifier].format, ( (float) entry->res * getpagesize() * .1024 ) / memory->memtotal );
}

void print_swap ( ppstat entry, int identifier )
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
		printw ( fields[identifier].format, temp, suffixes[c] );
	}
	else
	{
		printw ( fields[identifier].format_alt, temp );
	}
}


void print_time ( ppstat entry, int identifier )
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
		printw ( fields[identifier].format, min, sec );
	}
	else
	{
		printw ( fields[identifier].format_alt, min, sec, ms );
	}
}

void print_name ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->name );
}

void print_minflt ( ppstat entry, int identifier )
{
	float temp;
	int c;
	temp = entry->minflt;
	for ( c = 0; temp > 1000; c++ )
	{
		temp /= 1024;
	}
	if ( c > 0 )
	{
		printw ( fields[identifier].format, (long unsigned) temp, suffixes[c] );
	}
	else
	{
		printw ( fields[identifier].format_alt, (long unsigned) temp );
	}
}

void print_majflt ( ppstat entry, int identifier )
{
	float temp;
	int c;
	temp = entry->majflt;
	for ( c = 0; temp > 1000; c++ )
	{
		temp /= 1024;
	}
	if ( c > 0 )
	{
		printw ( fields[identifier].format, (long unsigned) temp, suffixes[c] );
	}
	else
	{
		printw ( fields[identifier].format_alt, (long unsigned) temp );
	}
}

void print_majflt_delta ( ppstat entry, int identifier )
{
	float temp;
	int c;
	temp = entry->majflt_delta;
	for ( c = 0; temp > 1000; c++ )
	{
		temp /= 1024;
	}
	if ( c > 0 )
	{
		printw ( fields[identifier].format, (long unsigned) temp, suffixes[c] );
	}
	else
	{
		printw ( fields[identifier].format_alt, (long unsigned) temp );
	}
}

void print_system_cpu_percent ( ppstat entry, int identifier )
{
	printw ( fields[identifier].format, entry->system_cpu_percent );
}
