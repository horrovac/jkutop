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
#include <stdlib.h>
#include "jkutop.h"

int sort_entries ( void )
{
	return 0;
}

int compare_elements ( const void *first, const void *second )
{
	struct pidstat *one = * (struct pidstat **)first;
	struct pidstat *two = * (struct pidstat **)second;
	int retval = 0;
	switch ( parametres.sortby )
	{
		case PID:
			if ( one->pid > two->pid )
			{
				retval = -1;
			}
			else if ( one->pid < two->pid )
			{
				retval = 1;
			}
			break;
		case SWAP:
			if ( one->swap[0] > two->swap[0] )
			{
				retval = -1;
			}
			else if ( one->swap[0] < two->swap[0] )
			{
				retval = 1;
			}
			break;
		case VIRT:
			if ( one->virt > two->virt )
			{
				retval = -1;
			}
			else if ( one->virt < two->virt )
			{
				retval = 1;
			}
			break;
		case RES:
			if ( one->res > two->res )
			{
				retval = -1;
			}
			else if ( one->res < two->res )
			{
				retval = 1;
			}
			break;
		default:
			if ( one->cpu_percent > two->cpu_percent )
			{
				retval = -1;
			}
			else if ( one->cpu_percent < two->cpu_percent )
			{
				retval = 1;
			}
			break;
	}
	if ( parametres.reversesort == 1 )
	{
		retval *= -1;
	}
	return ( retval );
}

