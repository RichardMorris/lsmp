
#include <stdio.h>
#include <math.h>

main()
{
	prod1();
	prod2();
	prod3();
	prod4();
	prod9();
	pow1();
	pow2();
	pow3();
	pow4();
	pow9();
	mult1();
	mult2();
	mult3();
	mult4();
	mult9();
	switch1();
	switch2();
	switch3();
	switch4();
	switch9();
}

pow1(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = pow(x,1.0); }
pow2(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = pow(x,2.0); }
pow3(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = pow(x,3.0); }
pow4(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = pow(x,4.0); }
pow9(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = pow(x,9.0); }

prod1(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = x; }
prod2(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = x * x; }
prod3(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = x * x * x; }
prod4(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = x * x * x * x; }
prod9(){ register int i; register double x,y; for(x=0.0;x<=1.0;x+=0.01) y = x * x * x * x * x * x * x * x * x; }

mult1(){ register int i; register double x,y;
for(x=0.0;x<=1.0;x+=0.01){ y = 0.0;  for(i=1;i>0;--i) y *= x;} }
mult2(){ register int i; register double x,y;
for(x=0.0;x<=1.0;x+=0.01){ y = 0.0;  for(i=2;i>0;--i) y *= x;} }
mult3(){ register int i; register double x,y;
for(x=0.0;x<=1.0;x+=0.01){ y = 0.0;  for(i=3;i>0;--i) y *= x;} }
mult4(){ register int i; register double x,y;
for(x=0.0;x<=1.0;x+=0.01){ y = 0.0;  for(i=4;i>0;--i) y *= x;} }
mult9(){ register int i; register double x,y;
for(x=0.0;x<=1.0;x+=0.01){ y = 0.0;  for(i=9;i>0;--i) y *= x;} }

switch1()
{ register double x,y; register int j; register i = 1;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
{ for(j=i;j>0;--j) y *= x; break; }
}

switch2(){ register double x,y; register int j; register i = 2;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
{ for(j=i;j>0;--j) y *= x; break; }
}

switch3(){ register double x,y; register int j; register int i = 3;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
{ for(j=i;j>0;--j) y *= x; break; }
}

switch4(){ register double x,y; register int j; register int i = 4;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
{ for(j=i;j>0;--j) y *= x; }
}

switch5(){ register double x,y; register int j; register int i = 5;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
{ for(j=i;j>0;--j) y *= x; }
}

switch9()
{ register double x,y; int j; int i;
i = 9;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
 {
 for(j=i;j>0;--j)
	 y *= x;
 }
}

switch_n( register int i)
{ register double x,y; register j;
for(x=0.0;x<=1.0;x+=0.01)
if( i <= 3 )
switch(i)
{
case 1: y=x;break;
case 2: y=x*x;break;
case 3: y=x*x*x;break;
case 4: y=x*x*x*x;break;
case 5: y=x*x*x*x*x;break;
case 6: y=x*x*x*x*x*x;break;
case 7: y=x*x*x*x*x*x*x;break;
case 8: y=x*x*x*x*x*x*x*x;break;
case 9: y=x*x*x*x*x*x*x*x*x;break;
case 10: y=x*x*x*x*x*x*x*x*x*x;break;
}
else
{ for(j=i;j>0;--j) y *= x; break; }
}
