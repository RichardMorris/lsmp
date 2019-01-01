#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eqn.h"
#include "Multi.h"
#include "MTrans.h"

/* Some macros adapted from Multi.c */

#define OP_GET_DEPVAR(i,j) \
	( (trans->op->depVar[i][(j)/8] ) & ( 0x01 << ((j)%8) ) )

#define OP_GET_DONE(i) ( trans->op->done[(i)/8] & ( 0x01 << ((i)%8) ) )

#define TGT_GET_DEPVAR(i,j) \
	( (trans->tgt->depVar[i][(j)/8] ) & ( 0x01 << ((j)%8) ) )

#define TGT_GET_DONE(i) ( trans->tgt->done[(i)/8] & ( 0x01 << ((i)%8) ) )


int MTdefine(MTrans *trans,Multi *op,Multi *tgt)
{
	int i;

	trans->op = op;
	trans->tgt = tgt;
	for(i=0;i<MAX_NUM_VARS;++i)
	{
		trans->varType[i] = 0;
		trans->varTrans[i] = 0;
		trans->varEle[i] = 0;
		trans->varConst[i] = 0.0;
	}
	for(i=0;i<MAX_NUM_PARAMS;++i)
	{
		trans->paramTrans[i] = -1;
	}
	return(TRUE);
}

/* variable, tops, and parameters are numbered from 0 */

int MTsetVarTop(MTrans *trans,int opVar,int topNum, int topEle)
{
	trans->varType[opVar] = MT_TOP;
	trans->varTrans[opVar] = topNum;
	trans->varEle[opVar] = topEle;
	return(TRUE);
}

int MTsetVarVar(MTrans *trans,int opVar,int tgtVar)
{
	trans->varType[opVar] = MT_VAR;
	trans->varTrans[opVar] = tgtVar;
	trans->varEle[opVar] = 0;
	return(TRUE);
}

int MTsetVarIgnore(MTrans *trans,int opVar)
{
	trans->varType[opVar] = MT_IGNORE;
	return(TRUE);
}

int MTsetVarConst(MTrans *trans,int opVar,double val)
{
	trans->varType[opVar] = MT_CONST;
	trans->varConst[opVar] = val;
	return(TRUE);
}

int MTsetVarInput(MTrans *trans,int opVar)
{
	trans->varType[opVar] = MT_INPUT;
	return(TRUE);
}

/* Must be called before calcDerivs on target */

int MTcalcVarTrans(MTrans *trans)
{
	int i,headNum,tailNum,topNum;
	char	*str,*ptr,temp[256];

	for(i=0;i<trans->op->n_vars;++i)
	{
	    if(trans->varType[i] == 0)
	    {
		/* Got an unset variable must be a derivative */

		str = Mget_var_name(trans->op,i);
		ptr = strchr(str,'@');
		if(ptr == NULL)
		{
			eprintf("Whoopse! can't translate %s\n",str);
			eprintf("it should contain an '@'\n");
			return(FALSE);
		}
		headNum = atoi(str+1)-1; /* note ?1 is vbl 0 */
		if(trans->varType[headNum] == MT_VAR 
		 ||trans->varType[headNum] == MT_IGNORE )
		{
			trans->varType[i] = MT_CONST;
			*ptr = '\0';
			if( !strcmp(str,ptr+1) ) /* dx/dx */
				trans->varConst[i] = 1.0;
			else
				trans->varConst[i] = 0.0;
			*ptr = '@';
			continue;
		}
		else if(trans->varType[headNum] != MT_TOP)
		{
			eprintf("Whoopse! can't translate %s\n",str);
			eprintf("the head does not corespond to an equation\n");
			eprintf("headNum %d type %d\n",
				headNum,trans->varType[headNum]);
			return(FALSE);
		}
		topNum = trans->varTrans[headNum];

		/* Now convert the tail */

		*temp = '\0';
/*CONSTCOND*/
		while(1)
		{
			++ptr;
			tailNum = atoi(ptr+1)-1;
			if(trans->varType[tailNum] != MT_VAR)
			{
			eprintf("Whoopse! can't translate %s\n",str);
			eprintf("the tail does not corespond to a variable\n");
			return(FALSE);
			}
	
			strcat(temp,
				Mget_var_name(trans->tgt,
					trans->varTrans[tailNum] ));
		
			ptr = strchr(ptr+1,'@');
			if(ptr == NULL) break;

			strcat(temp,"@");
		}
		trans->varType[i] = MT_DERIV;
		trans->varTrans[i] = topNum;
		trans->varEle[i] = trans->varEle[headNum];
		trans->varDeriv[i] = 
			MaddDerivName(trans->tgt,topNum,temp);

	    }
	}
	return(TRUE);
}

