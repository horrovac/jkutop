#include <stdlib.h>
#include "def.h"

int sort_entries ( void )
{
	return 0;
}

int compare_elements ( const void *first, const void *second )
{
	struct pidstat *one = * (struct pidstat **)first;
	struct pidstat *two = * (struct pidstat **)second;
	int retval = 0;
	/*
	if ( one->swap[0] > two->swap[0] )
	{
		retval = 1;
	}
	else if ( one->swap[0] < two->swap[0] )
	{
		retval = -1;
	}
	return ( retval );
	*/
	if ( one->pid > two->pid )
	{
		retval = -1;
	}
	else if ( one->pid < two->pid )
	{
		retval = 1;
	}
	return ( retval );
}

