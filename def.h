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
	int		updated; /* indication of activity, see comment below */
	struct pidstat *next;
}pstat;

/*
the "updated" element is flicked between 0 and 1 at each pass and is used
to weed out the entries which have not been updated in the last pass
(which means that the process went away)
*/

int compare_elements ( const void *first, const void *second );
int process_filter ( const char *execname );
int read_status ( pstat *stats, char *pid );