int MTcalcParamTrans(MTrans *trans)
{
	int	nParams,i;
	char	*str;

	nParams = Mget_n_params(trans->op);
	for(i=0;i<nParams;++i)
	{
		str = Mget_param_name(trans->op,i);
		trans->paramTrans[i] =
			Mwhich_param(trans->tgt,str+1);
	}
	return(TRUE);
}


int MTcopyParam(MTrans *trans)
{
	int nParams,i;

        nParams = Mget_n_params(trans->op);
        for(i=0;i<nParams;++i)
		if( trans->paramTrans[i] >= 0 )
			Mset_param_val(trans->op,i,
				Mget_param_val(trans->tgt,
					trans->paramTrans[i]));
	return(TRUE);
}

/* evaluate all the derivatives in target which may be used by
	the specified top eqn in the op */

int MTevalForTop(MTrans *trans,int topNum)
{
	int i,n,flag=TRUE;
	double *ptr;

	n = trans->op->n_vars;
	for(i=0;i<n;++i)
	{
	    if( !OP_GET_DEPVAR(topNum,i) ) continue;
	    switch(trans->varType[i])
	    {
	    case MT_TOP: 
		ptr = MevalTop2(trans->tgt,trans->varTrans[i]);
		break;
	    case MT_DERIV:
		ptr = MevalTopDeriv(trans->tgt,
			trans->varTrans[i],trans->varDeriv[i]);
		break;
	    case MT_VAR: case MT_IGNORE: case MT_CONST: case MT_INPUT:
		break;
	    default:
		eprintf("MTEvalForTop: Bad trans #%d = %d\n",i,trans->varType[i]);
		flag = FALSE;
		break;
	    }
	}
	return(flag);
}

int MTevalForTopDeriv(MTrans *trans,int topNum,int derivNum)
{
	int i,j,n,flag=TRUE;
	double *ptr;

	j = trans->op->n_top + trans->op->top_deriv_ref[topNum] + derivNum;
	n = trans->op->n_vars;
	for(i=0;i<n;++i)
	{
	    if( !OP_GET_DEPVAR(j,i) ) continue;
	    switch(trans->varType[i])
	    {
	    case MT_TOP: 
		ptr = MevalTop2(trans->tgt,trans->varTrans[i]);
		break;
	    case MT_DERIV:
		ptr = MevalTopDeriv(trans->tgt,
			trans->varTrans[i],trans->varDeriv[i]);
		break;
	    case MT_VAR: case MT_IGNORE: case MT_CONST: case MT_INPUT:
		break;
	    default:
		eprintf("MTEvalForTopDeriv: Bad trans #%d = %d\n",i,trans->varType[i]);
		break;
		flag = FALSE;
	    }
	}
	return(flag);
}

int MTsetIngrVar(MTrans *trans,int i,double val)
{
	int flag=TRUE;

	    switch(trans->varType[i])
	    {
	    case MT_VAR:
/*
printf("Mset-var_val %d\n",trans->varTrans[i]);
*/
		Mset_var_val(trans->tgt,trans->varTrans[i],val);
			break;
		break;
	    case MT_TOP: 
	    case MT_DERIV:
	    case MT_CONST:
	    case MT_IGNORE:
		break;
	    default:
		eprintf("MTSetIngrVar: Bad trans #%d = %d\n",i,trans->varType[i]);
		break;
		flag = FALSE;
	    }
	return(flag);
}

int MTsetOpVar(MTrans *trans,int i,double val)
{
	int flag=TRUE;

	    switch(trans->varType[i])
	    {
	    case MT_INPUT:
/*
printf("Mset-var_val %d\n",trans->varTrans[i]);
*/
		Mset_var_val(trans->op,i,val);
			break;
		break;
	    case MT_IGNORE:
	    case MT_TOP: 
	    case MT_DERIV:
	    case MT_CONST:
		break;
	    default:
		eprintf("MTSetIngrVar: Bad trans #%d = %d\n",i,trans->varType[i]);
		break;
		flag = FALSE;
	    }
	return(flag);
}

double MTevalForVar(MTrans *trans,int i)
{
	double *ptr,val = 0.0;

	    switch(trans->varType[i])
	    {
	    case MT_TOP: 
		ptr = MevalTop2(trans->tgt,trans->varTrans[i]);
		val = *(ptr + trans->varEle[i]);
/*
printf("MevalTop2 %d val %f\n",trans->varTrans[i],val);
*/
		break;
	    case MT_DERIV:
/*
printf("MevalTopDeriv %d\n",trans->varTrans[i],trans->varDeriv[i]);
*/
		ptr = MevalTopDeriv(trans->tgt,
			trans->varTrans[i],trans->varDeriv[i]);
		val = *(ptr + trans->varEle[i]);
		break;
	    case MT_VAR: case MT_IGNORE: case MT_INPUT:
		break;
	    case MT_CONST:
		val = trans->varConst[i];
		break;
	    default:
		eprintf("MTevalForVar: Bad trans #%d = %d\n",i,trans->varType[i]);
		break;
	    }
	return(val);
}

