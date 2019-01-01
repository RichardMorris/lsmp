#include <stdio.h>
#include <math.h>
#include <eqn.h>
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define MAXORDERT2 2*MAXORDER
#define MAXORDERP2 MAXORDER+2


/******************* RMAR GLOBALS ***********************************/

#define  numberofbitsinmantissaofrealnumber 27 /* at least for the */
/* purposes of drawing */
/*
unsigned int  RESOLUTION= 1000, NUM_REGIONS =  80;
*/
unsigned int LINK_FACE_LEVEL = 512, RESOLUTION = 32;

extern int vrml_version;

int    xord, yord;
                                     /* region of space to rmardraw in */
double small;
double region;
char   *name;

int	global_colour;	/* Which colour to draw in */

/****** vars todo with plotting ****/
char vect_file_name[L_tmpnam],ifs_coords_file_name[L_tmpnam],
        ifs_index_file_name[L_tmpnam];
FILE *vect_file,*ooglfile,*ifs_coords_file,*ifs_index_file;

int facet_vertex_count;		/*** The number of verticies on a facet ***/
int total_line_count;
int total_point_count;


/*********************** RMAR MAIN ******************************************/
/*rmardraws an arbitrary polynomial curve in a specified region of x,y space*/

int marmain(a,xmin,xmax,ymin,ymax)
double xmin,xmax,ymin,ymax;
double a[MAXORDER][MAXORDER];  /* the input polynomial */
{
  double b[MAXORDER][MAXORDER];  /* its bernstein form */

  small=(xmax-xmin)/LINK_FACE_LEVEL;
  region=(xmax-xmin)/RESOLUTION;

  order_poly2(a,&xord,&yord);
  formbernstein(a,b,xmin,xmax,ymin,ymax);

#ifdef VERB
  printbernstein(a);
  printf("orders %d %d max %d\n",xord,yord,MAXORDER);
  printf("First bernstein polynomial\n");
  printbernstein(b);
#endif
  if( xord == 0 && yord == 0 )
  {
	fprintf(stderr,"\007Warning: constant function value %f\n",a[0][0]);
	return;
  }
  generatecurve(b,xmin,xmax,ymin,ymax);
  return(1);
}


/*************************** RMAR SUBROUTINES ********************************/

printbernstein(b)
double b[MAXORDER][MAXORDER];
{
  int	 i,j;
  for( i=0; i<= xord; ++i )
  {
    for( j=0; j<=xord; ++j )
    {
        printzero(b[i][j]); printf(" ");
    }
    printf("\n");
  }
}

int comb(n,m)	/* rewritten in double so can get up to 13! */
int n,m;
{
   register int j;
   register double a;

   a = 1.0;
   for(j=m+1;j<=n;j++) a *= (double) j; 
   for(j=1;j<=n-m;j++) a /= (double) j;
   j = (int) (a+0.001); 
   if( fabs( a - j ) > 1.0e-3 ) fprintf(stderr,"comb: error n %d m %d %.12f %d\n",n,m,a,j);
   return(j);
}

error2(power)
int power;
{
   fprintf(stderr,"input power too large: %d\n",power);
   exit(2);
}

allonesign(array,isize,jsize)
double array[MAXORDER][MAXORDER];
int isize,jsize;
{
   /* checks if every element of a 2d array has strictly the same sign */
   register int i,j;
   register int sign = (array[0][0] < 0);

   for(i=0;i<=isize;i++)
      for(j=0;j<=jsize;j++)
         if( (array[i][j] == 0.0) ||
	     (array[i][j] < 0) != sign ) return(FALSE);
   return(TRUE);
}

onedallonesign(array,isize)
double array[MAXORDER];
int isize;
{
   /* checks if every element of a 1d array has strictly the same sign */
   register int i;
   register int sign = (array[0] < 0);

   for(i=0;i<=isize;i++)
      if( (array[i] == 0.0) ||
          (array[i] < 0) != sign) return(FALSE);
   return(TRUE);
}

