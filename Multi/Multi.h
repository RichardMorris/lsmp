#include <stdio.h>
#include "eqn.h"

#ifndef MULTI_INCLUDE
#define MULTI_INCLUDE

typedef rpeint Mvrpeint;

#define MAX_NUM_PARAMS 50
#define MAX_NUM_VARS 50
#define MAX_NUM_EQNS 200
#define MAX_NUM_OPTS 10
#define MAX_NUM_TOP 10
#define MAX_NUM_DERIVS 20
#define MAX_NUM_SUMS 25
#define MAX_FUN_ARGS 10
#define MAX_NUM_TRANS 30

#define M_VAR 1
#define M_PARAM 2
#define M_SUB 3
#define M_SPEC_VAR 4
#define M_SPEC_PARAM 5
#define M_FREE_VBL 6

#define EQN_TO_NAME_INDEX(i) (\
(i - mul->n_top - mul->n_derivs) + mul->n_vars + mul->n_param )
#define NAME_TO_EQN_INDEX(i) (\
(i - mul->n_vars - mul->n_param ) + mul->n_top + mul->n_derivs)

typedef struct
{
	short ref;
	short hard_code;
	eqn_funs *fun;
} Msums;

typedef struct
{
	short eqnno;
	short *refs;
	short *prov_req;
	short num;
} Mtvars;

typedef struct Multinod 
{
	short	error; /* TRUE if a mistake in parsing */
	short	n_eqns,n_param,n_vars,n_opts,
			n_top,n_big,n_derivs,n_sums,n_tvars;
	short	copy_top;
	short	top_dims[MAX_NUM_TOP];
	short	n_top_derivs[MAX_NUM_TOP];
	char	*top_deriv_names[MAX_NUM_TOP][MAX_NUM_DERIVS];
	double	*results;
	short	top_res_ref[MAX_NUM_TOP]; /* which position in results
						this top is */
	short	top_deriv_ref[MAX_NUM_TOP]; /* Which derivative in the
						list of derivs the first
						one for this top is */
	/* equations */
	eqnode	*eqns[MAX_NUM_EQNS];
	/* variables */
	double  var_min[MAX_NUM_VARS];
	double  var_max[MAX_NUM_VARS];
	/* parameters */
	/* options */
	char	*opt_names[MAX_NUM_OPTS];
	double	opt_vals[MAX_NUM_OPTS];
	/* big list */
	char	*bignames[MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS];
	short	bigdim[MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS];
	short	dimsum[MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS];
	/* sums */
	short	sumref[MAX_NUM_EQNS];
	Msums	sums[MAX_NUM_SUMS];
	/* trans variables */
	short	tvarref[MAX_NUM_EQNS];
	Mtvars	tvars[MAX_NUM_TRANS];
	/* rpes */
	Mvrpeint	*rpes[MAX_NUM_EQNS];
	double	bigvals[MAX_NUM_PARAMS+MAX_NUM_VARS+MAX_NUM_EQNS];
	/* evaluation info */
	unsigned char depend[MAX_NUM_EQNS][MAX_NUM_EQNS/8];
	unsigned char depVar[MAX_NUM_EQNS][MAX_NUM_VARS/8];
	unsigned char depSpec[MAX_NUM_EQNS][MAX_NUM_SUMS/8];
	unsigned char done[MAX_NUM_EQNS/8];

} Multi;

/* Now the external definitons from Multi.c */

extern int Mget_dim(Multi *mul,eqnode *base);
extern int MrmEqns(Multi *mul,int n, int m);
extern int MaddEqn(Multi *mul,int n,eqnode *eqn);
extern int MaddName(Multi *mul,int n,char *name,int dim);
extern int MrmName(Multi *mul,int n);
extern int Mget_type(Multi *mul,char *name);
extern int MaddRequest(Multi *mul,char *name);
extern int Mcount_eqn(eqnode *base, Multi *mul);

