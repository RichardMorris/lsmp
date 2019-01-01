/*
 * Routine to print out an equation as a 
 * C function
 * requires an expresion of the form 
 * X = .... ;
 * Will produce a typical ooange type definition like
 * 
 */
#ifdef NOT_DEF
/* BEGIN0 -- don't edit until END0 */   
struct in {
        double  x;
        double  y;
};

struct out {
        double  z;
};

void
myfunction(  struct in *in, struct out *out )
/* END0  */                             
{
        out->z = in->x * in->x - in->y * in->y -0.5;                    
    return;                             
}

}
#endif
/*
 * it can't handle vectors at present, 
 * multiplication signs are always added.
 * ^ is always interprated as pow(x,y)
 * all numbers are written as floating point
 * we use the names stuff to produce a list of name
 */


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "eqn.h"
/*
#include <values.h>
*/

#define grballoc(node) (node *) malloc(sizeof(node))
#define MAX(a,b)       a > b ? a : b ;
#define TRUE 1
#define FALSE 0

/**** prints the equation with lots of brackets! ****/

int fCpe_len = 0;

/* The recursive C printing routine */

void fCpe2(file,base)
FILE *file;
eqnode *base;
{
  if( ++fCpe_len > 20 ) 
  {
	fprintf(file,"\n");
	fCpe_len = 0;
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
    fCpe2(file,base->u.f.a);
    fprintf(file,")");
    break;
  case NAME:
    fprintf(file,"in->%s",base->u.str);
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
    fCpe2(file,base->u.n.r);
    fprintf(file,")");
    break;

  case INTERVAL:
    fprintf(stderr,"Intervals not allowed for C print out\n");
    return;
    fprintf(file,"[");
    fCpe2(file,base->u.n.l);
    fprintf(file,",");
    fCpe2(file,base->u.n.r);
    fprintf(file,"]");
    break;
  case ',':
  case '=': 
    fCpe2(file,base->u.n.l);
    fprintf(file,"%c",base->op);
    fCpe2(file,base->u.n.r);
    break;
    
  case '+':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      fCpe2(file,base->u.n.l); break;
    default:	/* ',' and '-' and '=' */
      fprintf(file,"("); fCpe2(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"+");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      fCpe2(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '-':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case '/': case '+': case NAME: case NUMBER:
    case FUNCTION:
      fCpe2(file,base->u.n.l); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"-");

    switch( base->u.n.r->op )
    {
    case '^': case '*': case '/': case NAME: case NUMBER:
    case FUNCTION:
      fCpe2(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '*': 
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      fCpe2(file,base->u.n.l); break;
    default: /* '+' '-' '/' ',' '=' */
      fprintf(file,"("); fCpe2(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"%c",base->op);

    switch( base->u.n.r->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      fCpe2(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
  case '/':
    switch( base->u.n.l->op )
    {
    case '^': case '*': case NAME: case NUMBER: case FUNCTION:
      fCpe2(file,base->u.n.l); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"/");

    switch( base->u.n.r->op )
    {
    case '^': case NAME: case NUMBER: case FUNCTION:
      fCpe2(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;

  case '^':
#ifdef NOT_DEF
    switch( base->u.n.l->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      fCpe2(file,base->u.n.l); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.l); fprintf(file,")"); break;
    }

    fprintf(file,"^");

    switch( base->u.n.r->op )
    {
    case NAME: case NUMBER: case FUNCTION:
      fCpe2(file,base->u.n.r); break;
    default:
      fprintf(file,"("); fCpe2(file,base->u.n.r); fprintf(file,")"); break;
    }
    break;
#endif
    fprintf(file,"pow(");
    fCpe2(file,base->u.n.l);
    fprintf(file,",");
    fCpe2(file,base->u.n.r);
    fprintf(file,")");
    break;

#ifdef NOT_DEF
    fCpe2(file,base->u.n.l);
    fprintf(file,"%c",base->op);
    fCpe2(file,base->u.n.r);
    break;
#endif
  case '.':
  default:
    eprintf("bad op: ");
    eprint_op(base->op);
    eprintf(" ");
  }
}
void fCprint_eqn(file, base )
FILE *file;
eqnode *base;
{
  eqn_names *names;
  int i,n;
  char *parameter;

  if( base == NULL )
  {
	eprintf("NULL equation in fCprint_eqn\n");
	return;
  }
  if( base->op != '=' || base->u.n.l->op != NAME ) 
  {
	eprintf("Equation must be of form  x = ...\n");
	return;
  }
  
  names = add_eqn_names(NULL,base->u.n.r);

  fprintf(file,"#include <math.h>\n\n");
  fprintf(file,"/* BEGIN0 -- don't edit until END0 */\n");
  fprintf(file,"struct in {\n");

  n = num_parameters(names);
  for(i=0;i<n;++i)
  {
	parameter = get_parameter(names,i+1);
	fprintf(file,"        float %s;\n",parameter);
  }
  fprintf(file,"};\n\n");

  fprintf(file,"struct out {\n");
  fprintf(file,"        float %s;\n",base->u.n.l->u.str);
  fprintf(file,"};\n\n");

  fprintf(file,"void\n");
  fprintf(file,"myfunction(  struct in *in, struct out *out )\n");
  fprintf(file,"/* END0  */\n");
  fprintf(file,"{\n");
  fprintf(file,"\tout->%s =\n",base->u.n.l->u.str);

  fCpe_len = 0;
  fCpe2(file,base->u.n.r);
  fprintf(file,";\n\treturn;\n");
  fprintf(file,"}\n");
} 

