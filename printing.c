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
#include "jkutop.h"

#define MENU_WIDTH 16

extern char suffixes[];
extern repr fields[];


int print_it ( ppstat *stats_array, int count )
{
	int input;
	int i, j;
	int hertz;
	extern pmstat memory;
	extern double ticks_passed;
	double	total_ticks;
	extern cstats cpu_stats[];
	MEVENT event;

	mousemask(ALL_MOUSE_EVENTS, NULL);
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
		attroff ( A_BOLD );
		if ( display_fields[j] == NULL )
		{
			break;
		}
		if ( display_fields[j]->identifier == parametres.sortby )
		{
			attron ( A_BOLD );
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
			case 'w':		/* save current configuration */
				save_config();
				break;
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
			case KEY_MOUSE:
				if(getmouse(&event) == OK)
				{
					if ( event.bstate & BUTTON1_DOUBLE_CLICKED )
					{
						if ( event.y > 6 )
						{
							show_process_detail ( stats_array, event.y - 7 );
						}
						else if ( event.y == 6 )
						{
							mouse_select_sortfield ( event.x );
						}
					}
				}
				break;
			default:
				break;
		}
	}
	return ( i + 1 );
}

int mouse_select_sortfield ( int x )
{
	int i;
	int reach = 0;
	for ( i = 0; i < FIELDS_AVAILABLE; i++ )
	{
		if ( display_fields[i] == NULL )
		{
			break;
		}
		reach += display_fields[i]->field_length;
		if ( reach > x )
		{
			/*we're on this field */
			if ( display_fields[i]->sortable == 0 )
			{
				/* we can't sort by this field */
				printf ( "\a" );
				attron ( A_REVERSE );
				mvprintw ( 5, 0, "%-79s", "Sorry, can't sort by this field!" );
				attroff ( A_REVERSE );
				refresh();
				sleep ( 1 );
				break;
			}
			else if ( parametres.sortby == display_fields[i]->identifier )
			{
				/* if we're already sorting by this field, switch
				   the sort order
				 */
				 parametres.reversesort ^= 1;
			}
			else
			{
				/* otherwise set this as sortfield and descending order */
				parametres.sortby = display_fields[i]->identifier;
				parametres.reversesort = 0;
			}
			break;
		}
	}
	return ( 0 );
}

int show_process_detail ( ppstat *stats_array, int member )
{
	WINDOW *detail_win;
	char procname[256];
	int input;
	struct passwd *pwentry;
	struct group	*grentry;

	sprintf ( procname, "%d", stats_array[member]->pid );
	read_cpuset ( stats_array[member], procname );

	pwentry = getpwuid ( stats_array[member]->uid );
	grentry = getgrgid ( pwentry->pw_gid );
	detail_win = newwin ( 21, 60, 1, 1 );
	cbreak();
	while ( 1 )
	{
		wborder ( detail_win, 0, 0, 0, 0, 0, 0, 0, 0 );
		/* UID */
		mvwprintw ( detail_win, 1, 1, "%-10s %15d, %s (%s)", "UID", stats_array[member]->uid, pwentry->pw_name, pwentry->pw_gecos );
		/* GID */
		mvwprintw ( detail_win, 2, 1, "%-10s %15d, %s", "GID", grentry->gr_gid, grentry->gr_name );
		/* PID */
		mvwprintw ( detail_win, 3, 1, "%-10s %15d", "PID", stats_array[member]->pid );
		/* PPID */
		mvwprintw ( detail_win, 4, 1, "%-10s %15d", "PPID", stats_array[member]->ppid );
		/* PGRP */
		mvwprintw ( detail_win, 5, 1, "%-10s %15d", "PGRP", stats_array[member]->pgrp );
		/* SESSION */
		mvwprintw ( detail_win, 6, 1, "%-10s %15d", "SessionID", stats_array[member]->session );
		/* Threads */
		mvwprintw ( detail_win, 7, 1, "%-10s %15ld", "#threads", stats_array[member]->num_threads );
		/* Cpuset */
		mvwprintw ( detail_win, 8, 1, "%-10s %15s", "Cpuset", stats_array[member]->cpuset );
		/* nMaj */
		mvwprintw ( detail_win, 9, 1, "%-10s %15d", "nMaj", stats_array[member]->majflt );
		/* nMin */
		mvwprintw ( detail_win, 10, 1, "%-10s %15d", "nMin", stats_array[member]->minflt );
		/* instructions */
		mvwprintw ( detail_win, 19, 12, "Click or press any key to close" );
		/* instructions */

		wrefresh(detail_win);
		input = getch();
		switch (input)
		{
			default:
				goto end;
				break;
		}
	}
	end: ;
	halfdelay ( 30 );
	return ( 0 );
}



