#define KEEPRECORDS 5

typedef struct pidstat
{
	char name[256];
	char state;
	int		uid;
	int		euid;
	int		pid;
	int		ppid;
	float	user;
	float	kernel;
	int		swap[KEEPRECORDS];
	int		swapchange;
	int		sequence; /* used to recognise old entries for removal */
	struct pidstat *next;
}pstat;

int compare_elements ( const void *first, const void *second );
int process_filter ( const char *execname );
int read_status ( pstat *stats, char *pid );
