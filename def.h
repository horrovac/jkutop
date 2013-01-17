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
#include <sys/time.h>
#include <ncurses.h>

#define KEEPRECORDS 5
#define BUFFSIZE 1024
#define ALLOC_CHUNK 1000
#define	HASH_TABLE_SIZE 1000

typedef struct pidstat
{
	char				name[256];
	char				state;
	int					uid;
	int					euid;
	int					pid;
	int					ppid;
	unsigned long long	utime;
	unsigned long long	stime;
	unsigned long long	utime_lastpass;
	unsigned long long	stime_lastpass;
	unsigned long long	cutime;
	unsigned long long	cstime;
	double				cpu_percent;
	long				priority;
	long				niceness;
	unsigned long		virt;
	long				res;
	int					swap[KEEPRECORDS];
	int					swapchange;
	int					sequence; /* used to recognise old entries for removal */
	struct pidstat		*next;
}pstat, *ppstat;

typedef struct meminfo
{
	unsigned long	memtotal;
	unsigned long	memfree;
	unsigned long	buffers;
	unsigned long	cached;
	unsigned long	swaptotal;
	unsigned long	swapfree;
}mstat, *pmstat;

int compare_elements ( const void *first, const void *second );
int process_filter ( const char *execname );
int read_status ( pstat *stats, char *pid );
int sort_entries ( void );
int print_it ( ppstat *stats_array, int count );
int clean_up ( int sequence );
int read_meminfo ( mstat *meminfo );
ppstat get_record (	int pid );

WINDOW *win;
int row, col;
