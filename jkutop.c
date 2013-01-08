#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <asm/param.h>		/* neded for HZ definition */
#include "def.h"

#define DEBUG_BUILDING 1
#undef DEBUG_CLEANUP

const char *blacklist[] = { "ksoftirqd/", "migration/", "events/", "kintegrityd/", "kblockd/", "kstop/", "kondemand/", "kswapd", "aio/", "crypto/", "ata/", "xfslogd/", "xfsdatad/", "xfsconvertd/", "rpciod/", "kworker", NULL };
double	ticks_passed;
ppstat stats[HASH_TABLE_SIZE];
ppstat *stats_array = NULL;

int main ( void )
{
	DIR				*dirp;
	struct dirent	*dir_entry;
	int				fd;
	int				procstat;	/* fd for /proc/stat */
	char			path[1024];
	char			buffer[BUFFSIZE];
	char			state[1];
	int				i;
	int				allocated=1000;
	int				c=0;
	int				sequence=0;
	int				chars_read;
	unsigned long long		ticks = 0, ticks_before, user, nice, system, idle;
	pstat			*current = NULL;
	//pstat			*temp = NULL;
	pstat			*stats_buffer;

	initscr(); /* ncurses initialisation */

	stats_array = malloc ( allocated * sizeof ( ppstat ) );

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

	while ( 1 )
	{
		/* get number of ticks passed since last pass */
		lseek ( procstat, 0, SEEK_SET );
		if ( read ( procstat, buffer, 256 ) )
		{
			ticks_before = ticks;
			sscanf ( buffer, "%*s %llu %llu %llu %llu", &user, &nice, &system, &idle );
			ticks = user + nice + system + idle;
			if ( ticks_before != 0 )
			{
				ticks_passed = ( ticks - ticks_before ) / sysconf ( _SC_NPROCESSORS_ONLN );
			}
		}
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
				sscanf ( buffer, "%d (%[^)]) %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %Lu %Lu %Ld %Ld %ld %ld %*d %*d %*d %lu %ld", &stats_buffer->pid, stats_buffer->name, state, &stats_buffer->ppid, &stats_buffer->utime, &stats_buffer->stime, &stats_buffer->cutime, &stats_buffer->cstime, &stats_buffer->priority, &stats_buffer->niceness, &stats_buffer->virt, &stats_buffer->res );
				if ( ! process_filter ( stats_buffer->name ) )
				{
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
					if ( ticks_passed > 0 )
					{
						stats_buffer->cpu_percent = (((stats_buffer->utime + stats_buffer->stime ) - (stats_buffer->utime_lastpass + stats_buffer->stime_lastpass )) / ticks_passed ) * 100;
					}
					/*
					else
					{
						stats_buffer->cpu_percent = 0;
					}
					*/
					stats_buffer->state = state[0];
					stats_buffer->sequence = sequence;
					stats_buffer->next = current->next;
					memcpy ( current, stats_buffer, sizeof ( pstat ) );
					read_status ( current, dir_entry->d_name );
				}
				/*
				if ( ! process_filter ( stats[c]->name ) )
				{
					//stats[c]->swap = read_smaps ( dir_entry->d_name );
					read_status ( stats[c], dir_entry->d_name );
					c++;
				}
				else
				{
					free ( stats[c] );
				}
				*/
				close ( fd );
			}
		next_iteration: ; /* gcc requires a statement after label, thus ; */
		}

		/*
		remove entries from exited jobs (recognisable by an old
		sequence number)
		for ( i = 0; i < HASH_TABLE_SIZE; i++ )
		{
			temp = NULL;
			current = stats[i];
			while ( current != NULL )
			{
				temp = current->next;
				if ( current->sequence != sequence )
				{
					if ( current == stats[i] )
					{
						stats[i] = temp;
					}
					free ( current );
					prev = temp;
					current = temp;
				}
				else
				{
					prev = current;
					current = current->next;
				}
			}
		}
		*/

		clean_up ( sequence );
	
		qsort ( stats_array, c, sizeof ( pstat * ), compare_elements );

		print_it ( stats_array, c );

		sequence++;
		//break;
		sleep ( 1 );
	}
	endwin ();
	return ( 0 );
}

int process_filter ( const char *execname )
{
	int i;
	int retval = 0;

	for ( i = 0; blacklist[i] != NULL; i++ )
	{
		if ( ! strncmp ( execname, blacklist[i], strlen ( blacklist[i] ) ) )
		{
#ifdef DEBUG
			fprintf ( stderr, "name %s blacklisted, skipping...\n", blacklist[i] );
#endif					
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
