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
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define KEEPRECORDS 5
#define BUFFSIZE 20480
#define ALLOC_CHUNK 1000
#define	HASH_TABLE_SIZE 1000
#define MAX_DISPLAY_FIELDS 20
#define CPUSET_NAME_LENGTH_MAX 256

enum
{
	ERR_OPTION=1
};

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
	MAJFLT_DELTA,
	SYS,
	CPUSET,
	NTHR,
	FIELDS_AVAILABLE	// not an identifier, # identifiers available
};

typedef struct pidstat
{
	int					pid;
	char				name[256];
	char				state;
	int					ppid;
	int					pgrp;
	int					session;
	int					tty_nr;
	int					uid;
	int					euid;
	int					gid;
	int					egid;
	unsigned long long	majflt;
	int					majflt_delta;
	unsigned long long	minflt;
	unsigned long long	utime;
	unsigned long long	stime;
	unsigned long long	utime_lastpass;
	unsigned long long	stime_lastpass;
	unsigned long long	cutime;
	unsigned long long	cstime;
	double				cpu_percent;
	double				system_cpu_percent;
	long				num_threads;
	unsigned long long	starttime;
	long				priority;
	long				niceness;
	unsigned long		virt;
	long				res;
	int					swap[KEEPRECORDS];
	int					swapchange;
	char				cpuset[CPUSET_NAME_LENGTH_MAX];
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
	int		sortable;
	char	fieldname[20];
	void	(*printout)(ppstat entry, int identifier);
}repr, *prepr;

typedef struct parametres
{
	int		sortby;
	int		reversesort;
	long	requested_fields; /*bitmask, read comment below*/
	char	requested_user[256];
	char	requested_group[256];
	char	requested_cpuset[CPUSET_NAME_LENGTH_MAX];
	int		restrict_to_uid; /* show only processes of uid */
	int		restrict_to_gid; /* show only processes of gid */
	int		restrict_to_cpuset; /* boolean */
	int		hertz;	/* system clock frequency */
	cstats	cpu_stats[2];
	unsigned long long	ticks_lastpass;
	double	ticks_passed;	/* ticks passed between updates */
	char	progname[256];
	time_t	btime;
	float	loadavg[3];
	int 	curses;
}params;

/*
 requested_fields is a bitmask built from field identifiers,
 it enables us to avoid reading data in case their display is
 not requested, thus speeding up the execution. The value are
 1's bitshifted by numerical value of the identifier, OR'ed
 together. Adding "COMMAND" for instance:
 paramtres.requested_fields |= 1 << COMMAND;
 ...to find out if we're displaying this:
 if ( ! parametres.requested_fields & 1 << COMMAND ) { ... }
 */

/* defined in jkutop.c */
int compare_elements ( const void *first, const void *second );
int process_filter ( ppstat stats_buffer );
int user_filter ( ppstat stats_buffer );
int clean_up ( int sequence );
void get_my_name ( char *argv );
ppstat get_record (	int pid );
void init_fields ( void );
int get_uid ( char *user );
int get_gid ( char *group );

/* defined in readproc.c */
int read_status ( pstat *stats, char *pid );
int read_smaps ( pstat *stats, char *pid );
int sort_entries ( void );
int read_proc_stat ( int procstat );
int read_btime ( int procstat );
int read_loadavg ( int loadavg );
int read_meminfo ( mstat *meminfo );
int read_cpuset ( pstat *stats, char *pid );

/* defined in printing.c */
int print_it ( ppstat *stats_array, int count );
/* defined in ps_printing.c */
int print_it_nocurses ( ppstat *stats_array, int count );
int save_config ( void );
prepr select_field ( int y, int x, prepr current );
void modify_display ( void );
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
void print_majflt_delta ( ppstat entry, int identifier );
void print_system_cpu_percent ( ppstat entry, int identifier );
void print_cpuset ( ppstat entry, int identifier );
void print_nthr ( ppstat entry, int identifier );
void show_help ( void );
int show_process_detail ( ppstat *stats_array, int member );
int mouse_select_sortfield ( int x );

/* pointer to be defined to printf or printw depending on the
 * art of the output (curses or not)
 */
int (*output)(const char *format, ...);

WINDOW *win;
int row, col;
params parametres;
prepr display_fields[MAX_DISPLAY_FIELDS];
