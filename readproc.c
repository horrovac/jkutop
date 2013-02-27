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

int read_smaps ( pstat *stats, char *pid )
{
	char buffer[BUFFSIZE];
	char path[256] = "/proc/";
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

int read_cpuset ( pstat *stats, char *pid )
{
	int fd;
	int read_chars;
	int retval = 0;
	char buffer[BUFFSIZE];
	char path[256] = "/proc/";

	strcat ( path, pid );
	strcat ( path, "/cpuset" );

	fd = open ( path, O_RDONLY );

	if ( fd > 0 )
	{
		read_chars = read ( fd, buffer, sizeof ( buffer ) );
		buffer[read_chars-1] = '\0'; /* erase teh newline */
		strncpy ( stats->cpuset, buffer, CPUSET_NAME_LENGTH_MAX );
		close ( fd );
	}
	else
	{
		retval = fd;
	}
	return ( retval );
}


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

