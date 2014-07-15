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
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <asm/param.h>		/* neded for HZ definition */
#include <signal.h>
#include <err.h>
#include "jkutop.h"

#define DEBUG_BUILDING 1
#undef DEBUG_CLEANUP

const char *blacklist[] = { "ksoftirqd/", "migration/", "events/", "kintegrityd/", "kblockd/", "kstop/", "kondemand/", "kswapd", "aio/", "crypto/", "ata/", "xfslogd/", "xfsdatad/", "xfsconvertd/", "rpciod/", "kworker", NULL };
ppstat stats[HASH_TABLE_SIZE];
ppstat *stats_array = NULL;
pmstat memory;

/*
 Keep this in sync with the enum defining the identifiers, or you're gonna
 have a very bad time
*/
repr fields[FIELDS_AVAILABLE] =
{
	{ PID, "%7d ", "", "%7s ", 8, 1, "PID", print_pid },
	{ PPID, "%7d ", "", "%7s ", 8, 0, "PPID", print_ppid },
	{ USER, "%8s ", "", "%-8s ", 9, 0, "USER", print_user },
	{ UID, "%6d ", "", "%6s ", 7, 0, "UID", print_uid },
	{ PR, "%2s ", "%2ld ", "%2s ", 3, 0, "PR", print_priority },
	{ NI, "%3ld ", "", "%3s ", 4, 0, "NI", print_niceness },
	{ VIRT, "%4ld%c ", "%5ld ", "%5s ", 6, 1, "VIRT", print_virt },
	{ RES, "%3lu%c ", "%4lu ", "%4s ", 5, 1, "RES", print_res },
	{ S, "%c ", "", "%1s ", 2, 0, "S", print_status },
	{ CPU, "%6.1f ", "", "%6s ", 7, 1, "%CPU", print_cpu_percent },
	{ MEM, "%4.1f ", "", "%4s ", 5, 1, "%MEM", print_mem_percent },
	{ SWAP, "%4d%c ", "%5d ", "%4s ", 5, 1, "SWAP", print_swap },
	{ TIME, "%6d:%02d ", "%3d:%02d.%02d ", "%9s ", 10, 0, "TIME+", print_time },
	{ COMMAND, "%-15s ", "", "%-s ", 8, 0, "COMMAND", print_name },
	{ MINFLT, "%3lu%c ", "%4lu ", "%4s ", 5, 1, "nMin", print_minflt },
	{ MAJFLT, "%3lu%c ", "%4lu ", "%4s ", 5, 1, "nMaj", print_majflt },
	{ MAJFLT_DELTA, "%3lu%c ", "%4lu ", "%4s ", 5, 1, "dFLT", print_majflt_delta },
	{ SYS, "%6.1f ", "", "%6s ", 7, 1, "%SYS", print_system_cpu_percent },
	{ CPUSET, "%20s ", "", "%20s ", 21, 0, "CPUSET", print_cpuset },
	{ NTHR, "%4d ", "", "%4s ", 5, 1, "nTHR", print_nthr }
};

char suffixes[] = " kmgtp";

/* signal handler functions */
void dummy ( int useless ){};
void resize ( int useless )
{
	endwin();
	refresh();
	getmaxyx ( win, row, col );
}