/* PUBLIC DEFINITIONS */

extern int Minit(Multi *mul);
extern int Mclear(Multi *mul);
extern int MfindOpts(Multi *mul);
extern int McombineTop(Multi *mul);
extern int MfindNames(Multi *mul);
extern int McheckDims(Multi *mul);
extern int McalcDerivs(Multi *mul);
extern int McalcRPEs(Multi *mul);
extern int MstartEval(Multi *mul);
extern double *MevalTop2(Multi *mul,int n);
extern double *MevalTopDeriv(Multi *mul,int n,int d);
extern double *MevalTop(Multi *mul,int n);
extern double *MevalDeriv(Multi *mul,int n);
extern double *MevalTopCB(Multi *mul,int n,void (*fun)());
extern double *MevalTopDerivCB(Multi *mul,int n,int d,void (*fun)());

/* IO */

extern int fscanMulti(FILE *fp,Multi *mul);
extern int fscanMultiParams(FILE *fp,Multi *mul);
extern void fprint_multi(FILE *fp,Multi *mul);
extern void dump_multi(FILE *fp,Multi *mul);
extern void fprintMvals(FILE *fp,Multi *mul);
extern void fprint_Mopts(FILE *fp,Multi *mul);
extern void fprintMdep(FILE *fp,Multi *mul);
extern void fprintMres(FILE *fp,Multi *mul);

/* Get-Set's */

extern int MsetNtop(Multi *mul,int n);
extern int MsetTopDim(Multi *mul,int n,int dim);
extern int MsetNtopDerivs(Multi *mul, int n,int nDeriv);
extern int MsetDerivName(Multi *mul,int n,int deriv,char *name);
extern int MaddDerivName(Multi *mul,int n,char *name);

extern int Mget_n_params(Multi *mul);
extern char *Mget_param_name(Multi *mul,int i);
extern double Mget_param_val(Multi *mul,int i);
extern int Mset_param_val(Multi *mul,int i,double val);
extern int Mset_param_val_by_name(Multi *mul,char *name,double val);
extern int Mwhich_param(Multi *mul,char *name);

extern int MgetNvars(Multi *mul);
extern int MsetNvars(Multi *mul,int n);
extern int Mset_var_name(Multi *mul,int i,char *name);
extern char *Mget_var_name(Multi *mul,int i);
extern double Mget_var_min(Multi *mul,int i);
extern double Mget_var_max(Multi *mul,int i);
extern double Mget_var_val(Multi *mul,int i);
extern int MaddVar(Multi *mul,char *name);
extern int Mset_var_minmax(Multi *mul,int i,double min,double max);
extern int Mset_var_val(Multi *mul,int i,double val);
extern int Mwhich_var(Multi *mul,char *name);

extern int Madd_opt(Multi *mul,char *name,double val);
extern int Mset_opt_val_by_name(Multi *mul,char *name,double val);
extern double Mget_opt_val_by_name(Multi *mul,char *name);
extern int Mmatch_opt(Multi *mul,char *name);



/* Mdiff.c private */

extern	void	Mdiff_wrt(Multi *mul,eqnode *base,char *var_name);
extern	void	Mdiff_fun_wrt(Multi *mul,eqnode *base,char *name);
extern	int	Mconvert_diff(Multi *mul,eqnode *eqn);
extern  int MconvertSum(Multi *mul,eqnode *base,int where);

/* Mvrpe.c private */

extern	Mvrpeint *Mmake_vrpe(Multi *mul,eqnode *eqn);
extern	void	print_Mvrpe(Mvrpeint *rpe,Multi *mul);
extern	void	fprint_Mvrpe(FILE *fp, Mvrpeint *rpe,Multi *mul);
extern	void	eprint_Mvrpe(Mvrpeint *rpe,Multi *mul);
extern	double	*Meval_vrpe(Mvrpeint *rpe,double *vars);
extern	void	clear_Mvrpe_const();

#endif
