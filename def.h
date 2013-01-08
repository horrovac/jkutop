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