/**************************************************************/
/*                                                            */
/*     input 'aa'   an array such that aa(i,j) is coeff of    */
/*                  x^i y^j                                   */
/*                                                            */
/**************************************************************/

formbernstein(aa,bb,xmin,xmax,ymin,ymax)
double aa[MAXORDER][MAXORDER],bb[MAXORDER][MAXORDER];
double xmin,xmax,ymin,ymax;
{
   static double d[MAXORDER][MAXORDERP2][MAXORDERP2];
   int col,row;
   int i,j,k;

   for( col = 0 ; col <= xord; col++)  /**** loop thru powers of x ****/
   {
      d[0][col+1][1]=aa[col][yord];

      for( i = 1 ; i <= yord;i++)
      {
         d[i-1][col+1][0] = 0;
         d[i-1][col+1][i+1] = 0;

         for( j = 0 ;j <= i;j++)
	 {
            d[i][col+1][j+1]= ymax*d[i-1][col+1][j]+
                ymin*d[i-1][col+1][j+1]+
                comb(i,j)*aa[col][yord-i];
	 }
      };

      for( k = 0 ; k<= yord;k++)
      {
         bb[col][k] = d[yord][col+1][k+1] / comb(yord,k);
      }
   };

   for( row = 0 ; row<= yord ;row++)
   {
      d[0][1][row+1] = bb[xord][row];
      for( i = 1 ; i<= xord;i++)
      {
         d[i-1][0][row+1] = 0;
         d[i-1][i+1][row+1] = 0;

         for( j = 0 ; j<= i;j++)
            d[i][j+1][row+1] = xmax * d[i-1][j][row+1] +
                xmin * d[i-1][j+1][row+1] +
                comb(i,j) * bb[xord-i][row];
      };

      for( k = 0 ; k<= xord;k++)
         bb[k][row] = d[xord][k+1][row+1]/comb(xord,k);
   }
}

generatecurve(bb, minx, maxx, miny, maxy)
double bb[MAXORDER][MAXORDER];
double minx,maxx,miny,maxy;
{
   double b1[MAXORDER][MAXORDER],b2[MAXORDER][MAXORDER];
   double b3[MAXORDER][MAXORDER],b4[MAXORDER][MAXORDER];
   double xderiv[MAXORDER][MAXORDER];
   double yderiv[MAXORDER][MAXORDER];
   double xhalf,yhalf;

   if( !allonesign(bb,xord,yord))
   {
      difx(bb,xderiv);
      dify(bb,yderiv);
      if(allonesign(xderiv,xord-1,yord)&&allonesign(yderiv,xord,yord-1))
         rmardraw(bb,minx,maxx,miny,maxy);
      else{
         if( maxx - minx < small)
            rmardraw(bb,minx,maxx,miny,maxy);
/*
            rmar_line(minx,maxx,miny,maxy);
*/
         else{
            reduce(bb,b1,b2,b3,b4);
            xhalf = (minx + maxx) / 2;
            yhalf = (miny + maxy) / 2;
            generatecurve(b1,minx,xhalf,miny,yhalf);
            generatecurve(b2,minx,xhalf,yhalf,maxy);
            generatecurve(b3,xhalf,maxx,miny,yhalf);
            generatecurve(b4,xhalf,maxx,yhalf,maxy);
         }
      }
   }
}

/************************************************************************/
/*									*/
/*	Now some routines for drawing the facets			*/
/*									*/
/************************************************************************/

initoogl(colour)
int	colour;
{

	if(vrml_version != 3)
	{
		tmpnam(vect_file_name);
		vect_file = fopen(vect_file_name,"w");
	}
	else
	{
	        tmpnam(ifs_coords_file_name);
        	tmpnam(ifs_index_file_name);
        	ifs_coords_file = fopen(ifs_coords_file_name,"wb");
        	ifs_index_file = fopen(ifs_index_file_name,"wb");
	}

	total_point_count = 0;
	total_line_count = 0;
	ooglfile = stdout;
	global_colour = colour;
}

