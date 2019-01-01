/*
 * A posible bug if gcc
 * heres the command line

$ make bug
gcc  -O2 -g   -c bug.c -o bug.o
g++ bug.o -lm -o bug
$ bug
val[0] = 1.000000 1 0 0
val[1] = NaN 0 0 1
val[2] = NaN 0 0 1
val[3] = 1.000000 1 0 0
val[4] = NaN 0 0 1
val[5] = 1.000000 1 0 0
val[6] = 1.000000 1 0 0
val[7] = NaN 0 0 1
+val[0] = 1.000000 1 0 0
-val[1] = NaN 0 0 1			 we would expect this to be ? 
-val[2] = NaN 0 0 1
+val[3] = 1.000000 1 0 0
-val[4] = NaN 0 0 1
+val[5] = 1.000000 1 0 0
+val[6] = 1.000000 1 0 0
-val[7] = NaN 0 0 1
$ gcc -v
Reading specs from /usr/lib/gcc-lib/i386-linux/2.7.2.1/specs
gcc version 2.7.2.1

 */

#include <stdio.h>
#include <math.h>
#define MEMSET
#ifdef MEMSET
#define bzero(a,b) memset((a),0,(b))
#endif

/*
#define PRINT_VALS
*/
double tolerance = 0.001;

#define PLUS_VAL(x) ( x >= -tolerance )
#define NEG_VAL(x) ( x < -tolerance )
#define BAD_VAL(x) ( x != x )
#define COL_INTERP(A,B,C,lam) do { \
	A.r = lam * B.r + (1-lam) * C.r; \
	A.g = lam * B.g + (1-lam) * C.g; \
	A.b = lam * B.b + (1-lam) * C.b; \
	A.a = lam * B.a + (1-lam) * C.a; \
	} while(0); 

/*
 * Function:	VectClip Clip
 * Action:	Returns a Clipped version of the vect
 */

main()
{
	int i,j,k,n,col_index;
	int	b;
	double  a = 0, c = 0;
	double *val;

	val = (double *) malloc(sizeof(double)*16);

	for(i=0;i<8;++i)
	{
		b = 1;
		if( i & 0x01 ) b = -b;
		if( i & 0x02 ) b = -b;
		if( i & 0x04 ) b = -b;
		if( i & 0x08 ) b = -b;
		if( b > 0 ) 
			val[i] = 1;
		else	val[i] = a/c;
		
printf("val[%d] = %f %d %d %d\n",i,val[i],
	PLUS_VAL(val[i]),NEG_VAL(val[i]),BAD_VAL(val[i]));
	}

	for(i=0;i<8;++i)
	{
			if( PLUS_VAL(val[i]) )
			{
printf("+val[%d] = %f %d %d %d\n",i,val[i],
	PLUS_VAL(val[i]),NEG_VAL(val[i]),BAD_VAL(val[i]));
			}
			else if( NEG_VAL(val[i]) )
			{
printf("-val[%d] = %f %d %d %d\n",i,val[i],
	PLUS_VAL(val[i]),NEG_VAL(val[i]),BAD_VAL(val[i]));
			}
			else if( BAD_VAL(val[i]) )
			{
printf("?val[%d] = %f %d %d %d\n",i,val[i],
	PLUS_VAL(val[i]),NEG_VAL(val[i]),BAD_VAL(val[i]));
			}
			else
			{
printf("*val[%d] = %f %d %d %d\n",i,val[i],
	PLUS_VAL(val[i]),NEG_VAL(val[i]),BAD_VAL(val[i]));
			}
	}
}
