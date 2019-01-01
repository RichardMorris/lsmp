#include <stdio.h>

fun()
{
fprintf(stderr,"fun\n");
}

main(int argc,char **argv)
{
fprintf(stderr,"main\n");
fun();
/*
*/
fprintf(stderr,"main done\n");
}

