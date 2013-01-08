#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "def.h"

#define BUFFSIZE 1024


int read_meminfo ( mstat *meminfo )
{
	char buffer[BUFFSIZE];
	char *eolp=NULL;
	char *bolp=NULL;
	ssize_t read_chars=0;
	int fd;
	int restsize = 0;

	fd = open ( "/proc/meminfo", O_RDONLY );

	do
	{
		read_chars = read ( fd, buffer + restsize, sizeof ( buffer ) - restsize );
		bolp = buffer;
		while ( ( eolp = memchr ( bolp, '\n', BUFFSIZE - ( bolp - buffer ) ) ) != NULL)
		{
			eolp[0] = '\0';
			eolp++;
			if ( ! strncmp ( bolp, "MemTotal:", 9 ) )
			{
				sscanf ( bolp, "MemTotal:        %lu kB", &meminfo->memtotal );
			}
			if ( ! strncmp ( bolp, "MemFree:", 8 ) )
			{
				sscanf ( bolp, "MemFree:        %lu kB", &meminfo->memfree );
			}
			if ( ! strncmp ( bolp, "Buffers:", 8 ) )
			{
				sscanf ( bolp, "Buffers:        %lu kB", &meminfo->buffers );
			}
			if ( ! strncmp ( bolp, "Cached:", 7 ) )
			{
				sscanf ( bolp, "Cached:        %lu kB", &meminfo->cached );
			}
			if ( ! strncmp ( bolp, "SwapTotal:", 10 ) )
			{
				sscanf ( bolp, "SwapTotal:        %lu kB", &meminfo->swaptotal );
			}
			if ( ! strncmp ( bolp, "SwapFree:", 9 ) )
			{
				sscanf ( bolp, "SwapFree:        %lu kB", &meminfo->swapfree );
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

	close ( fd );
	return ( 0 );
}