int main ( int argc, char **argv )
{
	DIR				*dirp;
	struct dirent	*dir_entry;
	int				fd;
	extern char		*optarg;
	int				opt;
	int				procstat;	/* fd for /proc/stat */
	int				loadavg;	/* fd for /proc/loadavg */
	char			path[1024];
	char			buffer[BUFFSIZE];
	char			state[1];
	int				i;
	int				allocated=1000;
	int				c=0;
	int				sequence=0;
	int				chars_read;
	pstat			*current = NULL;
	//pstat			*temp = NULL;
	pstat			*stats_buffer;

	/* signal handling stuff */
	struct sigaction	act;
	sigset_t			sigset;
	//int					signal;
	act.sa_handler = dummy;
	act.sa_flags = 0;
	sigaction ( 14, &act, NULL );
	act.sa_handler = resize;
	act.sa_flags = 0;
	sigaction ( 28, &act, NULL );
	sigemptyset ( &sigset );
	sigaddset ( &sigset, SIGALRM );

	/* sets parametres.progname to the name we were called with */
	get_my_name ( argv[0] ); 

	/* if the name is jkups, turn off curses printing */
	if ( strcmp ( parametres.progname, "jkups" ) == 0 )
	{
		parametres.curses=0;
	}

	init_fields();

	/*
	 * get commandline options
	 */
	while ( ( opt = getopt ( argc, argv, "bg:s:u:" ) ) > 0 )
	{
		switch ( opt )
		{
			case 'b':
				parametres.curses = 0;
				break;
			case 'g':
				parametres.restrict_to_gid = get_gid ( optarg );
				break;
			case 's':
				parametres.requested_fields |= 1 << CPUSET;
				parametres.restrict_to_cpuset = 1;
				strncpy ( parametres.requested_cpuset, optarg, CPUSET_NAME_LENGTH_MAX - 1 );
				break;	
			case 'u':
				parametres.restrict_to_uid = get_uid ( optarg );
				break;
			default:
				err ( ERR_OPTION, NULL );
				break;
		}
	}

	stats_array = malloc ( allocated * sizeof ( ppstat ) );
	memory = malloc ( sizeof ( mstat ) );
	memset ( memory, '\0', sizeof ( mstat ) );

	/*
	store stats here temporarily. This is for blacklisting, if the records
	are not desired, they'll be dropped, if not, they'll be memcpy()ed
	into the data. This saves allocating memory for this only to drop it in
	case the record is blacklisted
	*/
	stats_buffer = malloc ( sizeof ( pstat ) );
	
	dirp = opendir ( "/proc" );
	if ( ! ( procstat = open ( "/proc/stat", O_RDONLY )) )
	{
		fprintf ( stderr, "Can't open /proc/stat, something's effed.\n" );
		return 0;
	}

	if ( ! ( loadavg = open ( "/proc/loadavg", O_RDONLY )) )
	{
		fprintf ( stderr, "Can't open /proc/loadavg, something's effed.\n" );
		return 0;
	}

	/* read when the system was booted */
	read_btime ( procstat );

	if ( parametres.curses == 1 )
	{
		win = initscr();		/* ncurses initialisation */
		noecho();				/* don't show keys typed */
	
		/*
		 * set short initial delay so the display is updated with useful
		 * data as soon as possible. Right after the first call of print_it
		 * this is eased off to a delay of 3 seconds
		 */
		halfdelay ( 3 );
		getmaxyx ( win, row, col );
	}

	while ( 1 )
	{
		/* get number of ticks passed since last pass */
		read_proc_stat ( procstat );

		/* read the load averages */
		read_loadavg ( loadavg );
		
		/* read the memory info */
		read_meminfo ( memory );
		rewinddir ( dirp );
		c = 0;
		while ( ( dir_entry = readdir ( dirp ) ) != NULL ) 
		{
			/*
			we're only interested in the process directories, their
			names are numeric only
			*/
			for ( i = 0; dir_entry->d_name[i] != '\0'; i++ )
			{
				if ( ! isdigit ( dir_entry->d_name[i] ) )
				{
					goto next_iteration;
				}
			}

			/*
			passed the test, now open /proc/<pid>/stat and read it
			*/
			strcpy ( path, "/proc/" );
			strcat ( path, dir_entry->d_name );
			strcat ( path, "/stat" );
			if ( (fd = open ( path, O_RDONLY )) )
			{
				if ( ( chars_read = read ( fd, buffer, BUFFSIZE ) ) < 0 )
				{
					/*
					this kind of thing happens regularly on a system as
					ridiculously big as Altix UV. By the time we come around to
					reading the 'stat' file, the process is already gone. To
					fix this one would need to constantly update the contents
					of the /proc directory while parsing through it. I have no
					idea how one could do this. Plus it would probably slow the
					whole thing down. So I'll just ignore this silently and
					move on to the next iteration.
					fprintf ( stderr, "Can't read file %s\n", path );
					perror ( NULL );
					*/
					close ( fd );
					continue;
				}
				buffer[chars_read+1] = '\0';
				sscanf ( buffer, "%d (%[^)]) %c %d %d %d %d %*d %*d %Lu %*d %Lu %*d %Lu %Lu %Ld %Ld %ld %ld %ld %*d %Lu %lu %ld",
					&stats_buffer->pid,
					stats_buffer->name,
					state,
					&stats_buffer->ppid,
					&stats_buffer->pgrp,
					&stats_buffer->session,
					&stats_buffer->tty_nr,
					&stats_buffer->minflt,
					&stats_buffer->majflt,
					&stats_buffer->utime,
					&stats_buffer->stime,
					&stats_buffer->cutime,
					&stats_buffer->cstime,
					&stats_buffer->priority,
					&stats_buffer->niceness,
					&stats_buffer->num_threads,
					&stats_buffer->starttime,
					&stats_buffer->virt,
					&stats_buffer->res
					);
				if ( ! process_filter ( stats_buffer ) )
				{
					read_status ( stats_buffer, dir_entry->d_name );
					if ( user_filter ( stats_buffer ) )
					{
						close ( fd );
						goto next_iteration;
					}
					/*
					allocate space for the stats in the hash table, also
					store the pointer in the array, to be used for sorting
					*/
					current = get_record ( stats_buffer->pid );
					stats_array[c] = current;
					c++;
					if ( c > allocated )
					{
						allocated *= 2;
						stats_array = realloc ( stats_array, allocated * sizeof ( ppstat ) );
					}

					/*
					copy the last number of ticks used to the utime/stime_lastpass element
					so we can use it to calculate the CPU usage percentage
					*/
					stats_buffer->utime_lastpass = current->utime;
					stats_buffer->stime_lastpass = current->stime;
					if ( parametres.ticks_passed > 0 )
					{
						stats_buffer->cpu_percent = (((stats_buffer->utime + stats_buffer->stime ) - (stats_buffer->utime_lastpass + stats_buffer->stime_lastpass )) / parametres.ticks_passed ) * 100;
						stats_buffer->system_cpu_percent = ((stats_buffer->stime - stats_buffer->stime_lastpass ) / parametres.ticks_passed ) * 100;
					}
					/*
					else
					{
						stats_buffer->cpu_percent = 0;
					}
					*/
					stats_buffer->majflt_delta = stats_buffer->majflt - current->majflt;
					stats_buffer->state = state[0];
					stats_buffer->sequence = sequence;
					stats_buffer->next = current->next;
					if ( parametres.requested_fields & 1 << CPUSET )
					{
						read_cpuset ( stats_buffer, dir_entry->d_name );
					}
					memcpy ( current, stats_buffer, sizeof ( pstat ) );
					//read_smaps ( current, dir_entry->d_name );
					//read_status ( current, dir_entry->d_name );
				}
				close ( fd );
			}
		next_iteration: ; /* gcc requires a statement after label, thus ; */
		}

		/* this removes old entries */
		clean_up ( sequence );
	
		qsort ( stats_array, c, sizeof ( pstat * ), compare_elements );

		if ( parametres.curses == 1 )
		{
			print_it ( stats_array, c );
		}
		else if ( parametres.curses == 0 && sequence > 0 )
		{
			print_it_nocurses ( stats_array, c );
			break;
		}
		halfdelay ( 30 );

		//break;
		if ( sequence > 0 )
		{
			//alarm ( 3 );
			//sigwait ( &sigset, &signal );
		}
		else
		{
			usleep ( 400 );
		}
		sequence++;
	}
	endwin ();
	return ( 0 );
}

