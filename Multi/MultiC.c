/* print out a C function which represents the Multi
 * prints in accordance to oorange guidelines
 * with the variables being the first results in the argument
 * list, followed by the parameters, in alphabetical order
 * Assume that we have simple non vector notations
 * undefined results otherwise.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "eqn.h"
#include "Multi.h"

#define grballoc(node) (node *) malloc(sizeof(node))
#define MAX(a,b)       a > b ? a : b ;
#define TRUE 1
#define FALSE 0

int MfCpe_len = 0;


/**** prints the equation with lots of brackets! ****/

void fprintNoAt(FILE *file,char *str)
{
  char c;
  int len,i;

	/* A substitution equation, want to  replace any occurence
		of @ with _D_ */
	len = strlen(str);
	for(i=0;i<len;++i)
	{
		c = str[i];
		if( c == '@' ) fprintf(file,"_D_");
		else		putc(c,file);
	}
}

/* The recursive C printing routine */

void MfCpe2(Multi *mul,FILE *file,eqnode *base)
{
  int type;

  if( ++MfCpe_len > 15 ) 
  {
	fprintf(file,"\n");
	MfCpe_len = 0;
  }

  switch( base->op )
  {
  case FUNCTION:
    if(base->u.f.f->type == CONSTANT_FUN)
    {
	fprintf(file,"%s",base->u.f.f->name);
	break;
    }
    fprintf(file,"%s(",base->u.f.f->name);
    MfCpe2(mul,file,base->u.f.a);
    fprintf(file,")");
    break;
  case NAME:
    type = Mget_type(mul,base->u.str);
    switch(type)
    {
    case M_VAR:  case M_SPEC_VAR:
    case M_PARAM: case M_SPEC_PARAM:
    	fprintf(file,"in->%s",base->u.str);
        break;
    case M_SUB:
	fprintNoAt(file,base->u.str);
        break;
    }
    break;
		
  case NUMBER:
#ifdef __BORLANDC__
#define rint(a) floor(a)
#endif
    if( eqnval(base) == rint(eqn_val(base)) ) 
	fprintf(file,"%.1lf",eqn_val(base));
    else if( eqnval(base)*10.0 == rint(eqn_val(base)*10.0) ) 
	fprintf(file,"%.1lf",eqn_val(base));
    else if( eqnval(base)*100.0 == rint(eqn_val(base)*100.0) ) 
	fprintf(file,"%.2lf",eqn_val(base));
    else if( eqnval(base)*1000.0 == rint(eqn_val(base)*1000.0) ) 
	fprintf(file,"%.3lf",eqn_val(base));
    else if( eqnval(base)*10000.0 == rint(eqn_val(base)*10000.0) ) 
	fprintf(file,"%.4lf",eqn_val(base));
    else
	fprintf(file,"%lf",eqn_val(base));
    break;
  case BRACKET:
    fprintf(file,"(");
    MfCpe2(mul,file,base->u.n.r);
    fprintf(file,")");
    break;

  case INTERVAL:
    fprintf(stderr,"Intervals not allowed for C print out\n");
    return;
/*
    fprintf(file,"[");
    MfCpe2(mul,file,base->u.n.l);
    fprintf(file,",");
    MfCpe2(mul,file,base->u.n.r);
    fprintf(file,"]");
    break;
*/
  case ',':
  case '=': 
    MfCpe2(mul,file,base->u.n.l);
    fprintf(file,"%c",base->op);
    MfCpe2(mul,file,base->u.n.r);
    break;
    
  case '+':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      MfCpe2(mul,file,base->u.n.l); break;
    default:	/* ',' and '-' and '=' */
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"+");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      MfCpe2(mul,file,base->u.n.r); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '-':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      MfCpe2(mul,file,base->u.n.l); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"-");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case NAME: case NUMBER:
    case FUNCTION:
      MfCpe2(mul,file,base->u.n.r); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '*': 
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      MfCpe2(mul,file,base->u.n.l); break;
    default: /* '+' '-' '/' ',' '=' */
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"%c",base->op);

    switch( base->u.n.r->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      MfCpe2(mul,file,base->u.n.r); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '/':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      MfCpe2(mul,file,base->u.n.l); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"/");

    switch( base->u.n.r->op )
    {
    case '^': case NAME: case NUMBER: case FUNCTION:
      MfCpe2(mul,file,base->u.n.r); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.r); fprintf(file,")"); break;
    }
    break;

  case '^':
#ifdef NOT_DEF
    switch( base->u.n.l->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      MfCpe2(mul,file,base->u.n.l); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"^");

    switch( base->u.n.r->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      MfCpe2(mul,file,base->u.n.r); break;
    default:
      fprintf(file,"("); MfCpe2(mul,file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
#endif
    fprintf(file,"pow(");
    MfCpe2(mul,file,base->u.n.l);
    fprintf(file,",");
    MfCpe2(mul,file,base->u.n.r);
    fprintf(file,")");
    break;

#ifdef NOT_DEF
    MfCpe2(mul,file,base->u.n.l);
    fprintf(file,"%c",base->op);
    MfCpe2(mul,file,base->u.n.r);
    break;
#endif
  case '.':
  default:
    eprintf("bad op: ");
    eprint_op(base->op);
    eprintf(" ");
  }
}

void fCprintMulti(FILE *file,Multi *mul)
{
	int i;

	fprintf(file,"#include <math.h>\n\n");
	fprintf(file,"/* BEGIN0 -- don't edit until END0 */\n");
	fprintf(file,"struct in {\n");
	for(i=0;i<mul->n_vars;++i)
		fprintf(file,"double %s;\n",mul->bignames[i]);
	for(i=0;i<mul->n_param;++i)
		fprintf(file,"double %s;\n",
			mul->bignames[mul->n_vars+i]);
 	fprintf(file,"};\n\n");

	fprintf(file,"struct out {\n");
	for(i=0;i<mul->n_top;++i)
		fprintf(file,"double top%d;\n",i);
	fprintf(file,"};\n\n");

	fprintf(file,"void\n");
	fprintf(file,"myfunction(  struct in *in, struct out *out )\n");
	fprintf(file,"/* END0  */\n");
	fprintf(file,"{\n");
	for(i=mul->n_vars+mul->n_param;i<mul->n_big;++i)
	{
		fprintf(file,"double ");
		fprintNoAt(file,mul->bignames[i]);
		fprintf(file,";\n");
	}
/* Don't need to do derivatives 
	for(i=mul->n_top;i<mul->n_top+mul->n_derivs;++i)
		fprintf(file,"double deriv%d;\n",i-mul->n_top);
*/
	for(i=mul->n_eqns-1;i>mul->n_top+mul->n_derivs;--i)
	{
		MfCpe_len = 0;
		MfCpe2(mul,file,mul->eqns[i]);
		fprintf(file,";\n");
	}
	for(i=0;i<mul->n_top;++i)
	{
		MfCpe_len = 0;
		fprintf(file,"out->top%d = ",i);
		MfCpe2(mul,file,mul->eqns[i]);
		fprintf(file,";\n");
	}
	fprintf(file,"return;\n");
	fprintf(file,"}\n");
}


