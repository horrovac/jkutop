#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "def.h"

#define BUFFSIZE 1024


int read_status ( pstat *stats, char *pid )
{
	char buffer[BUFFSIZE];
	char path[256];
	char *eolp=NULL;
	char *bolp=NULL;
	ssize_t read_chars=0;
	int fd;
	int swap=0, s=0;
	int restsize = 0;

	/*
	fd = open ( argv[1], O_RDONLY );
	*/
	memset ( path, '\0', sizeof ( path ) );
	strcat ( path, "/proc/" );
	strcat ( path, pid );
	strcat ( path, "/status" );
	fd = open ( path, O_RDONLY );

	do
	{
		read_chars = read ( fd, buffer + restsize, sizeof ( buffer ) - restsize );
		bolp = buffer;
		while ( ( eolp = memchr ( bolp, '\n', BUFFSIZE - ( bolp - buffer ) ) ) != NULL)
		{
			eolp[0] = '\0';
			eolp++;
			if ( ! strncmp ( bolp, "Uid:", 4 ) )
			{
				sscanf ( bolp, "Uid: %d %d %*d %*d", &stats->uid, &stats->euid );

			}
			if ( ! strncmp ( bolp, "VmSw", 4 ) )
			{
				sscanf ( bolp, "VmSwap:                 %d kB", &s );
				stats->swap = s;
				break;
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
	/*
	fprintf ( stderr, "total swap: %d\n", swap );
	*/

	close ( fd );
	return ( swap );
}

