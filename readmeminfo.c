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