int process_filter ( ppstat stats_buffer )
{
	int i;
	int retval = 0;

	for ( i = 0; blacklist[i] != NULL; i++ )
	{
		if ( ! strncmp ( stats_buffer->name, blacklist[i], strlen ( blacklist[i] ) ) )
		{
#ifdef DEBUG
			fprintf ( stderr, "name %s blacklisted, skipping...\n", blacklist[i] );
#endif					
			retval = 1;
		}
	}
	return ( retval );
}

int user_filter ( ppstat stats_buffer )
{
	int retval = 0;
	if ( parametres.restrict_to_uid >= 0 )
	{
		if ( stats_buffer->uid != parametres.restrict_to_uid )
		{
			retval = 1;
		}
	}
	if ( parametres.restrict_to_gid >= 0 )
	{
		if ( stats_buffer->gid != parametres.restrict_to_gid )
		{
			retval = 1;
		}
	}
	return ( retval );
}

ppstat get_record ( int pid )
{
	ppstat current = NULL;
	ppstat prev = NULL;
	int key = pid % HASH_TABLE_SIZE;
	if ( stats[key] == NULL )
	{
		stats[key] = malloc ( sizeof ( pstat ) );
		//memset ( stats[key]->swap, 0, sizeof ( int ) * 5 );
		memset ( stats[key], '\0', sizeof ( pstat ) );
		stats[key]->next = NULL;
		return stats[key];
	}
	else
	{
		current = stats[key];
		while ( current != NULL )
		{
			if ( current->pid == pid )
			{
				return current;
			}
			prev = current;
			current = current->next;
		}
		prev->next = malloc ( sizeof ( pstat ) );
		current = prev->next;
		memset ( current, '\0', sizeof ( pstat ) );
		current->next = NULL;
	}
	return ( current );
}