/*
 * Function:	rewindoogl
 * Action:	When output goes to standard output and more than
 *		model is produced it is imposible to rewind the standard
 *		output. One way round is to delay the creation of the
 *		ooglfile untill the program has ended. This comand
 *		is called intermediatly between models to clear out
 *		the vect and quad tempory vector files.
 */

rewindoogl()
{
	if(vrml_version != 3)
	{
		fclose(vect_file);
		vect_file = fopen(vect_file_name,"w");
	}
	else
	{
		fclose(ifs_coords_file);
		fclose(ifs_index_file);
        	ifs_coords_file = fopen(ifs_coords_file_name,"wb");
        	ifs_index_file = fopen(ifs_index_file_name,"wb");
	}
	total_point_count = 0;
	total_line_count = 0;
}

/*
 * Function:	finiflush
 * Action:	convert all the data into oogl and write it to the
		ooglfile. Called when the model is complete 'finioogl'
		or when a flush is required 'flushoogl'
 */

finiflushoogl()
{
	float x,y,z,vert[2];
	int nlines,nverticies,ncols,num;

	/* First we have to read through the temp file to find the
		number of lines */

	fclose(vect_file);
	vect_file = fopen(vect_file_name,"r");

	/* find number of lines and total num of verticies */

	nlines = nverticies = 0;
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		++nlines;
		nverticies += num;
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f",&x,&y);
			--num;
		}
	}
	if( global_colour == -1 ) ncols = 0;
        else                      ncols = nlines;

	if( nverticies != 0 )
	{
	fprintf(ooglfile,"{ = VECT\n");
	fprintf(ooglfile,"%d %d %d\n",nlines,nverticies,ncols);

	/* find number of verticies in each line */

	fprintf(ooglfile,"# the number of verticies in each line \n");
	rewind(vect_file);
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		fprintf(ooglfile,"%d\n",num);
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f",&x,&y);
			--num;
		}
	}

	/* printf the number of colours for each line */

	fprintf(ooglfile,"# the number of colours for each line \n");
        if( ncols > 0 )
                for(num=0;num < nlines; ++num) fprintf(ooglfile,"1\n");
        else
                for(num=0;num < nlines; ++num) fprintf(ooglfile,"0\n");

	/* print the verticies */
	
	fprintf(ooglfile,"# The verticies\n");
	rewind(vect_file);
	while( fscanf(vect_file,"%d",&num) != EOF )
	{
		while( num > 0 )
		{
			fscanf(vect_file,"%f %f",&x,&y);
			map(&x,&y,&z);
			fprintf(ooglfile,"%f %f %f\n",x,y,z);
			--num;
		}
	}

	/* now print the colours */

	fprintf(ooglfile,"# The colours\n");
	for(num=1; num <= nlines; ++num)
	{
		switch(global_colour)
		{
		case 0: /* Black   */ fprintf(ooglfile,"0 0 0 1\n"); break;
		case 1: /* Red     */ fprintf(ooglfile,"1 0 0 1\n"); break;
		case 2: /* Green   */ fprintf(ooglfile,"0 1 0 1\n"); break;
		case 3: /* Yellow  */ fprintf(ooglfile,"1 1 0 1\n"); break;
		case 4: /* Blue    */ fprintf(ooglfile,"0 0 1 1\n"); break;
		case 5: /* Magenta */ fprintf(ooglfile,"1 0 1 1\n"); break;
		case 6: /* Cyan    */ fprintf(ooglfile,"0 1 1 1\n"); break;
		case 7: /* White   */ fprintf(ooglfile,"1 1 1 1\n"); break;
		}
	}
	
	fprintf(ooglfile,"}\n");
	}
	fflush(ooglfile);
	fclose(vect_file);
}

