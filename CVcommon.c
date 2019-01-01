/*
 *      file:   CGIVRMLcommon.c:   
 *      author: Rich Morris
 *      date:   18 Oct 2001
 *      
 *	common bits for all CGI VRML version of the LSMP
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "CVcommon.h"

void report_error(int code,char *str,int errcode)
{
                if(code == HEAD_ERROR)
                {
	                printf("Content-type: text/plain%c%c",10,10);
                        printf("ERROR %s\n",str);
                        fprintf(stderr,"ERROR %d %s\n",errcode,str);
                }
                else if(code == NO_REP_HEAD_ERROR)
                {
                        fprintf(stderr,"ERROR %d %s\n",errcode,str);
                }
		else if(code == HEAD_WARNING)
		{
	                printf("Content-type: text/plain%c%c",10,10);
                        printf("WARNING %s\n",str);
                        fprintf(stderr,"WARNING %d %s\n",errcode,str);
		}
                else
                {
                        printf("WARNING %s\n",str);
                        fprintf(stderr,"WARNING %d %s\n",errcode,str);
                }
}

void report_error2(int code,char *format,char *str,int errcode)
{
                if(code == HEAD_ERROR)
                {
	                printf("Content-type: text/plain%c%c",10,10);
                        printf("ERROR ");
                        fprintf(stderr,"ERROR %d ",errcode);
	                printf(format,str);
	                printf("\n");
                }
                else if(code == HEAD_WARNING)
                {
	                printf("Content-type: text/plain%c%c",10,10);
                        printf("WARNING ");
                        fprintf(stderr,"WARNING %d ",errcode);
	                printf(format,str);
	                printf("\n");
                }
                else if(code == NO_REP_HEAD_ERROR)
                {
                        fprintf(stderr,"ERROR %d %s\n",errcode,str);
                }
                else
                {
                        printf("WARNING ");
                        fprintf(stderr,"WARNING %d ",errcode);
        	        printf(format,str);
	       		printf("\n");
                }
                fprintf(stderr,format,str);
                fprintf(stderr,"\n");
}

void print_time_message(char *str)
{
	char *tstr;
	time_t tim;

        time(&tim);
        tstr = ctime(&tim);
        tstr[19] = '\0';
        fprintf(stderr,"%s\t\%d%s\n",tstr,clock(),str);
}

#define LF 10
#define CR 13

/* read through line until stop char copying that bit to word and deleting */

void getword(char *word, char *line, char stop) {
    int x = 0,y;

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
}
/* read through line until stop char copying that bit to word and deleting */
/* returns word */

char *makeword(char *line, char stop) {
    int x = 0,y;
    char *word = (char *) malloc(sizeof(char) * (strlen(line) + 1));

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
    return word;
}

/* read through file until stop char copying that bit to word */
/* returns word */

char *fmakeword(FILE *f, char stop, int *cl) {
    int wsize;
    char *word;
    int ll;

    wsize = 102400;
    ll=0;
    word = (char *) malloc(sizeof(char) * (wsize + 1));

    while(1) {
        word[ll] = (char)fgetc(f);
        if(ll==wsize) {
            word[ll+1] = '\0';
            wsize+=102400;
            word = (char *)realloc(word,sizeof(char)*(wsize+1));
        }
        --(*cl);
        if((word[ll] == stop) || (feof(f)) || (!(*cl))) {
            if(word[ll] != stop) ll++;
            word[ll] = '\0';
            return word;
        }
        ++ll;
    }
}

/* hex to char */