int clean_up ( int sequence )
{
	ppstat current = NULL;
	ppstat prev = NULL;
	int i;
	int c = 0;
	for ( i = 0; i < HASH_TABLE_SIZE; i++ )
	{
#ifdef DEBUG_CLEANUP
		fprintf ( stderr, "%d:\n\t", i );
#endif
		current = stats[i];
		while ( current != NULL )
		{
#ifdef DEBUG_CLEANUP
			if ( i != current->pid % 1000 )
			{
				fprintf ( stderr, "[%d] %d (%d)\n, ", i, current->pid, current->pid % 1000 );
			}
#endif
			if ( current->sequence != sequence )
			{
				if ( current == stats[i] )
				{
					stats[i] = current->next;
					free ( current );
					current = stats[i];
				}
				else
				{
					prev->next = current->next;
					free ( current );
					current = prev->next;
				}
				c++;
			}
			else
			{
				prev = current;
				current = current->next;
			}
		}
#ifdef DEBUG_CLEANUP
		fprintf ( stderr, "\n" );
#endif
	}
	return c;
}

int compare_elements ( const void *first, const void *second )
{
	struct pidstat *one = * (struct pidstat **)first;
	struct pidstat *two = * (struct pidstat **)second;
	int retval = 0;
	switch ( parametres.sortby )
	{
		case PID:
			if ( one->pid > two->pid )
			{
				retval = -1;
			}
			else if ( one->pid < two->pid )
			{
				retval = 1;
			}
			break;
		case SWAP:
			if ( one->swap[0] > two->swap[0] )
			{
				retval = -1;
			}
			else if ( one->swap[0] < two->swap[0] )
			{
				retval = 1;
			}
			break;
		case VIRT:
			if ( one->virt > two->virt )
			{
				retval = -1;
			}
			else if ( one->virt < two->virt )
			{
				retval = 1;
			}
			break;
		case RES:
			if ( one->res > two->res )
			{
				retval = -1;
			}
			else if ( one->res < two->res )
			{
				retval = 1;
			}
			break;
		case MINFLT:
			if ( one->minflt > two->minflt )
			{
				retval = -1;
			}
			else if ( one->minflt < two->minflt )
			{
				retval = 1;
			}
			break;
		case MAJFLT:
			if ( one->majflt > two->majflt )
			{
				retval = -1;
			}
			else if ( one->majflt < two->majflt )
			{
				retval = 1;
			}
			break;
		case MAJFLT_DELTA:
			if ( one->majflt_delta > two->majflt_delta )
			{
				retval = -1;
			}
			else if ( one->majflt_delta < two->majflt_delta )
			{
				retval = 1;
			}
			break;
		case SYS:
			if ( one->system_cpu_percent > two->system_cpu_percent )
			{
				retval = -1;
			}
			else if ( one->system_cpu_percent < two->system_cpu_percent )
			{
				retval = 1;
			}
			break;
		case MEM:
			if ( one->res > two->res )
			{
				retval = -1;
			}
			else if ( one->res < two->res )
			{
				retval = 1;
			}
			break;
		case NTHR:
			if ( one->num_threads > two->num_threads )
			{
				retval = -1;
			}
			else if ( one->num_threads < two->num_threads )
			{
				retval = 1;
			}
			break;
		default:
			if ( one->cpu_percent > two->cpu_percent )
			{
				retval = -1;
			}
			else if ( one->cpu_percent < two->cpu_percent )
			{
				retval = 1;
			}
			break;
	}
	if ( parametres.reversesort == 1 )
	{
		retval *= -1;
	}
	return ( retval );
}

