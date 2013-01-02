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

const char *blacklist[] = { "ksoftirqd/", "migration/", "events/", "kintegrityd/", "kblockd/", "kstop/", "kondemand/", "kswapd", "aio/", "crypto/", "ata/", "xfslogd/", "xfsdatad/", "xfsconvertd/", "rpciod/", "kworker", NULL };
ppstat stats[HASH_TABLE_SIZE];
ppstat *stats_array = NULL;

int main ( void )
{
	DIR				*dirp;
	struct dirent	*dir_entry;
	int				fd;
	char			path[1024];
	char			buffer[BUFFSIZE];
	char			state[1];
	int				i;
	int				allocated=1000;
	int				c=0;
	int				sequence=0;
	int				chars_read;
	pstat			*current = NULL;
	pstat			*temp = NULL;
	pstat			*stats_buffer;

	stats_array = malloc ( allocated * sizeof ( ppstat ) );

	/*
	store stats here temporarily. This is for blacklisting, if the records
	are not desired, they'll be dropped, if not, they'll be memcpy()ed
	into the data. This saves allocating memory for this only to drop it in
	case the record is blacklisted
	*/
	stats_buffer = malloc ( sizeof ( pstat ) );
	
	dirp = opendir ( "/proc" );

	while ( 1 )
	{
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
					fprintf ( stderr, "Can't read file %s\n", path );
					close ( fd );
					continue;
				}
				buffer[chars_read+1] = '\0';
				sscanf ( buffer, "%d (%[^)]) %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu %ld %ld %ld %ld %*d %*d %*d %lu %ld", &stats_buffer->pid, stats_buffer->name, state, &stats_buffer->ppid, &stats_buffer->utime, &stats_buffer->stime, &stats_buffer->cutime, &stats_buffer->cstime, &stats_buffer->priority, &stats_buffer->niceness, &stats_buffer->virt, &stats_buffer->res );
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

					memcpy ( current, stats_buffer, sizeof ( pstat ) );
					current->state = state[0];
					current->sequence = sequence;
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
		*/
		for ( i = 0; i < HASH_TABLE_SIZE; i++ )
		{
			current = stats[i];
			while ( current != NULL )
			{
				temp = current->next;
				if ( temp != NULL &&  temp->sequence != sequence )
				{
					current->next = temp->next;
					free ( temp );
					current = current->next;
				}
				else
				{
					current = current->next;
				}
			}
		}

	
		qsort ( stats_array, c, sizeof ( pstat * ), compare_elements );

		print_it ( stats_array, c );

		sequence++;
		//break;
		sleep ( 1 );
	}
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
	ppstat current;
	int key = pid % HASH_TABLE_SIZE;
	if ( stats[key] == NULL )
	{
		stats[key] = malloc ( sizeof ( pstat ) );
		memset ( stats[key]->swap, 0, sizeof ( int ) * 5 );
		stats[key]->next = NULL;
		return stats[key];
	}
	else
	{
		current = stats[key];
		while ( current->next != NULL )
		{
			current = current->next;
			if ( current->pid == pid )
			{
				return current;
			}
		}
		current->next = malloc ( sizeof ( pstat ) );
		current = current->next;
		memset ( current->swap, 0, sizeof ( int ) * 5 );
		current->next = NULL;
	}
	return ( current );
}