char x2c(char *what) {
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

/* converts url replacing  %2d  characters */

void unescape_url(char *url) {
    register int x,y;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

/* converts + to spaces */

void plustospace(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

/* find last occurence of c */

int rind(char *s, char c) {
    register int x;
    for(x=strlen(s) - 1;x != -1; x--)
        if(s[x] == c) return x;
    return -1;
}

/* reads a line from a file result in s  returns true if EOF */

int getline(char *s, int n, FILE *f) {
    register int i=0;

    while(1) {
        s[i] = (char)fgetc(f);

        if(s[i] == CR)
            s[i] = fgetc(f);

        if((s[i] == 0x4) || (s[i] == LF) || (i == (n-1))) {
            s[i] = '\0';
            return (feof(f) ? 1 : 0);
        }
        ++i;
    }
}

/* copy f to fd */

void send_fd(FILE *f, FILE *fd)
{
    char c;

    while (1) {
        c = fgetc(f);
        if(feof(f))
            return;
        fputc(c,fd);
    }
}

/* finds first occurence of c */

int ind(char *s, char c) {
    register int x;

    for(x=0;s[x];x++)
        if(s[x] == c) return x;

    return -1;
}

/* changes  & to \& etc. */

void escape_shell_cmd(char *cmd) {
    register int x,y,l;

    l=strlen(cmd);
    for(x=0;cmd[x];x++) {
        if(ind("\n&;`'\"|*?~<>^()[]{}$\\",cmd[x]) != -1){
            for(y=l+1;y>x;y--)
                cmd[y] = cmd[y-1];
            l++; /* length has been increased */
            cmd[x] = '\\';
            x++; /* skip the character */
        }
    }
}

void print_jvx_header(char *title,char *abstract)
{

printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"no\"?>\n");
printf("<!DOCTYPE jvx-model SYSTEM \"http://www.javaview.de/rsrc/jvx.dtd\">\n");

printf("<jvx-model>\n");
printf(" <meta generator=\"LSMP CGI-Version\" />\n");
printf(" <version type=\"dump\">0.02</version>\n");
printf(" <title>%s</title>\n",title);
printf(" <authors>\n");
printf("  <author>\n");
printf("   <firstname>Richard</firstname>\n");
printf("   <lastname>Morris</lastname>\n");
printf("   <affiliation>\n");
printf("    <organization>Dept Statistics, University of Leeds</organization>\n");
printf("    <address>\n");
printf("     <line>Leeds</line>\n");
printf("     <line>LS2 9JT</line>\n");
printf("    </address>\n");
printf("   </affiliation>\n");
printf("   <email>webmaster@pfaf.org, rjm@amsta.leeds.ac.uk</email>\n");
printf("   <url>www.pfaf.org</url>\n");
printf("  </author>\n");
printf(" </authors>\n");
printf(" <description>\n");
printf("  <abstract>Automatically Generated %s</abstract>\n",abstract);
printf("  <detail>\n");
}

void print_jvx_header2(char *keyword)
{
	printf("  </detail>\n");
	printf("  <msc2000>\n");
	printf("    <primary>14J99</primary>\n");
	printf("    <secondary>14J17</secondary>\n");
	printf("    <secondary>32S25</secondary>\n");
	printf("    <secondary>65S05</secondary>\n");
	printf("    <secondary>14Q10</secondary>\n");
	printf("  </msc2000>\n");
	printf("  <keywords>\n");
	printf("   <keyword>%s</keyword>\n",keyword);
	printf("  </keywords>\n");
	printf("  <software>Liverpool Surface Modeling Package</software>\n");
	printf(" </description>\n");
}

void print_jvx_tail()
{
	printf("</jvx-model>\n");
}

void HPt3Copy(HPoint3 *sol,HPoint3 *res)
{
	res->x = sol->x;
	res->y = sol->y;
	res->z = sol->z;
	res->w = sol->w;
}

void HPt3LinSum(double lambda,HPoint3 *low,double mu,HPoint3 *high,HPoint3 *sol)
{
	sol->x = lambda * low->x + mu * high->x;
	sol->y = lambda * low->y + mu * high->y;
	sol->z = lambda * low->z + mu * high->z;
	sol->w = lambda * low->w + mu * high->w;
}

void Pt3Comb(double lambda,Point3 *low,double mu,Point3 *high,Point3 *sol)
{
	sol->x = lambda * low->x + mu * high->x;
	sol->y = lambda * low->y + mu * high->y;
	sol->z = lambda * low->z + mu * high->z;
}

/*****************************************************
Functions to handle reading input
This input will be psudo xml style
but tags are only on one line 

<definition name="Ridge" type="intersect" opType="psurf icurve">
V3ppp = 0;
....
<option name="colour" type="String" value="Red">
<option name="iterations" type="int" value="20">
<option name="tolerance" type="double" value="0.000001">
</definition>

<definition name="Monge Form" type="psurf">
....
<variable name="x" min="-0.5" max="0.5">
</definition>

<definition name="Principal directions" type="icurve" opType="psurf">
....
<option name="orientation" type="String" value="Major Eigen Vector">
</definition>
<start-jvx-section/>
<jvx-model>
.....
</jvx-model>
EOF
*****************************************************/

char *getAttribute(char *line,char *attName)
{
	char *tmp,*ptr,*ptr2,*ptr3,*res;
	int len,len2;

	tmp = (char *) malloc(sizeof(char) * (strlen(attName+7)));
	sprintf(tmp," %s=\"",attName);
	if( (ptr = strstr(line,tmp)) != NULL)
	{
		len = strlen(tmp);
		ptr2 = ptr+len;
		if(ptr2 == NULL || *ptr2 == '\0') return NULL;
		ptr3 = strchr(ptr2,'"');
		if(ptr3 == NULL) return NULL;
		len2 = ptr3-ptr2;
		/* match found */

		res = (char *) malloc(sizeof(char)*(len2+1));
		strncpy(res,ptr2,len2);
		*(res + len2) = '\0';
		return res;
	}
	else
		return NULL;
}

LsmpDef *newLsmpDef(char *name,char *type,char *opType)
{
	LsmpDef *def;
	def = (LsmpDef *) malloc(sizeof(LsmpDef));
	def->name = name;
	def->type = type;
	def->opType = opType;
	def->n_options = 0;
	def->n_parameters = 0;
	def->n_variables = 0;
	def->alloc_size = LSMP_ALLOC_CHUNK;
	def->data = (char *) malloc(sizeof(char)*def->alloc_size);
	def->data[0] = '\0';
	return def;
}

LsmpOption *newLsmpOption(char *name,char *type,char *value)
{
	LsmpOption *opt;

	if(name == NULL || value == NULL)
	{
		report_error(HEAD_ERROR,"option must have name and value",701);
		return NULL;
	}

	opt = (LsmpOption *) malloc(sizeof(LsmpOption));
	opt->name = name;
	opt->type = type;
	opt->value = value;
	opt->d_value = atof(value);
	opt->i_value = atoi(value);
	return opt;
}

LsmpVariable *newLsmpVariable(char *name,char *min,char *max,char *type,char *value)
{
	LsmpVariable *var;

	if(name == NULL)
	{
		report_error(HEAD_ERROR,"variable must have a name",701);
		return NULL;
	}
	var = (LsmpVariable *) malloc(sizeof(LsmpVariable));
	var->name = name;
	if(min != NULL)	var->min = atof(min); else var->min = HUGE_VAL;
	if(max != NULL) var->max = atof(max); else var->max = HUGE_VAL;
	var->type = type;
	if(value != NULL) var->value = atof(value);else var->value = HUGE_VAL;
	return var;
}

LsmpParameter *newLsmpParameter(char *name,char *value)
{
	LsmpParameter *param;
	if(name == NULL || value == NULL)
	param = (LsmpParameter *) malloc(sizeof(LsmpParameter));
	param->name = name;
	param->value = atof(value);
	return param;
}

void addLsmpOption(LsmpDef *def,LsmpOption *opt)
{
	if(opt==NULL) return;
	if(def->n_options + 1 >= MAX_LSMP_OPTS)
	{
		report_error(HEAD_ERROR,"Too many options in input",701);
		return;
	}
	def->options[def->n_options++] = opt;
}

void addLsmpVariable(LsmpDef *def,LsmpVariable *opt)
{
	if(opt==NULL) return;
	if(def->n_variables + 1 >= MAX_LSMP_VARS)
	{
		report_error(HEAD_ERROR,"Too many variables in input",702);
		return;
	}
	def->variables[def->n_variables++] = opt;
}

void addLsmpParameter(LsmpDef *def,LsmpParameter *opt)
{
	if(opt==NULL) return;
	if(def->n_parameters + 1 >= MAX_LSMP_PARAMS)
	{
		report_error(HEAD_ERROR,"Too many parameters in input",703);
		return;
	}
	def->parameters[def->n_parameters++] = opt;
}

void addLsmpDataLine(LsmpDef *def,char *line)
{
	if(strlen(def->data)+strlen(line) > def->alloc_size - 5)
	{
		def->alloc_size *= 2;
		def->data = (char *) realloc(def->data,def->alloc_size);
	}
	strcat(def->data,line);
}

LsmpOption *getLsmpOption(LsmpDef *def,char *name)
{
	int i;
	for(i=0;i<def->n_options;++i)
	{
		if(!strcmp(def->options[i]->name,name))
			return def->options[i];
	}
	return NULL;
}

/** Reads the specification from the input */

LsmpInputSpec *readInputSpec(FILE *fp,int *cl_ptr,FILE *fl)
{
        char *env_query;
	char buf[1024];
	int state=0,cl,i;
	xml_tree *jvx;
	LsmpInputSpec *spec;
	int defNum=0;
	LsmpDef *curDef;

	spec = (LsmpInputSpec *) malloc(sizeof(LsmpInputSpec));

	while(state>=0  && *cl_ptr != 0 && fgets(buf,1023,fp) != NULL)
	{
	   *cl_ptr -= strlen(buf);
fprintf(fl,"%s",buf);
	  switch(state)
	  {
	  case 0:
		if(strstr(buf,"<definition")!=NULL)
		{
			state = 1;
			curDef = newLsmpDef(
				getAttribute(buf,"name"),
				getAttribute(buf,"type"),
				getAttribute(buf,"opType"));
			switch(defNum)
			{
			case 0:	spec->Def = curDef; break;
			case 1:	spec->auxDef = curDef; break;
			case 2:	spec->auxDef2 = curDef; break;
			}
			++defNum;
			state = 1;
		}
		else if(strstr(buf,"<start-jvx-section/>")!=NULL)
		{
			state = -1;
		}
		break;
	  case 1:
		if(strstr(buf,"<option")!=NULL)
		{
			addLsmpOption(curDef,newLsmpOption(
				getAttribute(buf,"name"),
				getAttribute(buf,"type"),
				getAttribute(buf,"value")));
		}
		else if(strstr(buf,"<variable")!=NULL)
		{
			addLsmpVariable(curDef,newLsmpVariable(
				getAttribute(buf,"name"),
				getAttribute(buf,"min"),
				getAttribute(buf,"max"),
				getAttribute(buf,"type"),
				getAttribute(buf,"value")));
		}
		else if(strstr(buf,"<parameter")!=NULL)
		{
			addLsmpParameter(curDef,newLsmpParameter(
				getAttribute(buf,"name"),
				getAttribute(buf,"value")));
		}
		else if(strstr(buf,"</definition>")!=NULL)
		{
			state = 0;
		}
		else
			addLsmpDataLine(curDef,buf);
		break;
	  }
	  if(feof(fp)) break;
	} /* end while */

	return spec;
}

