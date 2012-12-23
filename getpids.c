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

const char *blacklist[] = { "ksoftirqd/", "migration/", "events/", "kintegrityd/", "kblockd/", "kstop/", "kondemand/", "kswapd", "aio/", "crypto/", "ata/", "xfslogd/", "xfsdatad/", "xfsconvertd/", "rpciod/", "kworker", NULL };


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
	int chars_read;
	int user_jiffies;
	int kernel_jiffies;
	pstat **stats;
	/*
	int blacklist_len[] = { 10, 10, 7, 12, 8, 6, 10, 7, 4, 7, 4, 8, 9, 12, 7, 7, 0 };
	*/

	stats = malloc ( sizeof ( pstat * ) * ALLOC_NUM );
	memset ( stats, '\0', sizeof ( pstat * ) * ALLOC_NUM );


	dirp = opendir ( "/proc" );

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
			buffer[chars_read+1] = '\0';
			stats[c] = malloc ( sizeof ( pstat ) );
			sscanf ( buffer, "%d (%[^)]) %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d", &stats[c]->pid, stats[c]->name, state, &stats[c]->ppid, &user_jiffies, &kernel_jiffies );
			stats[c]->state = state[0];
			stats[c]->user = user_jiffies / HZ;
			stats[c]->kernel = kernel_jiffies / HZ;
			stats[c]->swap = 0;
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
			close ( fd );
		}
	next_iteration: ; /* gcc requires a statement after label, thus ; */
	}

	qsort ( stats, c, sizeof ( pstat * ), compare_elements );

	/*
	print it out
	*/

	printf ( "%-15s %8s %8s %10s %10s %10s %5s\n", "CMD", "SWAP (kB)", "PID", "PPID", "USER", "SYSTEM", "STATE" );
	for ( i = 0; stats[i] != NULL ; i++ )
	{
		//if ( stats[i]->swap )
		{
			printf ( "%-15s %8d %8d %10d %10.2f %10.2f %5c\n", stats[i]->name, stats[i]->swap, stats[i]->pid, stats[i]->ppid, stats[i]->user, stats[i]->kernel, stats[i]->state );
		}
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