finiflushjvx()
{
	float x,y,z,vert[2];
	int nlines,nverticies,ncols,num;
	int indicies[2];

	/* First we have to read through the temp file to find the
		number of lines */

	fclose(ifs_coords_file);
	fclose(ifs_index_file);
       	ifs_coords_file = fopen(ifs_coords_file_name,"rb");
       	ifs_index_file = fopen(ifs_index_file_name,"rb");

fprintf(ooglfile,"  <geometry name=\"acurve\">\n");
#ifdef SHOW_VERTICES
fprintf(ooglfile,"   <pointSet dim=\"2\" point=\"show\">\n");
#else
fprintf(ooglfile,"   <pointSet dim=\"2\" point=\"hide\">\n");
#endif
fprintf(ooglfile,"    <points>\n");

	for(num=0;num<total_point_count;++num)
	{
		fread((char *) vert,sizeof(float),2,ifs_coords_file);
                fprintf(ooglfile,"<p>%f %f</p>\n",
                                        vert[0],vert[1]);
        }
        fclose(ifs_coords_file);

fprintf(ooglfile,"    </points>\n");
fprintf(ooglfile,"   </pointSet>\n");
fprintf(ooglfile,"   <lineSet line=\"show\">\n");
fprintf(ooglfile,"    <lines>\n");

	for(num=0;num<total_line_count;++num)
	{
		fread((char *) indicies,sizeof(int),2,ifs_index_file);
                fprintf(ooglfile,"<l>%d %d</l>\n",
                                        indicies[0],indicies[1]);
        }
	fclose(ifs_index_file);
fprintf(ooglfile,"    </lines>\n");
fprintf(ooglfile,"   </lineSet>\n");
fprintf(ooglfile,"  </geometry>\n");
	fflush(ooglfile);
}

finiflush()
{
	if(vrml_version == 3)
		finiflushjvx();
	else
		finiflushoogl();
}

flushoogl()
{
	finiflush();
	fopen(vect_file_name,"a");
}

finioogl()
{
	finiflush();
	unlink(vect_file_name);
}

rmar_point(x1,y1)
double x1,y1;
{
	if(vrml_version == 3)
	{
		float pos[2];
		pos[0] = x1; pos[1] = y1;
		fwrite((void *) pos, sizeof(float),2,ifs_coords_file);
		++total_point_count;
	}
	else
	{
		fprintf(vect_file,"1 %f %f\n",x1,y1);
	}
}

rmar_line(x1,x2,y1,y2)
double x1,x2,y1,y2;
{
	if(vrml_version == 3)
	{
		float pos[2]; int indicies[2];
		pos[0] = x1; pos[1] = y1;
		fwrite((void *) pos, sizeof(float),2,ifs_coords_file);
		pos[0] = x2; pos[1] = y2;
		fwrite((void *) pos, sizeof(float),2,ifs_coords_file);
		indicies[0] = total_point_count++;
		indicies[1] = total_point_count++;
		fwrite((void *) indicies, sizeof(int),2,ifs_index_file);
		++total_line_count;
	}
	else
	{
		fprintf(vect_file,"2 %f %f ",x1,y1);
		fprintf(vect_file,"%f %f\n",x2,y2);
	}
}

difx(bb,xderiv)
double bb[MAXORDER][MAXORDER];
double xderiv[MAXORDER][MAXORDER];
{
   int row,element;

     for(row=0;row<=yord;row++)
   for(element=0;element<=xord-1;element++)
      xderiv[element][row] = bb[element+1][row] - bb[element][row];
}

dify(bb,yderiv)
double bb[MAXORDER][MAXORDER];
double yderiv[MAXORDER][MAXORDER];
{
   int col,element;

   for(col=0;col<=xord;col++)
      for(element=0;element<=yord-1;element++)
    yderiv[col][element] = bb[col][element+1] - bb[col][element];
}

