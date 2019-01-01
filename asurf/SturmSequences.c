/*
Using Sturm Sequences to Bracket Real Roots of Polynomial Equations
by D.G. Hook and P.R. McAree
from "Graphics Gems", Academic Press, 1990
#
# Makefile
#
#	command file for make to compile the solver.

solve: main.o sturm.o util.o
	cc -o solve main.o sturm.o util.o -lm

*/

#include <math.h>
#include <stdio.h>
#include "bern.h"

/*
 * solve.h
 *
 *	some useful constants and types.
 */
#define	 	MAX_ORDER  		MAXORDER
/* maximum order for a polynomial */
	
#define	  	RELERROR	  		1.0e-14
/* smallest relative error we want */

#define	  	MAXPOW	    	32		
/* max power of 10 we wish to search to */

#define	  	MAXIT	     	800		
/* max number of iterations */

/* a coefficient smaller than SMALL_ENOUGH is considered to 
   be zero (0.0). */

#define	  	SMALL_ENOUGH		1.0e-12


/*
 * structure type for representing a polynomial
 */
typedef  	struct	p {
		     int	ord;
		     double	coef[MAX_ORDER];
} poly;

extern 	int		modrf();
extern 	int		numroots();
extern 	int		numchanges();
extern 	int		buildsturm();

extern 	double	evalpoly();
	


/*
 * util.c
 *
 *	some utlity functions for root polishing and evaluating
 * polynomials.
 */

/*rjm
#include "solve.h"
*/

/*
 * evalpoly
 *
 *	evaluate polynomial defined in coef returning its value.
 */
double
evalpoly (ord, coef, x)
	int		ord;
	double	*coef, x;
{
	double	*fp, f;

	fp = &coef[ord];
	f = *fp;

	for (fp--; fp >= coef; fp--)
	f = x * f + *fp;

	return(f);
}


/*
 * modrf
 *
 *	uses the modified regula-falsi method to evaluate the root
 * in interval [a,b] of the polynomial described in coef. The
 * root is returned is returned in *val. The routine returns zero
 * if it can't converge.
 */
int
modrf(ord, coef, a, b, val)
	int		ord;
	double	*coef;
	double	a, b, *val;
{
	int		its;
	double	fa, fb, x, lx, fx, lfx;
	double	*fp, *scoef, *ecoef;

	scoef = coef;
	ecoef = &coef[ord];

	fb = fa = *ecoef;
	for (fp = ecoef - 1; fp >= scoef; fp--) {
		fa = a * fa + *fp;
		fb = b * fb + *fp;
	}

	/*
	 * if there is no sign difference the method won't work
	 */
	if (fa * fb > 0.0)
		return(0);

	if (fabs(fa) < RELERROR) {
		*val = a;
		return(1);
	}

	if (fabs(fb) < RELERROR) {
		*val = b;
		return(1);
	}

	lfx = fa;
	lx = a;


	for (its = 0; its < MAXIT; its++) {

		x = (fb * a - fa * b) / (fb - fa);

		fx = *ecoef;
		for (fp = ecoef - 1; fp >= scoef; fp--)
				fx = x * fx + *fp;

		if (fabs(x) > RELERROR) {
				if (fabs(fx / x) < RELERROR) {
					*val = x;
					return(1);
				}
		} else if (fabs(fx) < RELERROR) {
				*val = x;
				return(1);
		}

		if ((fa * fx) < 0) {
				b = x;
				fb = fx;
				if ((lfx * fx) > 0)
					fa /= 2;
		} else {
				a = x;
				fa = fx;
				if ((lfx * fx) > 0)
					fb /= 2;
		}

		lx = x;
		lfx = fx;
	}

	fprintf(stderr, "modrf overflow %f %f %f\n", a, b, fx);

	return(0);
}
	



/*
 * main.c
 *
 *	a sample driver program.
 */