int displayfield_shift_up ( int focus )
{
	int result = focus;
	if ( focus == MAX_DISPLAY_FIELDS )
	{
		return ( 0 );
	}
	else if ( display_fields[focus] != NULL )
	{
		result = displayfield_shift_up ( focus + 1 );
	}
	if ( result != 0 )
	{
		display_fields[focus] = display_fields[focus-1];
	}
	return focus;
}

int displayfield_shift_down ( int focus )
{
	display_fields[focus] = display_fields[focus + 1];
	if ( display_fields[focus] != NULL )
	{
		displayfield_shift_down ( focus + 1 );
	}
	return focus;
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
		mvwprintw ( mod_win,  3, 0, "'i' to insert a field, 'a' to append it, 'd' to delete");
		wmove ( mod_win, 6, 0 );
		for ( i = 0; i < MAX_DISPLAY_FIELDS; i++ )
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
			case 's':
			case 'S':
				if (  display_fields[focus]->sortable == 1 )
				{
				 	parametres.sortby = display_fields[focus]->identifier;
					goto end;
				}
				else
				{
					printf ( "\a" );
					mvwprintw ( mod_win,  5, 0, "%-79s", "Sorry, can't sort by this field");
					wrefresh ( mod_win );
					sleep ( 1 );
				}
				break;
			case 'r':
			case 'R':
				parametres.reversesort ^= 1;
				break;
			case 'd':
					displayfield_shift_down ( focus );
				break;
			case 'i':		/* insert a field */
				if ( displayfield_shift_up ( focus + 1 ) != 0 )
				{
					ungetch ( 'j' );
				}
				break;
			case 'a':		/* append a field */
				focus++;
				if ( displayfield_shift_up ( focus ) != 0 )
				{
					ungetch ( 'j' );
				}
				else
				{
					focus--;
				}
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
	parametres.requested_fields = 0;
	for ( i = 0; i < MAX_DISPLAY_FIELDS; i++ )
	{
		if ( display_fields[i] == NULL )
		{
			break;
		}
		parametres.requested_fields |= 1 << display_fields[i]->identifier;
	}
}


prepr select_field ( int y, int x, prepr current )
{
	WINDOW *menu_win;
	prepr retval = current;
	int i, j;
	int menu_height;
	int c;
	int highlight = 0;
	extern  int row;

	cbreak();
	menu_height = FIELDS_AVAILABLE + 2;
	if ( y + FIELDS_AVAILABLE + 2 > row )
	{
		menu_height =  row - y;
	}
	menu_win = newwin ( menu_height, MENU_WIDTH, y, x );
	keypad ( menu_win, TRUE );
	wborder ( menu_win, 0, 0, 0, 0, 0, 0, 0, 0 );

	while ( 1 )
	{
		j = 0;
		if ( highlight > menu_height - 3 )
		{
			j = highlight - ( menu_height - 3 );
		}
		for ( i = 0; i < FIELDS_AVAILABLE; i++ )
		{
			if ( i > menu_height - 3 )
			{
				break;
			}
			wattroff ( menu_win, A_REVERSE );
			if ( j == highlight )
			{
				wattron ( menu_win, A_REVERSE );
			}
			/*
			wattroff ( menu_win, A_INVIS );
			if ( ! ( parametres.requested_fields & 1 << i ) )
			{
				wattron ( menu_win, A_INVIS );
			}
			*/
			mvwprintw ( menu_win, i+1, 2, "%12s", fields[j].fieldname );
			if ( current->identifier == fields[i].identifier )
			{
				wprintw ( menu_win, "*" );
			}
			j++;
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

void print_cpuset ( ppstat entry, int identifier )
{
	char temp;
	temp = entry->cpuset[30];
	entry->cpuset[30] = '\n';
	printw ( fields[identifier].format, entry->cpuset );
	entry->cpuset[30] = temp;
}