void MTstdCallback(MTrans *trans,short  *prov_req,double *vals)
{
        int i;
	double	val;
	int	flag = 0;

/*
fprintf(stderr,"Callback\n");
        fprintMvals(stderr,trans->op);
        fprintMvals(stderr,trans->tgt);
*/
        for(i=0;i<trans->op->n_vars;++i)
        {
                if(prov_req[i]>0)
                {
/*
                        fprintf(stderr,"prov  %d val %f\n",i,vals[i]);
*/
			val = Mget_var_val(trans->tgt,trans->varTrans[i]);
			if(val != vals[i] )
			{
				flag = 1;
                        	MTsetIngrVar(trans,i,vals[i]);
			}
                }
        }
        if( flag ) MstartEval(trans->tgt);
        for(i=0;i<trans->op->n_vars;++i)
        {
                if(prov_req[i]<0)
                {
                        vals[i] = MTevalForVar(trans,i);
/*
                        fprintf(stderr,"req %d val %f\n",i,vals[i]);
*/
                }
        }
}

/* Copies all the variable */

int MTcopyVars(MTrans *trans)
{
	int i,j,n,flag=TRUE;

	n = trans->op->n_vars;
	for(i=0;i<n;++i)
	{
		switch(trans->varType[i])
		{
		case MT_TOP:
			if( trans->varEle[i] == -1 ) break;
			trans->op->bigvals[i] =
			    *(trans->tgt->results 	
				+ trans->tgt->top_res_ref[ trans->varTrans[i] ] 
				+ trans->varEle[i] );
			break;
	        case MT_DERIV:
			if( trans->varEle[i] == -1 ) break;
			j = trans->varTrans[i];
			trans->op->bigvals[i] =
			    *(trans->tgt->results 	
				+ trans->tgt->top_res_ref[j] 
				+ ( 1 + trans->varDeriv[i] ) * 
					abs(trans->tgt->top_dims[j])
				+ trans->varEle[i] );
			break;
		case MT_CONST:
			trans->op->bigvals[i] = trans->varConst[i];
			break;
		case MT_VAR: case MT_IGNORE: case MT_INPUT:
			break;
		default:
			eprintf("Bad trans #%d = %d\n",i,trans->varType[i]);
			flag= FALSE;
			break;
		}
	}
	return(flag);
}
		
void dumpMTrans(FILE *fp,MTrans *trans)
{
	int i,n;

	fprintf(fp,"TRANSLATION VARIABLES\n");
	n = MgetNvars(trans->op);
	for(i=0;i<n;++i)
	{
		fprintf(fp,"%s -> ",Mget_var_name(trans->op,i));
		switch(trans->varType[i])
		{
		case MT_TOP: 
			fprintf(fp,"TOP num %d ele %d\n",
				trans->varTrans[i],trans->varEle[i]);
			break;

		case MT_VAR:
			fprintf(fp,"VAR num %d = %s\n",
				trans->varTrans[i],
				Mget_var_name(trans->tgt,trans->varTrans[i]));
			break;

		case MT_DERIV:
			fprintf(fp,"DERIV top %d deriv %d ele %d\n",
				trans->varTrans[i],
				trans->varDeriv[i],
				trans->varEle[i]);
			break;

		case MT_IGNORE:
			fprintf(fp,"IGNORE\n");
			break;

		case MT_CONST:
			fprintf(fp,"CONST %f\n",trans->varConst[i]);
			break;

		case MT_INPUT:
			fprintf(fp,"INPUT\n");
			break;

		default:
			fprintf(fp,"BAD code %d\n",
				trans->varType[i]);
		}
	}
	fprintf(fp,"PARAMETERS\n");
	n = Mget_n_params(trans->op);
	for(i=0;i<n;++i)
	{
		if( trans->paramTrans[i] > 0 )
		fprintf(fp,"%s -> %s\n",
			Mget_param_name(trans->op,i),
			Mget_param_name(trans->tgt,
				trans->paramTrans[i]));
	}
}

int MTcheck(MTrans *trans)
{
	int i,n;

	n = MgetNvars(trans->op);
	for(i=0;i<n;++i)
	{
		switch(trans->varType[i])
		{
		case MT_TOP: 
		case MT_VAR:
		case MT_DERIV:
		case MT_CONST:
		case MT_IGNORE:
		case MT_INPUT:
			break;
		default:
	eprintf("Miss match between definitions\n");
		return(0);
		}
	}
	return(1);
}