#include <stdio.h>
#include <math.h>
/*rjm
#include "solve.h"
*/

/*
 * a driver program for a root solver.
 */
sterm_main()
{
	poly	sseq[MAX_ORDER];
	double 	min, max, roots[MAX_ORDER];
	int		i, j, order, nroots, nchanges, np, atmin, atmax;

	/*
	 * get the details...
	 */

	while(1)
	{
	printf("Please enter order of polynomial: ");
	if( scanf("%d", &order) == 0) break;

	printf("\n");

	for (i = order; i >= 0; i--) {
			printf("Please enter coefficient number %d: ", i);
			scanf("%lf", &sseq[0].coef[i]);
	}

	printf("\n");

	/*
	 * build the Sturm sequence
	 */
	np = buildsturm(order, sseq);

	printf("Sturm sequence for:\n");

	for (i = order; i >= 0; i--)
			printf("%lg ", sseq[0].coef[i]);

	printf("\n\n");

	for (i = 0; i <= np; i++) {
			for (j = sseq[i].ord; j >= 0; j--)
				printf("%lg ", sseq[i].coef[j]);
			printf("\n");
	}

	printf("\n");


	/* 
	 * get the number of real roots
	 */
	nroots = numroots(np, sseq, &atmin, &atmax);

	if (nroots == 0) {
			printf("solve: no real roots\n");
			return(0);
	}

	/*
	 * calculate the bracket that the roots live in
	 */
/*
	min = -1.0;
	nchanges = numchanges(np, sseq, min);
	for (i = 0; nchanges != atmin && i != MAXPOW; i++) { 
			min *= 10.0;
			nchanges = numchanges(np, sseq, min);
	}

	if (nchanges != atmin) {
			printf("solve: unable to bracket all negetive roots\n");
			atmin = nchanges;
	}

	max = 1.0;
	nchanges = numchanges(np, sseq, max);
	for (i = 0; nchanges != atmax && i != MAXPOW; i++) { 
			max *= 10.0;
			nchanges = numchanges(np, sseq, max);
	}

	if (nchanges != atmax) {
			printf("solve: unable to bracket all positive roots\n");
			atmax = nchanges;
	}
*/
	min = 0.0;
	max = 1.0;
	atmin = numchanges(np, sseq, min);
	atmax = numchanges(np, sseq, max);

	nroots = atmin - atmax;

	/*
	 * perform the bisection.
	 */
	sbisect(np, sseq, min, max, atmin, atmax, roots);


	/*
	 * write out the roots...
	 */
	if (nroots == 1) {
			printf("\n1 distinct real root at x = %f\n", roots[0]);
	} else {
			printf("\n%d distinct real roots for x: ", nroots);

			for (i = 0; i != nroots; i++)
				printf("%f ", roots[i]);
			printf("\n");
	}
	}
}

int calc_sterm_root(bern1D *bb,double roots[MAXORDER])
{
	poly	sseq[MAX_ORDER];
	double 	min, max;
	int		i, j, n, order, nroots, nchanges, np, atmin, atmax;

	/*
	 * get the details...
	 */
		n = bb->ord;
		for(i=0;i<=bb->ord;++i)
			sseq[0].coef[i]=0.0;
	
		for(i=0;i<=n;++i)
		{
			for(j=0;j<=i;++j)
			{
			    if(j%2==1)
				sseq[0].coef[n+j-i] -= comb(n,i) * comb(i,j) * *(bb->array+i);
			    else
				sseq[0].coef[n+j-i] += comb(n,i) * comb(i,j) * *(bb->array+i);
			}
		}

	order = n;
	np = buildsturm(order, sseq);
/*
	printf("Sturm sequence for:\n");
	for (i = order; i >= 0; i--)
			printf("%lg ", sseq[0].coef[i]);
	printf("\n\n");
	for (i = 0; i <= np; i++) {
			for (j = sseq[i].ord; j >= 0; j--)
				printf("%lg ", sseq[i].coef[j]);
			printf("\n");
	}
	printf("\n");
*/


	/* 
	 * get the number of real roots
	 */
	nroots = numroots(np, sseq, &atmin, &atmax);

	if (nroots == 0) { return 0; }

	min = 0.0;
	max = 1.0;
	atmin = numchanges(np, sseq, min);
	atmax = numchanges(np, sseq, max);

	nroots = atmin - atmax;
	sbisect(np, sseq, min, max, atmin, atmax, roots);
	return nroots;
}

