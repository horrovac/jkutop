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
#include "jkutop.h"


int read_smaps ( pstat *stats, char *pid )
{
	char buffer[BUFFSIZE];
	char path[256];
	char *eolp=NULL;
	char *bolp=NULL;
	ssize_t read_chars=0;
	int fd;
	int i;
	int swap=0, s=0;
	int restsize = 0;

	/*
	fd = open ( argv[1], O_RDONLY );
	*/
	memset ( path, '\0', sizeof ( path ) );
	strcat ( path, "/proc/" );
	strcat ( path, pid );
	strcat ( path, "/smaps" );
	fd = open ( path, O_RDONLY );

	for ( i = KEEPRECORDS - 1; i > 0; i-- )
	{
		stats->swap[i] = stats->swap[i-1];
	}

	do
	{
		read_chars = read ( fd, buffer + restsize, sizeof ( buffer ) - restsize );
		bolp = buffer;
		while ( ( eolp = memchr ( bolp, '\n', BUFFSIZE - ( bolp - buffer ) ) ) != NULL)
		{
			eolp[0] = '\0';
			eolp++;
			/*
			if ( bolp[0] == 'S' && bolp[1] == 'w' )
			{
				*/
				sscanf ( bolp, "Swap:                 %d kB", &s );
				swap += s;
				/*
				fprintf ( stderr, "line: %s (%d, total: %d)\n", bolp, s, swap );
			}
				*/
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
	stats->swap[0] = swap;

	close ( fd );
	return ( swap );
}

