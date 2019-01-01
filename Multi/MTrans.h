#include <stdio.h>
#include "eqn.h"
#include "Multi.h"

/* Whether a var in op coresponds to a TOP VAR or DERIV in tgt */

#define MT_TOP 1
#define MT_VAR 2
#define MT_DERIV 3
#define MT_IGNORE 4
#define MT_CONST 5
#define MT_INPUT 6

/* describes how to transform from one multi to an other */

typedef struct MTrans {
	Multi	*op,*tgt;
	int	varType[MAX_NUM_VARS];
	int	varTrans[MAX_NUM_VARS];
	int	varEle[MAX_NUM_VARS];
	int	varDeriv[MAX_NUM_VARS];
	double	varConst[MAX_NUM_VARS];
	int	paramTrans[MAX_NUM_PARAMS];
} MTrans;

extern int MTdefine(MTrans *trans,Multi *op,Multi *tgt);
extern int MTsetVarTop(MTrans *trans,int opVar,int topNum, int topEle);
extern int MTsetVarVar(MTrans *trans,int opVar,int tgtVar);
extern int MTsetVarIgnore(MTrans *trans,int opVar);
extern int MTsetVarInput(MTrans *trans,int opVar);
extern int MTsetVarConst(MTrans *trans,int opVar,double val);
extern int MTcalcVarTrans(MTrans *trans);
extern int MTcalcParamTrans(MTrans *trans);
extern int MTcopyParam(MTrans *trans);
extern int MTevalForTop(MTrans *trans,int topNum);
extern int MTevalForTopDeriv(MTrans *trans,int topNum,int derivNum);
extern int MTsetIngrVar(MTrans *trans,int i,double val);
extern double MTevalForVar(MTrans *trans,int i);
extern void MTstdCallback(MTrans *trans,short  *prov_req,double *vals);
extern int MTcopyVars(MTrans *trans);
extern int MTcheck(MTrans *trans); /* 0 if error */
extern void dumpMTrans(FILE *fp,MTrans *trans);