reduce(bb,b1,b2,b3,b4)
double bb[MAXORDER][MAXORDER];
double b1[MAXORDER][MAXORDER],b2[MAXORDER][MAXORDER],
b3[MAXORDER][MAXORDER],b4[MAXORDER][MAXORDER];
{
   double pyramid[MAXORDERT2][MAXORDERT2];
   int col,row;
   int level;

   for(col=0;col<=xord;col++)
      for(row=0;row<=yord;row++)
         pyramid[2*col][2*row] = bb[col][row];

   for(level=1;level<=yord;level++)
      for(col=0;col<=2*xord;col+=2)
         for(row=level;row<=2*yord-level;row+=2)
            pyramid[col][row] = 0.5 * ( pyramid[col][row-1] +
                pyramid[col][row+1] );

   for(level=1;level<=xord;level++)
      for(col=level;col<= 2*xord -level;col+=2)
         for(row=0;row<=2*yord;row++)
            pyramid[col][row] = 0.5 * ( pyramid[col-1][row] +
                pyramid[col+1][row] );

   for(col=0;col<=xord;col++)
      for(row=0;row<=yord;row++)
      {
         b1[col][row] = pyramid[col][row];
         b2[col][row] = pyramid[col][row+yord];
         b3[col][row] = pyramid[col+xord][row];
         b4[col][row] = pyramid[col+xord][row+yord];
      }
}

double bernstein(bb,order,root)
double bb[];
int order;
double root;
{
   double work[MAXORDERT2];
   double oneminusroot=1.0-root;
   register int element,level;


   for(element=0;element<=order;element++)
      work[2*element] = bb[element];

   for(level=1;level<=order;level++)
      for(element=level;element<=2*order-level;element+=2)
         work[element] = oneminusroot * work[element-1] +
             root * work[element+1];

   return(work[order]);
}

solve(bb,order,l,h,root)
double bb[MAXORDER];
int order;
double l,h;
double *root;
{
   double rootm; 
   double valm;
   double rootl=0.0; 
   double rooth=1.0;
   double vall=bb[0];
   double bernstein();
   int i;

   if(bb[0] == 0.0)
   {
      *root = l;
      return(TRUE);
   }
   if(bb[order] == 0.0) /* This prevents corner points being found twice */
   {
      *root = h;
      return(FALSE); 
   }
   else if(onedallonesign(bb,order)) return(FALSE);
   else
      for(i=1;i<=numberofbitsinmantissaofrealnumber;i++)
      {
         rootm = (rootl +rooth) * 0.5;
         valm = bernstein(bb, order, rootm);
         if((vall<0) != (valm<0)) rooth = rootm;
         else
         {
            vall = valm; 
            rootl = rootm;
         }
      }
   *root = (1.0-rootm)*l + rootm*h;
/*
   return(rooth != 1.0 );  
*/
/*****
*     This was originally included to prevent points at the corners being
*     found twice. However if the solution is infinitesimally close to the
*     corner and bb[order] is not 0.0 then the corner point will not be 
*     selected when an other side is tested so we return TRUE here.
*****/

   return(TRUE);
}

printzero(x)
double x;
{
  if( x >= 0.0000005 || x <= -0.0000005 )  printf("%9.6f ",x);
  else if( x == 0.0 )                      printf(" 0        ");
  else if( x  > 0.0 )                      printf("+0        ");
  else if( x  < 0.0 )                      printf("-0        ");
  else					   printf("! %f ",x);
}

/****
*    Returns true if bb is all one sign apart from one point
****/
int onezero2D(bb,xord,yord)
double bb[MAXORDER][MAXORDER]; int xord,yord;
{
  int num_zeros=0,num_plus=0,num_neg=0,i,j;

  for( i=0; i<=xord; ++i )
    for( j=0; j<=yord; ++j )
      if( bb[i][j] == 0.0 ) ++num_zeros;
      else if( bb[i][j] > 0.0 ) ++num_plus;
      else                      ++num_neg;

  return( num_zeros == 1 && ( num_plus == 0 || num_neg == 0 ) );
}

