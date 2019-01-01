#include <polylist.h>
#include <polylistP.h>

typedef struct clipedge
{
	Vertex *low,*high,*sol;
	struct clipedge *next;
} clipedge;

/*
 * Function:	newposn
 * Action:	given a pointer to old array return pointer to new array
 */

Vertex *newfromold(Vertex *oldposn);
