#define KEEPRECORDS 5
#define BUFFSIZE 1024
#define ALLOC_CHUNK 1000
#define	HASH_TABLE_SIZE 1000

typedef struct pidstat
{
	char			name[256];
	char			state;
	int				uid;
	int				euid;
	int				pid;
	int				ppid;
	unsigned long	utime;
	unsigned long	stime;
	unsigned long	cutime;
	unsigned long	cstime;
	long			priority;
	long			niceness;
	unsigned long	virt;
	long			res;
	int				swap[KEEPRECORDS];
	int				swapchange;
	int				sequence; /* used to recognise old entries for removal */
	struct pidstat	*next;
}pstat, *ppstat;

int compare_elements ( const void *first, const void *second );
int process_filter ( const char *execname );
int read_status ( pstat *stats, char *pid );
int sort_entries ( void );
int print_it ( ppstat *stats_array, int count );
ppstat get_record (	int pid );