/*
 * sturm.c
 *
 *	the functions to build and evaluate the Sturm sequence
 */
#include <math.h>
#include <stdio.h>
/*rjm
#include "solve.h"
*/

/*
 * modp
 *
 *	calculates the modulus of u(x) / v(x) leaving it in r, it
 *  returns 0 if r(x) is a constant.
 *  note: this function assumes the leading coefficient of v 
 *	is 1 or -1
 */
static int
modp(u, v, r)
	poly	*u, *v, *r;
{
	int		k, j;
	double	*nr, *end, *uc;

	nr = r->coef;
	end = &u->coef[u->ord];

	uc = u->coef;
	while (uc <= end)
			*nr++ = *uc++;

	if (v->coef[v->ord] < 0.0) {


			for (k = u->ord - v->ord - 1; k >= 0; k -= 2)
				r->coef[k] = -r->coef[k];

			for (k = u->ord - v->ord; k >= 0; k--)
				for (j = v->ord + k - 1; j >= k; j--)
					r->coef[j] = -r->coef[j] - r->coef[v->ord + k]
					* v->coef[j - k];
	} else {
			for (k = u->ord - v->ord; k >= 0; k--)
				for (j = v->ord + k - 1; j >= k; j--)
				r->coef[j] -= r->coef[v->ord + k] * v->coef[j - k];
	}

	k = v->ord - 1;
	while (k >= 0 && fabs(r->coef[k]) < SMALL_ENOUGH) {
		r->coef[k] = 0.0;
		k--;
	}

	r->ord = (k < 0) ? 0 : k;

	return(r->ord);
}

/*
 * buildsturm
 *
 *	build up a sturm sequence for a polynomial in smat, returning
 * the number of polynomials in the sequence
 */
int
buildsturm(ord, sseq)
	int		ord;
	poly	*sseq;
{
	int		i;
	double	f, *fp, *fc;
	poly	*sp;

	sseq[0].ord = ord;
	sseq[1].ord = ord - 1;


	/*
	 * calculate the derivative and normalise the leading
	 * coefficient.
	 */
	f = fabs(sseq[0].coef[ord] * ord);
	fp = sseq[1].coef;
	fc = sseq[0].coef + 1;
	for (i = 1; i <= ord; i++)
			*fp++ = *fc++ * i / f;

	/*
	 * construct the rest of the Sturm sequence
	 */
	for (sp = sseq + 2; modp(sp - 2, sp - 1, sp); sp++) {

		/*
		 * reverse the sign and normalise
		 */
		f = -fabs(sp->coef[sp->ord]);
		for (fp = &sp->coef[sp->ord]; fp >= sp->coef; fp--)
				*fp /= f;
	}

	sp->coef[0] = -sp->coef[0];	/* reverse the sign */

	return(sp - sseq);
}

/*
 * numroots
 *
 *	return the number of distinct real roots of the polynomial
 * described in sseq.
 */
