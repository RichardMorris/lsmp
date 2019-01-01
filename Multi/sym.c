#include <math.h>

/* BEGIN0 -- don't edit until END0 */
struct in {
double s;
double t;
double a;
double b;
};

struct out {
double top0;
double top1;
double top2;
};

void
myfunction(  struct in *in, struct out *out )
/* END0  */
{
double gggx1;
double gggy1;
double k1;
double k1_D_s;
double XX1;
double XX1_D_s;
double XX1_D_s_D_s;
double YY1;
double YY1_D_s;
double YY1_D_s_D_s;
double X1;
double X1_D_s;
double X1_D_s_D_s;
double X1_D_s_D_s_D_s;
double Y1;
double Y1_D_s;
double Y1_D_s_D_s;
double Y1_D_s_D_s_D_s;
double root_s;
double root_s_D_s;
double root_s_D_s_D_s;
double root_s_D_s_D_s_D_s;
double m_s;
double m_s_D_s;
double m_s_D_s_D_s;
double m_s_D_s_D_s_D_s;
double sin_s;
double sin_s_D_s;
double sin_s_D_s_D_s;
double sin_s_D_s_D_s_D_s;
double cos_s;
double cos_s_D_s;
double cos_s_D_s_D_s;
double cos_s_D_s_D_s_D_s;
double gggx2;
double gggy2;
double k2;
double k2_D_t;
double XX2;
double XX2_D_t;
double XX2_D_t_D_t;
double YY2;
double YY2_D_t;
double YY2_D_t_D_t;
double X2;
double X2_D_t;
double X2_D_t_D_t;
double X2_D_t_D_t_D_t;
double Y2;
double Y2_D_t;
double Y2_D_t_D_t;
double Y2_D_t_D_t_D_t;
double root_t;
double root_t_D_t;
double root_t_D_t_D_t;
double root_t_D_t_D_t_D_t;
double sin_t;
double sin_t_D_t;
double sin_t_D_t_D_t;
double sin_t_D_t_D_t_D_t;
double cos_t;
double cos_t_D_t;
double cos_t_D_t_D_t;
double cos_t_D_t_D_t_D_t;
cos_t_D_t_D_t_D_t=sin(in->t);
cos_t_D_t_D_t=-1.0*cos(in->t);
cos_t_D_t=-1.0*sin(in->t);
cos_t=cos(in->t);
sin_t_D_t_D_t_D_t=-1.0*cos(in->t);
sin_t_D_t_D_t=-1.0*sin(in->t);
sin_t_D_t=cos(in->t);
sin_t=sin(in->t);
root_t_D_t_D_t_D_t=(0.5*(-0.5*
-1.5*pow(6.0+2.0*cos_t,-2.5)*2.0*cos_t_D_t*2.0*cos_t_D_t+
-0.5*pow(6.0+2.0*cos_t,-1.5)*2.0*cos_t_D_t_D_t)*2.0*cos_t_D_t+
0.5*-0.5*pow(6.0+2.0*cos_t,-1.5)*2.0*
cos_t_D_t*2.0*cos_t_D_t_D_t+0.5*-0.5*pow(6.0+
2.0*cos_t,-1.5)*2.0*cos_t_D_t*2.0*cos_t_D_t_D_t+0.5*pow(6.0+
2.0*cos_t,-0.5)*2.0*cos_t_D_t_D_t_D_t)*16384.0/65536.0;
root_t_D_t_D_t=(0.5*-0.5*pow(6.0+
2.0*cos_t,-1.5)*2.0*cos_t_D_t*2.0*cos_t_D_t+0.5*pow(
6.0+2.0*cos_t,-0.5)*2.0*cos_t_D_t_D_t)*64.0/256.0;
root_t_D_t=0.5*pow(6.0+2.0*cos_t,-0.5)*
2.0*cos_t_D_t*4.0/16.0;
root_t=pow(6.0+2.0*cos_t,0.5)/4.0;
Y2_D_t_D_t_D_t=in->a*(sin_t_D_t_D_t_D_t*(root_t/4.0)+sin_t_D_t_D_t*(
root_t_D_t*4.0/16.0)+sin_t_D_t_D_t*(root_t_D_t*4.0/16.0)+sin_t_D_t*(
root_t_D_t_D_t*64.0/256.0)+sin_t_D_t_D_t*(root_t_D_t*4.0/16.0)+sin_t_D_t*(
root_t_D_t_D_t*64.0/256.0)+sin_t_D_t*(root_t_D_t_D_t*64.0/256.0)+sin_t*(
root_t_D_t_D_t_D_t*16384.0/65536.0))+in->b*(cos_t_D_t_D_t_D_t*128.0/256.0);
Y2_D_t_D_t=in->a*(sin_t_D_t_D_t*(root_t/4.0)+sin_t_D_t*(
root_t_D_t*4.0/16.0)+sin_t_D_t*(root_t_D_t*4.0/16.0)+sin_t*(
root_t_D_t_D_t*64.0/256.0))+in->b*(cos_t_D_t_D_t*8.0/16.0);
Y2_D_t=in->a*(sin_t_D_t*(root_t/4.0)+sin_t*(
root_t_D_t*4.0/16.0))+in->b*(cos_t_D_t*2.0/4.0);
Y2=in->a*sin_t*(root_t/4.0)+in->b*((1.0+
cos_t)/2.0);
X2_D_t_D_t_D_t=sin(in->t)*128.0/256.0;
X2_D_t_D_t=cos(in->t)*-8.0/16.0;
X2_D_t=sin(in->t)*-2.0/4.0;
X2=(1.0+cos(in->t))/2.0;
YY2_D_t_D_t=Y2_D_t_D_t_D_t;
YY2_D_t=Y2_D_t_D_t;
YY2=Y2_D_t;
XX2_D_t_D_t=X2_D_t_D_t_D_t;
XX2_D_t=X2_D_t_D_t;
XX2=X2_D_t;
k2_D_t=X2_D_t_D_t*YY2_D_t+X2_D_t*YY2_D_t_D_t-(Y2_D_t_D_t*XX2_D_t+
Y2_D_t*XX2_D_t_D_t);
k2=X2_D_t*YY2_D_t-Y2_D_t*XX2_D_t;
gggy2=pow(k2,-0.666667)*YY2_D_t-k2_D_t*pow(k2,-1.666667)*(
YY2/3.0);
gggx2=pow(k2,-0.666667)*XX2_D_t-k2_D_t*pow(k2,-1.666667)*(
XX2/3.0);
cos_s_D_s_D_s_D_s=sin(in->s);
cos_s_D_s_D_s=-1.0*cos(in->s);
cos_s_D_s=-1.0*sin(in->s);
cos_s=cos(in->s);
sin_s_D_s_D_s_D_s=-1.0*cos(in->s);
sin_s_D_s_D_s=-1.0*sin(in->s);
sin_s_D_s=cos(in->s);
sin_s=sin(in->s);
m_s_D_s_D_s_D_s=2.0*cos_s_D_s_D_s_D_s;
m_s_D_s_D_s=2.0*cos_s_D_s_D_s;
m_s_D_s=2.0*cos_s_D_s;
m_s=6.0+2.0*cos_s;
root_s_D_s_D_s_D_s=(0.5*(-0.5*
-1.5*pow(m_s,-2.5)*m_s_D_s*m_s_D_s+-0.5*pow(m_s,-1.5)*m_s_D_s_D_s)*m_s_D_s+
0.5*-0.5*pow(m_s,-1.5)*m_s_D_s*m_s_D_s_D_s+0.5*
-0.5*pow(m_s,-1.5)*m_s_D_s*m_s_D_s_D_s+0.5*pow(m_s,-0.5)*
m_s_D_s_D_s_D_s)*16384.0/65536.0;
root_s_D_s_D_s=(0.5*-0.5*pow(m_s,-1.5)*
m_s_D_s*m_s_D_s+0.5*pow(m_s,-0.5)*m_s_D_s_D_s)*64.0/256.0;
root_s_D_s=0.5*pow(m_s,-0.5)*m_s_D_s*4.0/16.0;
root_s=pow(m_s,0.5)/4.0;
Y1_D_s_D_s_D_s=in->a*(sin_s_D_s_D_s_D_s*(root_s/4.0)+sin_s_D_s_D_s*(
root_s_D_s*4.0/16.0)+sin_s_D_s_D_s*(root_s_D_s*4.0/16.0)+sin_s_D_s*(
root_s_D_s_D_s*64.0/256.0)+sin_s_D_s_D_s*(root_s_D_s*4.0/16.0)+sin_s_D_s*(
root_s_D_s_D_s*64.0/256.0)+sin_s_D_s*(root_s_D_s_D_s*64.0/256.0)+sin_s*(
root_s_D_s_D_s_D_s*16384.0/65536.0))+in->b*(cos_s_D_s_D_s_D_s*128.0/256.0);
Y1_D_s_D_s=in->a*(sin_s_D_s_D_s*(root_s/4.0)+sin_s_D_s*(
root_s_D_s*4.0/16.0)+sin_s_D_s*(root_s_D_s*4.0/16.0)+sin_s*(
root_s_D_s_D_s*64.0/256.0))+in->b*(cos_s_D_s_D_s*8.0/16.0);
Y1_D_s=in->a*(sin_s_D_s*(root_s/4.0)+sin_s*(
root_s_D_s*4.0/16.0))+in->b*(cos_s_D_s*2.0/4.0);
Y1=in->a*sin_s*(root_s/4.0)+in->b*((1.0+
cos_s)/2.0);
X1_D_s_D_s_D_s=sin(in->s)*128.0/256.0;
X1_D_s_D_s=cos(in->s)*-8.0/16.0;
X1_D_s=sin(in->s)*-2.0/4.0;
X1=(1.0+cos(in->s))/2.0;
YY1_D_s_D_s=Y1_D_s_D_s_D_s;
YY1_D_s=Y1_D_s_D_s;
YY1=Y1_D_s;
XX1_D_s_D_s=X1_D_s_D_s_D_s;
XX1_D_s=X1_D_s_D_s;
XX1=X1_D_s;
k1_D_s=X1_D_s_D_s*YY1_D_s+X1_D_s*YY1_D_s_D_s-(Y1_D_s_D_s*XX1_D_s+
Y1_D_s*XX1_D_s_D_s);
k1=X1_D_s*YY1_D_s-Y1_D_s*XX1_D_s;
gggy1=pow(k1,-0.666667)*YY1_D_s-k1_D_s*pow(k1,-1.666667)*(
YY1/3.0);
out->top0 = in->s;
out->top1 = in->t;
out->top2 = (X1-X2)*(gggy1-gggy2)-(Y1-Y2)*(gggx1-gggx2);
return;
}
