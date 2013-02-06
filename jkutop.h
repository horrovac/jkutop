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
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#define KEEPRECORDS 5
#define BUFFSIZE 20480
#define ALLOC_CHUNK 1000
#define	HASH_TABLE_SIZE 1000

enum
{
	PID = 0,
	PPID,
	USER,
	UID,
	PR,
	NI,
	VIRT,
	RES,
	S,
	CPU,
	MEM,
	SWAP,
	TIME,
	COMMAND,
	MINFLT,
	MAJFLT,
	FIELDS_AVAILABLE	// not an identifier, # identifiers available
};

typedef struct pidstat
{
	char				name[256];
	char				state;
	int					uid;
	int					euid;
	int					pid;
	int					ppid;
	unsigned long long	majflt;
	unsigned long long	minflt;
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

typedef struct cpustats
{
	unsigned long long	user;
	unsigned long long	nice;
	unsigned long long	system;
	unsigned long long	idle;
	unsigned long long	iowait;
	unsigned long long	irq;
	unsigned long long	softirq;
	unsigned long long	steal;
}cstats, *pcstats;

typedef struct meminfo
{
	unsigned long	memtotal;
	unsigned long	memfree;
	unsigned long	buffers;
	unsigned long	cached;
	unsigned long	swaptotal;
	unsigned long	swapfree;
}mstat, *pmstat;

typedef struct representation
{
	int		identifier;
	char	format[20];
	char	format_alt[20];
	char	header_format[20];
	int		field_length;
	char	fieldname[20];
	void	(*printout)(ppstat entry, int identifier);
}repr, *prepr;

typedef struct parametres
{
	int		sortby;
	int		reversesort;
}params;

int compare_elements ( const void *first, const void *second );
int process_filter ( const char *execname );
int read_status ( pstat *stats, char *pid );
int read_smaps ( pstat *stats, char *pid );
int sort_entries ( void );
int print_it ( ppstat *stats_array, int count );
int clean_up ( int sequence );
int read_meminfo ( mstat *meminfo );
void init_fields ( void );
ppstat get_record (	int pid );
void print_pid ( ppstat entry, int identifier );
void print_ppid ( ppstat entry, int identifier );
void print_user ( ppstat entry, int identifier );
void print_uid ( ppstat entry, int identifier );
void print_priority ( ppstat entry, int identifier );
void print_niceness ( ppstat entry, int identifier );
void print_virt ( ppstat entry, int identifier );
void print_res ( ppstat entry, int identifier );
void print_status ( ppstat entry, int identifier );
void print_cpu_percent ( ppstat entry, int identifier );
void print_mem_percent ( ppstat entry, int identifier );
void print_swap ( ppstat entry, int identifier );
void print_time ( ppstat entry, int identifier );
void print_name ( ppstat entry, int identifier );
void print_minflt ( ppstat entry, int identifier );
void print_majflt ( ppstat entry, int identifier );

WINDOW *win;
int row, col;
params parametres;
prepr display_fields[20];