int
numroots(np, sseq, atneg, atpos)
		int		np;
		poly	*sseq;
		int		*atneg, *atpos;
{
		int		atposinf, atneginf;
		poly	*s;
		double	f, lf;

		atposinf = atneginf = 0;


	/*
	 * changes at positve infinity
	 */
	lf = sseq[0].coef[sseq[0].ord];

	for (s = sseq + 1; s <= sseq + np; s++) {
			f = s->coef[s->ord];
			if (lf == 0.0 || lf * f < 0)
				atposinf++;
		lf = f;
	}

	/*
	 * changes at negative infinity
	 */
	if (sseq[0].ord & 1)
			lf = -sseq[0].coef[sseq[0].ord];
	else
			lf = sseq[0].coef[sseq[0].ord];

	for (s = sseq + 1; s <= sseq + np; s++) {
			if (s->ord & 1)
				f = -s->coef[s->ord];
			else
				f = s->coef[s->ord];
			if (lf == 0.0 || lf * f < 0)
				atneginf++;
			lf = f;
	}

	*atneg = atneginf;
	*atpos = atposinf;

	return(atneginf - atposinf);
}

/*
 * numchanges
 *
 *	return the number of sign changes in the Sturm sequence in
 * sseq at the value a.
 */
int
numchanges(np, sseq, a)
	int		np;
	poly	*sseq;
	double	a;

{
	int		changes;
	double	f, lf;
	poly	*s;

	changes = 0;

	lf = evalpoly(sseq[0].ord, sseq[0].coef, a);

	for (s = sseq + 1; s <= sseq + np; s++) {
			f = evalpoly(s->ord, s->coef, a);
			if (lf == 0.0 || lf * f < 0)
				changes++;
			lf = f;
	}

	return(changes);
}

/*
 * sbisect
 *
 *	uses a bisection based on the sturm sequence for the polynomial
 * described in sseq to isolate intervals in which roots occur,
 * the roots are returned in the roots array in order of magnitude.
 */
void sbisect(np, sseq, min, max, atmin, atmax, roots)
	int		np;
	poly	*sseq;
	double	min, max;
	int		atmin, atmax;
	double	*roots;
{
	double	mid;
	int		n1, n2, its, atmid, nroot;

	if ((nroot = atmin - atmax) == 1) {

		/*
		 * first try a less expensive technique.
		 */
		if (modrf(sseq->ord, sseq->coef, min, max, &roots[0]))
			return;


		/*
		 * if we get here we have to evaluate the root the hard
		 * way by using the Sturm sequence.
		 */
		for (its = 0; its < MAXIT; its++) {
				mid = (min + max) / 2;

				atmid = numchanges(np, sseq, mid);

				if (fabs(mid) > RELERROR) {
					if (fabs((max - min) / mid) < RELERROR) {
						roots[0] = mid;
						return;
					}
				} else if (fabs(max - min) < RELERROR) {
					roots[0] = mid;
					return;
				}

				if ((atmin - atmid) == 0)
					min = mid;
				else
					max = mid;
			}

		if (its == MAXIT) {
				fprintf(stderr, "sbisect: overflow min %f max %f\
					diff %e nroot %d n1 %d n2 %d\n",
					min, max, max - min, nroot, n1, n2);
			roots[0] = mid;
		}

		return;
	}

	/*
	 * more than one root in the interval, we have to bisect...
	 */
	for (its = 0; its < MAXIT; its++) {

			mid = (min + max) / 2;

			atmid = numchanges(np, sseq, mid);

			n1 = atmin - atmid;
			n2 = atmid - atmax;


			if (n1 != 0 && n2 != 0) {
				sbisect(np, sseq, min, mid, atmin, atmid, roots);
				sbisect(np, sseq, mid, max, atmid, atmax, &roots[n1]);
				break;
			}

			if (n1 == 0)
				min = mid;
			else
				max = mid;
	}

	if (its == MAXIT) {
			fprintf(stderr, "sbisect: roots too close together\n");
			fprintf(stderr, "sbisect: overflow min %f max %f diff %e\
				nroot %d n1 %d n2 %d\n",
				min, max, max - min, nroot, n1, n2);
			for (n1 = atmax; n1 < atmin; n1++)
			roots[n1 - atmax] = mid;
	}
}


