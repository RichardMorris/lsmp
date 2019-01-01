/*
 * Copyright I guess there should be some copywrite for this package,
 * 
 * 			Copyright (c) 1992
 * 
 * 	Liverpool University Department of Pure Mathematics,
 * 	Liverpool, L69 3BX, England.
 * 
 * 	Author Dr R. J. Morris.
 * 
 * 	e-mail rmorris@uk.ac.liv.uxb
 *
 * This software is copyrighted as noted above.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is
 * preserved on all copies.
 *
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the authors, who may or may not act on them as they desire.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 *
 * All this software is public domain, as long as it is not used by any military
 * establishment. Please note if you are a military establishment then a mutating
 * virus has now escaped into you computer and is presently turning all your
 * programs into socially useful, peaceful ones.
 * 
 */

#ifndef EQN_HEADER
#define EQN_HEADER

#ifdef THINK_C
#define BOR_THINK
#endif
#ifdef __BORLANDC__
#define BOR_THINK
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef	M_PI
#define	M_PI		3.14159265358979323846
#endif

#define TAD  0.0000001

/* Codes used for the operator in equations and rpe's */
/* Can also have '+' '-' '*' '/' '^' '=' ',' 	      */

#define NAME -23
#define NUMBER -24
#define INTERVAL -25
#define BRACKET -26
#define FUNCTION 3

/* Special codes used in rpe calculator */

#define SUM1	5
#define SUB1	6
#define MULT1	7
#define DIV1	8
#define POW1	9
#define INT_POW 10
#define SUM2	11
#define SUM3    12	
#define SUB2    13	
#define SUB3	14 
#define DOT2	15
#define	DOT3	16
#define SCALE2	17
#define SCALE3	18
#define	CPLX_MUL	19
#define	CROSS2	20
#define	CROSS3	21
#define EQUALS1 22
#define SUM4	23
#define SUB4	24
#define DOT4	25
#define SCALE4  26
#define DIV2    27
#define DIV3    28
#define CPLX_DIV 29

/* Maximum order for polynomials  and small order for 4D polys */

#ifdef __BORLANDC__
#define MAXORDER 5
#else
#define MAXORDER 25
#endif
#define SMALLORDER 6

/* Where to read an equation from */

#define EQNFROM_STDIN 1
#define EQNFROM_FILE 2
#define EQNFROM_STRING 3

/* The main type for equations */

struct Fnode;	/* Forward def */

typedef struct Eqnode
{
	short int op;	/* The operator */
	union
	{
		struct
		{
			struct Eqnode *l, *r; /* left + right sub trees */
		} n;
		struct
		{
			struct Fnode *f;	/* Function	*/
			struct Eqnode *a;	/* Its arguments */
		} f;
	    
		double num;			/* Value for NUMBERS */
		char   *str;			/* String for NAMES */
	} u;
} eqnode;

typedef eqnode eqn_node;
typedef eqnode * eqn_ptr;
extern  char      eqn_err_string[];
extern  int    eqn_err_flag;
extern  int    eqn_errs_to_stderr;

#define VARIABLE 1
#define PARAMETER 2

typedef struct Nnode   /* a node about the list of names */
{
  int type;			/* The type of name	*/
  char *str;			/* The name		*/
  struct Nnode *next;		/* next name in list	*/
} eqn_names;

/* The list of functions */

#define EXTERN_FUN 4
#define INTERN_FUN 5
#define CONSTANT_FUN 6
#define OPERATOR 7
#define EXTERN_MAP 8
#define INTERN_MAP 9
#define SUM_FUN 10

typedef struct Fnode
{
	short   type;		/* The type of function 		*/
	char    *name;		/* The name				*/
	short   nvars;		/* The number of arguments		*/
	short	dim;		/* Dimension of target space		*/
	char	**vars;		/* The list of varible names		*/
	double  (*fun)();	/* Pointer to single valued function	*/
	double  *(*vfun)();	/* Pointer to multivalued mapping	*/
	eqn_ptr (*op)();	/* Pointer to operator			*/
	int	*rpe;		/* Reverse polish string		*/
	int	*rpe2;		/* for sums one for initial one for recurence		*/
	int	*vrpe;		/* Vetor rpe mappings			*/
	double  val;		/* Value of constant			*/
	eqnode  *eqn;		/* The equation for the function	*/
	eqnode  *eqn2;		/* The equation for the function	*/
	eqnode	**diff;		/* Equations for each derivative	*/
	struct  Fnode *next;	/* The next function in the list.	*/
} eqn_funs;

/* exteral functions i.e. those define outside the eqn package 
	use name, fun, nvars, vars, diff
   interal functions i.e. those defined ase strings "sec(x)=1/cos(x)"
	use name, eqn, nvars, vars, diff, rpe
   constant funs 
	use name, val
   operators i.e. diff
	use name, op
   sum operators Sum, Min, Max, Prod, Int
	use name, fun, nvars, vars, diff
   external mappings 
	use name, vfun, nvars, vars, diff
   internal mappings
	use name, eqn, nvars, vars, vrpe, diff
  */
   