int save_config ( void )
{
	char *home;
	char conffile[256];
	int retval=0;
	FILE *fp;
	int i;
	home = getenv ( "HOME" );
	strcpy ( conffile, home );
	strcat ( conffile, "/." );
	strcat ( conffile, parametres.progname );
	strcat ( conffile, "rc");
	if ( ( fp = fopen ( conffile, "w" ) ) < 0 )
	{
		retval = -1;
	}
	else
	{
		fprintf ( fp, "#AUTOMATICALLY GENERATED by jkutop (called as %s), careful when editing\n", parametres.progname );
		fprintf ( fp, "#The parser is not very robust, so if you edit this manually, better do it\n" );
		fprintf ( fp, "#right. If you screw up (or to go back to defaults) delete this file\n" );
		fprintf ( fp, "%s ", "Fields:" );
		for ( i = 0; i < MAX_DISPLAY_FIELDS; i++ )
		{
			if ( display_fields[i] == NULL )
			{
				break;
			}
			fprintf ( fp, "%s ", display_fields[i]->fieldname );
		}
		fprintf ( fp, "\n" );
	}
	if ( parametres.sortby != CPU || parametres.reversesort != 0 )
	{
		fprintf ( fp, "%s ", "Sort:" );
		fprintf ( fp, "%s ", fields[parametres.sortby].fieldname );
		if ( parametres.reversesort > 0 )
		{
			fprintf ( fp, "asc" );
		}
		fprintf ( fp, "\n" );
	}
		
	fclose ( fp );
	return ( retval );
}

