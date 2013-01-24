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

//#define BUFFSIZE 1024


int read_status ( pstat *stats, char *pid )
{
	char buffer[BUFFSIZE];
	char path[256];
	char *eolp=NULL;
	char *bolp=NULL;
	ssize_t read_chars=0;
	int fd;
	int s;
	int swap=0;
	int restsize = 0;

	/*
	fd = open ( argv[1], O_RDONLY );
	*/
	memset ( path, '\0', sizeof ( path ) );
	strcat ( path, "/proc/" );
	strcat ( path, pid );
	strcat ( path, "/status" );
	fd = open ( path, O_RDONLY );

	/*
	functionality moved to read_smaps()
	for ( i = KEEPRECORDS - 1; i > 0; i-- )
	{
		stats->swap[i] = stats->swap[i-1];
	}
	*/

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
				break;
			}
			/*
			functionality moved to read_smaps()
			...and then back here again, smaps is rubbish.
			*/
			if ( ! strncmp ( bolp, "VmSw", 4 ) )
			{
				sscanf ( bolp, "VmSwap:                 %d kB", &s );
				stats->swap[0] = s;
				stats->swapchange = stats->swap[KEEPRECORDS-1] - stats->swap[0];
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