typedef int rpeint;	/* The type used in rpe strings */

/* Definitions from eqnbase.c */

extern eqnode	*scan_eqn(), *fscan_eqn(), *sscan_eqn(), *duplicate();
extern void	display_eqn(), print_eqn(), fprint_eqn(), eprint_eqn();
extern void	copy_node(), free_eqn_tree(), free_eqn_node();
extern int	count_eqn_tree(), count_eqn_args();
extern eqnode	*get_eqn_arg(), *join_dup_eqns(), *join_eqns();
extern int	eprintf();
extern void	eprint_op(),print_op(),fprint_op(),fprint_num();

/* Definitions from eqnexpand.c */

extern int	clean_eqn(),eval_funs(),eval_ops();
extern int	expand(), multiply_out(), devide_out(), raise_out();

extern void common_denominator(eqnode *eqn);
extern void common_denom2(eqnode *l, eqnode *r);

/* defs from eqncmp.c */

extern int compare_sums(eqnode *l,eqnode *r);
int compare_elements(eqnode *l,eqnode *r);
int compare_products(eqnode *l,eqnode *r);


/* Definition from eqndiff.c */

extern void 	diff_wrt(), diff_fun_wrt();
extern eqnode	*diff_wrt_eqn();

/* Definitions from eqnsubst.c */

extern int	substitute();
extern eqnode	*assign();

/* Definitions from eqnpoly.c */

extern	void	fprint_poly1(), print_poly1(), init_poly1(), order_poly1();
extern	void	fprint_poly2(), print_poly2(), init_poly2(), order_poly2();
extern	void	fprint_poly3(),	print_poly3(), init_poly3(), order_poly3();
extern	void	fprint_poly4(), print_poly4(), init_poly4(), order_poly4();
extern int	add_to_poly1(), sub_from_poly1();
extern int	add_to_poly2(), sub_from_poly2();
extern int	add_to_poly3(), sub_from_poly3();
extern int	add_to_poly2(), sub_from_poly2();
extern int	eval_term1(), eval_term4();
extern int	eval_term2(), eval_term3();

/* Definitions from eqnrpe.c */

extern int	make_rpe2(), check_rpe();
extern rpeint	*make_rpe();
extern	void	fprint_rpe(), print_rpe(), clear_rpe_const();
extern double	eval_rpe();

/* Definitions from eqnvrpe.c */

extern int	make_vrpe2(), check_vrpe();
extern rpeint	*make_vrpe();
extern	void	fprint_vrpe(), print_vrpe(), clear_vrpe_const();
extern double	*eval_vrpe();

/* Definitions from eqnnames.c */

extern eqn_names	*add_eqn_names();
extern	void	print_eqn_names();
extern	void	fprint_eqn_names();
extern int	num_parameters();
extern void	free_eqn_names();
extern int	make_variable();
extern char	*get_parameter();

/* Definitions from eqnfunct.c */

extern eqn_funs	*add_external_function();
extern eqn_funs	*add_internal_function();
extern eqn_funs	*add_standard_functions();
extern eqn_funs	*add_sum_function();
extern eqn_funs	*add_constant();
extern eqn_funs	*add_operator();
extern	void	fprint_funs();
extern int	use_functions();
extern 	void	set_input_functions();
extern eqn_funs *get_input_functions();

/* Macros */

#define	eqn_op(eqn)	(eqn == NULL ? NULL : (eqn)->op)
#define eqn_l(eqn)	(eqn == NULL ? NULL :(eqn)->u.n.l)
#define	eqn_r(eqn)	(eqn == NULL ? NULL :(eqn)->u.n.r)
#define eqn_val(eqn)	(eqn == NULL ? 0 : (eqn)->op == NUMBER ? (eqn)->u.num : 0)
#define eqn_name(eqn)	(eqn == NULL ? NULL : (eqn)->op == NAME ? (eqn)->u.str : NULL)
#define eqn_fun(eqn)	(eqn == NULL ? NULL : (eqn)->op == FUNCTION ? (eqn)->u.f.f : NULL)
#define eqn_arg(eqn)	(eqn == NULL ? NULL : (eqn)->op == FUNCTION ? (eqn)->u.f.a : NULL)

#define	eqnop(eqn)	((eqn)->op)
#define eqnl(eqn)	((eqn)->u.n.l)
#define	eqnr(eqn)	((eqn)->u.n.r)
#define eqnval(eqn)	((eqn)->u.num)
#define eqnname(eqn)	((eqn)->u.str)
#define eqnfun(eqn)	((eqn)->u.f.f)
#define eqnarg(eqn)	((eqn)->u.f.a )

#define remove_eqn_name(namelist,name)	make_varible(namelist,name)
#define num_eqn_names(namelist)	num_parameters(namelist)
#define get_eqn_name(namelist,i)	get_parameter(namelist,i)

#endif /* EQN_HEADER */
