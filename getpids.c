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

#define BUFFSIZE 1024
#define ALLOC_NUM 40000
#define	HASH_TABLE_SIZE 1000

const char *blacklist[] = { "ksoftirqd/", "migration/", "events/", "kintegrityd/", "kblockd/", "kstop/", "kondemand/", "kswapd", "aio/", "crypto/", "ata/", "xfslogd/", "xfsdatad/", "xfsconvertd/", "rpciod/", "kworker", NULL };
ppstat stats[HASH_TABLE_SIZE];

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



int main ( void )
{
	DIR				*dirp;
	struct dirent	*dir_entry;
	int fd;
	char path[1024];
	char buffer[BUFFSIZE];
	char state[1];
	int i;
	int c=0;
	int pid;
	int sequence=0;
	int chars_read;
	int user_jiffies;
	int kernel_jiffies;
	pstat *current = NULL;
	pstat *temp = NULL;
	/*
	int blacklist_len[] = { 10, 10, 7, 12, 8, 6, 10, 7, 4, 7, 4, 8, 9, 12, 7, 7, 0 };
	stats = malloc ( sizeof ( pstat * ) * ALLOC_NUM );
	memset ( stats, '\0', sizeof ( pstat * ) * ALLOC_NUM );
	*/


	dirp = opendir ( "/proc" );

	while ( 1 )
	{
		rewinddir ( dirp );
		while ( ( dir_entry = readdir ( dirp ) ) != NULL ) 
		{
			for ( i = 0; dir_entry->d_name[i] != '\0'; i++ )
			{
				if ( ! isdigit ( dir_entry->d_name[i] ) )
				{
	#ifdef DEBUG
					fprintf ( stderr, "entry %s not a pid, skipping...\n", dir_entry->d_name );
	#endif
					goto next_iteration;
				}
			}
			memset ( path, '\0', 1024 );
			strcat ( path, "/proc/" );
			strcat ( path, dir_entry->d_name );
			strcat ( path, "/stat" );
	#ifdef DEBUG
			fprintf ( stderr, "path: %s\n", path );
	#endif
			if ( (fd = open ( path, O_RDONLY )) )
			{
				if ( ( chars_read = read ( fd, buffer, BUFFSIZE ) ) < 0 )
				{
					fprintf ( stderr, "Can't read file %s\n", path );
					close ( fd );
					continue;
				}
				pid = atoi ( dir_entry->d_name );
				current = get_record ( pid );
				/*
				if ( sequence )
				{
					current = stats;
					while ( current->pid != pid )
					{
						if ( current->next == NULL )
						{
							current->next = malloc ( sizeof ( pstat ) );
							current = current->next;
							memset ( current->swap, '\0', sizeof ( int ) * KEEPRECORDS );
							current->next = NULL;
							break;
						}
						else
						{
							current = current->next;
						}
					}
				}
				else
				{
					current->next = malloc ( sizeof ( pstat ) );
					current = current->next;
					memset ( current->swap, '\0', sizeof ( int ) * KEEPRECORDS );
					current->next = NULL;
				}
				*/
				buffer[chars_read+1] = '\0';
				sscanf ( buffer, "%d (%[^)]) %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d", &current->pid, current->name, state, &current->ppid, &user_jiffies, &kernel_jiffies );
				current->state = state[0];
				current->user = user_jiffies / HZ;
				current->kernel = kernel_jiffies / HZ;
				current->sequence = sequence;
				read_status ( current, dir_entry->d_name );
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
		remove entries from exited jobs
		fprintf ( stderr, "hoy!\n" );
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

	
	//	qsort ( stats, c, sizeof ( pstat * ), compare_elements );
	
		/*
		print it out
		printf ( "%-15s %8s %8s %10s %10s %10s %5s\n", "CMD", "SWAP (kB)", "PID", "PPID", "USER", "SYSTEM", "STATE" );
		*/
	
		c = 0;
		for ( i = 0; i < HASH_TABLE_SIZE; i++ )
		{
			current = stats[i];
			while ( current != NULL )
			{
				/*
				if ( current->swap[0] )
				*/
				{
					printf ( "%-15s %8d %8d %10d %10.2f %10.2f %5c\n", current->name, current->swap[0], current->pid, current->ppid, current->user, current->kernel, current->state );
					//printf ( "%-15s %8d %8d %8d %8d %8d %8d\n", current->name, current->swap[0], current->swap[1], current->swap[2], current->swap[3], current->swap[4], current->swapchange );
				}
				c++;
				current = current->next;
			}
		}
		printf ( "%d records (seq %d)\n", c, sequence );
		sequence++;
		//break;
		sleep ( 1 );
	}
	return ( 0 );
}

int compare_elements ( const void *first, const void *second )
{
	struct pidstat *one = * (struct pidstat **)first;
	struct pidstat *two = * (struct pidstat **)second;
	int retval = 0;
	/*
	fprintf ( stderr, "comparing %d and %d\n", one->swapsize, two->swapsize );
	*/
	if ( one->swap > two->swap )
	{
		retval = 1;
	}
	else if ( one->swap < two->swap )
	{
		retval = -1;
	}
	return ( retval );
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




/*
23205 (pids) R 20958 23205 20958 34829 23205 4202496 205 0 0 0 0 0 0 0 20 0 1 0 835953 4308992 85 18446744073709551615 4194304 4197084 140735530590432 140735530577624 140596710277696 0 0 128 0 0 0 0 17 0 0 0 0 0 0 6294240 6294856 17702912 140735530596500 140735530596507 140735530596507 140735530598385 0
*/
