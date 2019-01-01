/*
 * A simple program illustrating the use of the eqn libray
 *
 * This program reads in an equation from a string,
 * prints out the derivative of the equation with respect to x
 *
 * This program can be compiled with 
 *
 * cc simple.c -I/usr/local/include -L/usr/local/lib -leqn -lm -o simple
 *
 */

#include <stdio.h>
#include <eqn.h>

main()
{
	eqnode *eqn,*diff; /* Two pointers to structures used
				to store the equations. */

	/* Convert a string to an equation */

	eqn = sscan_eqn(" x * ( x + 1 )");

	/* create a copy of the equation */

	diff = duplicate(eqn); 

	/* Differentiate with respect to x */

	diff_wrt(diff,"x");

	/* Tidy up the answer */

	clean_eqn(diff);

	/* Print out the answer */

	printf("The derivative of ");
	print_eqn(eqn);
	printf("\nis ");
	print_eqn(diff);
	printf("\n");
}
	