rmardraw(bb,xl,xh,yl,yh)
double bb[MAXORDER][MAXORDER];
double xl,xh,yl,yh;
{
   double b1[MAXORDER][MAXORDER],b2[MAXORDER][MAXORDER];
   double b3[MAXORDER][MAXORDER],b4[MAXORDER][MAXORDER];
   double bottom[MAXORDER],top[MAXORDER];
   double left[MAXORDER],right[MAXORDER];
   double xend[2],yend[2];
   double root;
   double xhalf,yhalf;
   int i,j;
   int end= -1;
   double bot0,bot1,top0,top1,lef0,lef1,rht0,rht1;

   if((xh -xl) < region)
   {

   /******  if contained in 1/20th of screen ******/

      for(i=0;i<=xord;i++)
      {
         bottom[i] = bb[i][0];
         top[i] = bb[xord-i][yord];
      }
      for(j=0;j<=yord;j++)
      {
         right[j] = bb[xord][j];
         left[j] = bb[0][yord-j];
      }
   /****** these go anti-clockwise round square *****/

        if(solve(bottom,xord,xl,xh,&root))
        {
           xend[++end] = root;
           yend[end] =  yl;
        }
        if(solve(right,yord,yl,yh,&root))
        {
           xend[++end] = xh;
           yend[end]=root;
        }
        if(solve(top,xord,xh,xl,&root))
        {
           xend[++end] = root;
           yend[end] = yh;
        }
        if(solve(left,yord,yh,yl,&root))
        {
           xend[++end] = xl;
           yend[end] = root;
        }
      if(end==1)
         rmar_line(xend[0],xend[1],yend[0],yend[1]);
      else if( end==0 )
      {
#ifdef PRINT_ERRORS
         printf("rmardraw: only one end for ");
         printf("[ %f , %f ] X [ %f , %f ]\n", xl,xh,yl,yh );
	 printf("          solution ");
         for(i=0;i<=end;++i)
         {
           printf("x "); printzero(xend[i]);
           printf(" y ");printzero(yend[i]);printf("\n");
         }
#endif
         rmar_point(xend[0],yend[0]);
      }
      else if( onezero2D(bb,xord,yord) )
      {
#ifdef PRINT_ERRORS
         printf("rmardraw: bernstrein polynomial all one sign apart from");
         printf(" one zero\n");
         for(i=0;i<=end;++i)
         {
           printf("x "); printzero(xend[i]);
           printf(" y ");printzero(yend[i]);printf("\n");
         }
         printf("Square [ %f , %f ] X [ %f , %f ]\n", xl,xh,yl,yh );
#endif
      }
      else
      {
#ifdef PRINT_ERRORS
         printf("Error wrong number of ends %d\007\n",end+1); 
         for(i=0;i<=end;++i)
         {
           printf("x "); printzero(xend[i]);
           printf(" y ");printzero(yend[i]);printf("\n");
         }
         printf("Square [ %f , %f ] X [ %f , %f ]\n", xl,xh,yl,yh );
         printf("Bernstein polynomial\n");
         for( j=yord; j>=0; --j)
         {
           for(i=0;i<=xord;i++) printzero(bb[i][j]);
           printf("\n");
         }
#endif
      }
   }
   else
   {

   /*****   devide into 4 quarters do each separatly ******/

      reduce(bb,b1,b2,b3,b4);
      xhalf = (xl + xh) /2;
      yhalf = (yl + yh) /2;
      if(!allonesign(b1,xord,yord))
         rmardraw(b1,xl,xhalf,yl,yhalf);
      if(!allonesign(b2,xord,yord))
         rmardraw(b2,xl,xhalf,yhalf,yh);
      if(!allonesign(b3,xord,yord))
         rmardraw(b3,xhalf,xh,yl,yhalf);
      if(!allonesign(b4,xord,yord))
         rmardraw(b4,xhalf,xh,yhalf,yh);
   }
}