void init_fields ( void )
{
	char buffer[BUFFSIZE];
	char *eolp=NULL;
	char *bolp=NULL;
	ssize_t read_chars=0;
	int restsize = 0;
	int fd;
	char conffile[256];
	char sort[256];
	char order[256]="";
	char conffile_fields[MAX_DISPLAY_FIELDS][256];

	memset ( conffile_fields, 0, MAX_DISPLAY_FIELDS * 256 );

	char *home;
	int i, j=0, fieldindex=0;
	int got_config_from_conffile = 0;
	//extern repr fields[];

	parametres.sortby = CPU;
	parametres.reversesort = 0;
	parametres.requested_fields = 0;
	parametres.restrict_to_uid = -1337;
	parametres.restrict_to_gid = -81;
	parametres.hertz = sysconf ( _SC_CLK_TCK );
	parametres.curses = 1;
	parametres.restrict_to_cpuset = 0;
	memset ( parametres.requested_cpuset, 0, CPUSET_NAME_LENGTH_MAX );

	home = getenv ( "HOME" );
	strcpy ( conffile, home );
	strcat ( conffile, "/." );
	strcat ( conffile, parametres.progname );
	strcat ( conffile, "rc");
	if ( ( fd = open ( conffile, O_RDONLY ) ) > 0  )
	{
		do
		{
			read_chars = read ( fd, buffer + restsize, sizeof ( buffer ) - restsize );
			bolp = buffer;
			while ( ( eolp = memchr ( bolp, '\n', BUFFSIZE - ( bolp - buffer ) ) ) != NULL)
			{
				eolp[0] = '\0';
				eolp++;
				if ( ! strncmp ( bolp, "#", 1 ) )
				{
					bolp = eolp;
					continue;
				}
				if ( ! strncmp ( bolp, "Fields:", 7 ) )
				{
					sscanf ( bolp, "Fields: %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s %255s", 
					conffile_fields[0],
					conffile_fields[1],
					conffile_fields[2],
					conffile_fields[3],
					conffile_fields[4],
					conffile_fields[5],
					conffile_fields[6],
					conffile_fields[7],
					conffile_fields[8],
					conffile_fields[9],
					conffile_fields[10],
					conffile_fields[11],
					conffile_fields[12],
					conffile_fields[13],
					conffile_fields[14],
					conffile_fields[15],
					conffile_fields[16],
					conffile_fields[17],
					conffile_fields[18],
					conffile_fields[19]
					 );
					got_config_from_conffile = 1;
				}
				if ( ! strncmp ( bolp, "Sort:", 5 ) )
				{
					sscanf ( bolp, "Sort: %s %s", sort, order );
				}
				bolp = eolp;
			}
			/*
			move the rest of the string to the beginning
			*/
			restsize = BUFFSIZE - ( bolp - buffer );
			memmove ( buffer, bolp, restsize );
		}
		while ( read_chars > 0 );
	
		close ( fd );
		while ( strcmp ( conffile_fields[j], "") )
		{
			for ( i = 0; i < FIELDS_AVAILABLE; i++ )
			{
				if ( ! strcmp ( conffile_fields[j], fields[i].fieldname ) )
				{
					display_fields[fieldindex] = &fields[i];
					parametres.requested_fields |= 1 << i;
					fieldindex++;
				}
			}
			j++;
			if ( j == MAX_DISPLAY_FIELDS )
			{
				break;
			}
		}
		for ( i = 0; i < FIELDS_AVAILABLE; i++ )
		{
			if ( ! strcmp ( sort, fields[i].fieldname ) )
			{
				parametres.sortby = i;
			}
		}
		if ( ! strcmp ( order, "asc" ) )
		{
			parametres.reversesort = 1;
		}

		//exit ( 0 );

	}

	if ( got_config_from_conffile == 0 )
	{
		/* initialise display fields */
		display_fields[0] = &fields[PID];
		display_fields[1] = &fields[USER];
		display_fields[2] = &fields[PR];
		display_fields[3] = &fields[NI];
		display_fields[4] = &fields[VIRT];
		display_fields[5] = &fields[RES];
		display_fields[6] = &fields[S];
		display_fields[7] = &fields[CPU];
		display_fields[8] = &fields[MEM];
		display_fields[9] = &fields[TIME];
		display_fields[10] = &fields[COMMAND];
		display_fields[12] = NULL;
	}
}

void get_my_name ( char *argv )
{
	char *offset;
	if ( ( offset = strrchr ( argv, '/' ) ) != NULL )
	{
		strncpy ( parametres.progname, offset + 1, 256 );
	}
	else
	{
		strncpy ( parametres.progname, argv, 256 );
	}
}

int get_uid ( char *user )
{
	int i;
	struct passwd	*pwent;
	int retval = -1;
	for ( i = 0; i < 256; i++ )
	{
		if ( user[i] == '\0' )
		{
			/* all digits until end of string, assume parametre passed
			 * is an UID and return that
			 */
			retval = atoi ( user );
			break;
		}
		if ( ! isdigit ( user[i] ) )
		{
			/* 
			 * this seems not to be an UID, try handling it
			 * as an username
			 */
			 if ( ( pwent = getpwnam ( user ) ) != NULL )
			 {
				 retval = pwent->pw_uid;
				 break;
			 }
			 else
			 {
				 /* parametre passed seems not to be a user name, giving
				  * up
				  */
				  break;
			 }
		}
	}
	return ( retval );
}

int get_gid ( char *group )
{
	int i;
	struct group	*grent;
	int retval = -1;
	for ( i = 0; i < 256; i++ )
	{
		if ( group[i] == '\0' )
		{
			/* all digits until end of string, assume parametre passed
			 * is an UID and return that
			 */
			retval = atoi ( group );
			break;
		}
		if ( ! isdigit ( group[i] ) )
		{
			/* 
			 * this seems not to be an UID, try handling it
			 * as an groupname
			 */
			 if ( ( grent = getgrnam ( group ) ) != NULL )
			 {
				 retval = grent->gr_gid;
				 break;
			 }
			 else
			 {
				 /* parametre passed seems not to be a group name, giving
				  * up
				  */
				  break;
			 }
		}
	}
	return ( retval );
}
