#include "jvx.h"
/*
 * Automatically by org.pfaf/lsmp/jvx/genParser.pl generated from:
 *
document	"Document Type Definition of the JVX geometry file format."
status	"This is a release specification."
author	"Konrad Polthier"
url	"http://www.javaview.de/rsrc/jvx.dtd"
version	"1.01.00"
date	"05.04.01"
*/


int create_node_specific_info(xml_tree *node)
{
	int i;

	node->error_message = NULL; node->error = 0;
	if(0) {}
	else if(!strcmp(node->name,"jvx-model")) node->type = LSMP_JVX_MODEL;
	else if(!strcmp(node->name,"meta")) node->type = LSMP_META;
	else if(!strcmp(node->name,"version")) node->type = LSMP_VERSION;
	else if(!strcmp(node->name,"title")) node->type = LSMP_TITLE;
	else if(!strcmp(node->name,"authors")) node->type = LSMP_AUTHORS;
	else if(!strcmp(node->name,"author")) node->type = LSMP_AUTHOR;
	else if(!strcmp(node->name,"firstname")) node->type = LSMP_FIRSTNAME;
	else if(!strcmp(node->name,"lastname")) node->type = LSMP_LASTNAME;
	else if(!strcmp(node->name,"affiliation")) node->type = LSMP_AFFILIATION;
	else if(!strcmp(node->name,"organization")) node->type = LSMP_ORGANIZATION;
	else if(!strcmp(node->name,"address")) node->type = LSMP_ADDRESS;
	else if(!strcmp(node->name,"line")) node->type = LSMP_LINE;
	else if(!strcmp(node->name,"email")) node->type = LSMP_EMAIL;
	else if(!strcmp(node->name,"url")) node->type = LSMP_URL;
	else if(!strcmp(node->name,"description")) node->type = LSMP_DESCRIPTION;
	else if(!strcmp(node->name,"abstract")) node->type = LSMP_ABSTRACT;
	else if(!strcmp(node->name,"detail")) node->type = LSMP_DETAIL;
	else if(!strcmp(node->name,"msc2000")) node->type = LSMP_MSC2000;
	else if(!strcmp(node->name,"primary")) node->type = LSMP_PRIMARY;
	else if(!strcmp(node->name,"secondary")) node->type = LSMP_SECONDARY;
	else if(!strcmp(node->name,"keywords")) node->type = LSMP_KEYWORDS;
	else if(!strcmp(node->name,"keyword")) node->type = LSMP_KEYWORD;
	else if(!strcmp(node->name,"software")) node->type = LSMP_SOFTWARE;
	else if(!strcmp(node->name,"geometries")) node->type = LSMP_GEOMETRIES;
	else if(!strcmp(node->name,"geometry")) node->type = LSMP_GEOMETRY;
	else if(!strcmp(node->name,"pointSet")) node->type = LSMP_POINTSET;
	else if(!strcmp(node->name,"lineSet")) node->type = LSMP_LINESET;
	else if(!strcmp(node->name,"faceSet")) node->type = LSMP_FACESET;
	else if(!strcmp(node->name,"vectorField")) node->type = LSMP_VECTORFIELD;
	else if(!strcmp(node->name,"points")) node->type = LSMP_POINTS;
	else if(!strcmp(node->name,"p")) node->type = LSMP_P;
	else if(!strcmp(node->name,"colors")) node->type = LSMP_COLORS;
	else if(!strcmp(node->name,"c")) node->type = LSMP_C;
	else if(!strcmp(node->name,"colorsBack")) node->type = LSMP_COLORSBACK;
	else if(!strcmp(node->name,"normals")) node->type = LSMP_NORMALS;
	else if(!strcmp(node->name,"n")) node->type = LSMP_N;
	else if(!strcmp(node->name,"lines")) node->type = LSMP_LINES;
	else if(!strcmp(node->name,"l")) node->type = LSMP_L;
	else if(!strcmp(node->name,"faces")) node->type = LSMP_FACES;
	else if(!strcmp(node->name,"f")) node->type = LSMP_F;
	else if(!strcmp(node->name,"edges")) node->type = LSMP_EDGES;
	else if(!strcmp(node->name,"e")) node->type = LSMP_E;
	else if(!strcmp(node->name,"neighbours")) node->type = LSMP_NEIGHBOURS;
	else if(!strcmp(node->name,"nb")) node->type = LSMP_NB;
	else if(!strcmp(node->name,"vectors")) node->type = LSMP_VECTORS;
	else if(!strcmp(node->name,"v")) node->type = LSMP_V;
	else if(!strcmp(node->name,"textures")) node->type = LSMP_TEXTURES;
	else if(!strcmp(node->name,"t")) node->type = LSMP_T;
	else if(!strcmp(node->name,"boundaries")) node->type = LSMP_BOUNDARIES;
	else if(!strcmp(node->name,"color")) node->type = LSMP_COLOR;
	else if(!strcmp(node->name,"colorBack")) node->type = LSMP_COLORBACK;
	else if(!strcmp(node->name,"colorTag")) node->type = LSMP_COLORTAG;
	else if(!strcmp(node->name,"primitive")) node->type = LSMP_PRIMITIVE;
	else if(!strcmp(node->name,"cube")) node->type = LSMP_CUBE;
	else if(!strcmp(node->name,"lowerLeft")) node->type = LSMP_LOWERLEFT;
	else if(!strcmp(node->name,"upperRight")) node->type = LSMP_UPPERRIGHT;
	else if(!strcmp(node->name,"sphere")) node->type = LSMP_SPHERE;
	else if(!strcmp(node->name,"midpoint")) node->type = LSMP_MIDPOINT;
	else if(!strcmp(node->name,"cylinder")) node->type = LSMP_CYLINDER;
	else if(!strcmp(node->name,"bottom")) node->type = LSMP_BOTTOM;
	else if(!strcmp(node->name,"top")) node->type = LSMP_TOP;
	else if(!strcmp(node->name,"cone")) node->type = LSMP_CONE;
	else if(!strcmp(node->name,"radius")) node->type = LSMP_RADIUS;
	else if(!strcmp(node->name,"thickness")) node->type = LSMP_THICKNESS;
	else if(!strcmp(node->name,"length")) node->type = LSMP_LENGTH;
	else if(!strcmp(node->name,"bndbox")) node->type = LSMP_BNDBOX;
	else if(!strcmp(node->name,"center")) node->type = LSMP_CENTER;
	else if(!strcmp(node->name,"labelAtt")) node->type = LSMP_LABELATT;
	else if(!strcmp(node->name,"xOffset")) node->type = LSMP_XOFFSET;
	else if(!strcmp(node->name,"yOffset")) node->type = LSMP_YOFFSET;
	else if(!strcmp(node->name,"material")) node->type = LSMP_MATERIAL;
	else if(!strcmp(node->name,"ambientIntensity")) node->type = LSMP_AMBIENTINTENSITY;
	else if(!strcmp(node->name,"diffuse")) node->type = LSMP_DIFFUSE;
	else if(!strcmp(node->name,"emissive")) node->type = LSMP_EMISSIVE;
	else if(!strcmp(node->name,"shininess")) node->type = LSMP_SHININESS;
	else if(!strcmp(node->name,"specular")) node->type = LSMP_SPECULAR;
	else if(!strcmp(node->name,"transparency")) node->type = LSMP_TRANSPARENCY;
	else if(!strcmp(node->name,"image")) node->type = LSMP_IMAGE;
	else
	{
		node->error_message = (char *) calloc(sizeof(char),
			strlen("image: bad element name ") + strlen(node->name) +1 );
		node->type = LSMP_LSMP_ERROR;
		node->error = 1000;
		sprintf(node->error_message,"image: bad element name %s",node->name);
	}

	switch(node->type)
	{
	case LSMP_JVX_MODEL:
		node->u.jvx_model = (struct jvx_model *) malloc(sizeof(struct jvx_model));
		node->u.jvx_model->meta = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.jvx_model->n_meta = 0;
		node->u.jvx_model->description =NULL;
		node->u.jvx_model->geometries =NULL;
		node->u.jvx_model->title =NULL;
		node->u.jvx_model->version =NULL;
		node->u.jvx_model->authors =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_META:
			node->u.jvx_model->meta[node->u.jvx_model->n_meta++] = node->children[i];
			break;
		    case LSMP_DESCRIPTION:
			if(node->u.jvx_model->description != NULL)
			{
				node->error_message = strdup("jvx_model: description can only be defined once");
				node->error = 1001;
			}
			else
				node->u.jvx_model->description = node->children[i];
			break;
		    case LSMP_GEOMETRIES:
			if(node->u.jvx_model->geometries != NULL)
			{
				node->error_message = strdup("jvx_model: geometries can only be defined once");
				node->error = 1002;
			}
			else
				node->u.jvx_model->geometries = node->children[i];
			break;
		    case LSMP_TITLE:
			if(node->u.jvx_model->title != NULL)
			{
				node->error_message = strdup("jvx_model: title can only be defined once");
				node->error = 1003;
			}
			else
				node->u.jvx_model->title = node->children[i];
			break;
		    case LSMP_VERSION:
			if(node->u.jvx_model->version != NULL)
			{
				node->error_message = strdup("jvx_model: version can only be defined once");
				node->error = 1004;
			}
			else
				node->u.jvx_model->version = node->children[i];
			break;
		    case LSMP_AUTHORS:
			if(node->u.jvx_model->authors != NULL)
			{
				node->error_message = strdup("jvx_model: authors can only be defined once");
				node->error = 1005;
			}
			else
				node->u.jvx_model->authors = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.jvx_model->geometries == NULL)
		{
			node->error_message = strdup("jvx_model: geometries must be defined");
			node->error = 1006;
		}
		if(node->u.jvx_model->title == NULL)
		{
			node->error_message = strdup("jvx_model: title must be defined");
			node->error = 1007;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("jvx_model: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"jvx_model: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1008;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_META:
		node->u.meta = (struct meta *) malloc(sizeof(struct meta));
		node->u.meta->date = NULL;
		node->u.meta->generator = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"date"))
				node->u.meta->date = node->attr[i+1];
			else if(!strcmp(node->attr[i],"generator"))
				node->u.meta->generator = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("meta: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"meta: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1009;
			}
		}
		/** Check attributes **/
		/* date [CDATA] [#IMPLIED] */
		/* generator [CDATA] [#IMPLIED] */
		break;
	case LSMP_VERSION:
		node->u.version = (struct version *) malloc(sizeof(struct version));
		node->u.version->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"type"))
				node->u.version->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("version: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"version: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1010;
			}
		}
		/** Check attributes **/
		/* type [(dump|beta|final)] ["dump"] */
		if(node->u.version->type == NULL) {}
		else if(!strcmp(node->u.version->type,"dump")) {}
		else if(!strcmp(node->u.version->type,"beta")) {}
		else if(!strcmp(node->u.version->type,"final")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("version: atribute type (%s) not in specification (dump|beta|final)\n") + strlen(node->u.version->type));
			sprintf(node->error_message,"version: atribute type (%s) not in specification (dump|beta|final)\n",node->u.version->type);
		}
		if(node->u.version->type == NULL)
			node->u.version->type = "dump";
		break;
	case LSMP_TITLE:
		node->u.title = (struct title *) malloc(sizeof(struct title));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("title: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"title: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1013;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_AUTHORS:
		node->u.authors = (struct authors *) malloc(sizeof(struct authors));
		node->u.authors->author = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.authors->n_author = 0;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_AUTHOR:
			node->u.authors->author[node->u.authors->n_author++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.authors->n_author < 1)
		{
			node->error = 1014;
			node->error_message = strdup("authors author must be defined at least once");
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("authors: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"authors: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1015;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_AUTHOR:
		node->u.author = (struct author *) malloc(sizeof(struct author));
		node->u.author->lastname =NULL;
		node->u.author->url =NULL;
		node->u.author->firstname =NULL;
		node->u.author->email =NULL;
		node->u.author->affiliation =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_LASTNAME:
			if(node->u.author->lastname != NULL)
			{
				node->error_message = strdup("author: lastname can only be defined once");
				node->error = 1016;
			}
			else
				node->u.author->lastname = node->children[i];
			break;
		    case LSMP_URL:
			if(node->u.author->url != NULL)
			{
				node->error_message = strdup("author: url can only be defined once");
				node->error = 1017;
			}
			else
				node->u.author->url = node->children[i];
			break;
		    case LSMP_FIRSTNAME:
			if(node->u.author->firstname != NULL)
			{
				node->error_message = strdup("author: firstname can only be defined once");
				node->error = 1018;
			}
			else
				node->u.author->firstname = node->children[i];
			break;
		    case LSMP_EMAIL:
			if(node->u.author->email != NULL)
			{
				node->error_message = strdup("author: email can only be defined once");
				node->error = 1019;
			}
			else
				node->u.author->email = node->children[i];
			break;
		    case LSMP_AFFILIATION:
			if(node->u.author->affiliation != NULL)
			{
				node->error_message = strdup("author: affiliation can only be defined once");
				node->error = 1020;
			}
			else
				node->u.author->affiliation = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.author->lastname == NULL)
		{
			node->error_message = strdup("author: lastname must be defined");
			node->error = 1021;
		}
		if(node->u.author->firstname == NULL)
		{
			node->error_message = strdup("author: firstname must be defined");
			node->error = 1022;
		}
		if(node->u.author->email == NULL)
		{
			node->error_message = strdup("author: email must be defined");
			node->error = 1023;
		}
		if(node->u.author->affiliation == NULL)
		{
			node->error_message = strdup("author: affiliation must be defined");
			node->error = 1024;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("author: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"author: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1025;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_FIRSTNAME:
		node->u.firstname = (struct firstname *) malloc(sizeof(struct firstname));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("firstname: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"firstname: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1026;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_LASTNAME:
		node->u.lastname = (struct lastname *) malloc(sizeof(struct lastname));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("lastname: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"lastname: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1027;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_AFFILIATION:
		node->u.affiliation = (struct affiliation *) malloc(sizeof(struct affiliation));
		node->u.affiliation->organization =NULL;
		node->u.affiliation->address =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_ORGANIZATION:
			if(node->u.affiliation->organization != NULL)
			{
				node->error_message = strdup("affiliation: organization can only be defined once");
				node->error = 1028;
			}
			else
				node->u.affiliation->organization = node->children[i];
			break;
		    case LSMP_ADDRESS:
			if(node->u.affiliation->address != NULL)
			{
				node->error_message = strdup("affiliation: address can only be defined once");
				node->error = 1029;
			}
			else
				node->u.affiliation->address = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.affiliation->organization == NULL)
		{
			node->error_message = strdup("affiliation: organization must be defined");
			node->error = 1030;
		}
		if(node->u.affiliation->address == NULL)
		{
			node->error_message = strdup("affiliation: address must be defined");
			node->error = 1031;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("affiliation: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"affiliation: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1032;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_ORGANIZATION:
		node->u.organization = (struct organization *) malloc(sizeof(struct organization));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("organization: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"organization: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1033;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_ADDRESS:
		node->u.address = (struct address *) malloc(sizeof(struct address));
		node->u.address->line = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.address->n_line = 0;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_LINE:
			node->u.address->line[node->u.address->n_line++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.address->n_line < 1)
		{
			node->error = 1034;
			node->error_message = strdup("address line must be defined at least once");
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("address: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"address: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1035;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_LINE:
		node->u.line = (struct line *) malloc(sizeof(struct line));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("line: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"line: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1036;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_EMAIL:
		node->u.email = (struct email *) malloc(sizeof(struct email));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("email: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"email: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1037;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_URL:
		node->u.url = (struct url *) malloc(sizeof(struct url));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("url: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"url: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1038;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_DESCRIPTION:
		node->u.description = (struct description *) malloc(sizeof(struct description));
		node->u.description->abstract =NULL;
		node->u.description->msc2000 =NULL;
		node->u.description->keywords =NULL;
		node->u.description->software =NULL;
		node->u.description->detail =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_ABSTRACT:
			if(node->u.description->abstract != NULL)
			{
				node->error_message = strdup("description: abstract can only be defined once");
				node->error = 1039;
			}
			else
				node->u.description->abstract = node->children[i];
			break;
		    case LSMP_MSC2000:
			if(node->u.description->msc2000 != NULL)
			{
				node->error_message = strdup("description: msc2000 can only be defined once");
				node->error = 1040;
			}
			else
				node->u.description->msc2000 = node->children[i];
			break;
		    case LSMP_KEYWORDS:
			if(node->u.description->keywords != NULL)
			{
				node->error_message = strdup("description: keywords can only be defined once");
				node->error = 1041;
			}
			else
				node->u.description->keywords = node->children[i];
			break;
		    case LSMP_SOFTWARE:
			if(node->u.description->software != NULL)
			{
				node->error_message = strdup("description: software can only be defined once");
				node->error = 1042;
			}
			else
				node->u.description->software = node->children[i];
			break;
		    case LSMP_DETAIL:
			if(node->u.description->detail != NULL)
			{
				node->error_message = strdup("description: detail can only be defined once");
				node->error = 1043;
			}
			else
				node->u.description->detail = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.description->abstract == NULL)
		{
			node->error_message = strdup("description: abstract must be defined");
			node->error = 1044;
		}
		if(node->u.description->msc2000 == NULL)
		{
			node->error_message = strdup("description: msc2000 must be defined");
			node->error = 1045;
		}
		if(node->u.description->keywords == NULL)
		{
			node->error_message = strdup("description: keywords must be defined");
			node->error = 1046;
		}
		if(node->u.description->software == NULL)
		{
			node->error_message = strdup("description: software must be defined");
			node->error = 1047;
		}
		if(node->u.description->detail == NULL)
		{
			node->error_message = strdup("description: detail must be defined");
			node->error = 1048;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("description: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"description: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1049;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_ABSTRACT:
		node->u.abstract = (struct abstract *) malloc(sizeof(struct abstract));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("abstract: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"abstract: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1050;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_DETAIL:
		node->u.detail = (struct detail *) malloc(sizeof(struct detail));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("detail: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"detail: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1051;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_MSC2000:
		node->u.msc2000 = (struct msc2000 *) malloc(sizeof(struct msc2000));
		node->u.msc2000->primary =NULL;
		node->u.msc2000->secondary = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.msc2000->n_secondary = 0;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_PRIMARY:
			if(node->u.msc2000->primary != NULL)
			{
				node->error_message = strdup("msc2000: primary can only be defined once");
				node->error = 1052;
			}
			else
				node->u.msc2000->primary = node->children[i];
			break;
		    case LSMP_SECONDARY:
			node->u.msc2000->secondary[node->u.msc2000->n_secondary++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.msc2000->primary == NULL)
		{
			node->error_message = strdup("msc2000: primary must be defined");
			node->error = 1053;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("msc2000: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"msc2000: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1054;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_PRIMARY:
		node->u.primary = (struct primary *) malloc(sizeof(struct primary));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("primary: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"primary: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1055;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_SECONDARY:
		node->u.secondary = (struct secondary *) malloc(sizeof(struct secondary));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("secondary: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"secondary: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1056;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_KEYWORDS:
		node->u.keywords = (struct keywords *) malloc(sizeof(struct keywords));
		node->u.keywords->keyword = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.keywords->n_keyword = 0;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_KEYWORD:
			node->u.keywords->keyword[node->u.keywords->n_keyword++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.keywords->n_keyword < 1)
		{
			node->error = 1057;
			node->error_message = strdup("keywords keyword must be defined at least once");
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("keywords: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"keywords: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1058;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_KEYWORD:
		node->u.keyword = (struct keyword *) malloc(sizeof(struct keyword));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("keyword: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"keyword: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1059;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_SOFTWARE:
		node->u.software = (struct software *) malloc(sizeof(struct software));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("software: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"software: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1060;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_GEOMETRIES:
		node->u.geometries = (struct geometries *) malloc(sizeof(struct geometries));
		node->u.geometries->geometry = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.geometries->n_geometry = 0;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_GEOMETRY:
			node->u.geometries->geometry[node->u.geometries->n_geometry++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.geometries->n_geometry < 1)
		{
			node->error = 1061;
			node->error_message = strdup("geometries geometry must be defined at least once");
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("geometries: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"geometries: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1062;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_GEOMETRY:
		node->u.geometry = (struct geometry *) malloc(sizeof(struct geometry));
		node->u.geometry->lineSet =NULL;
		node->u.geometry->bndbox =NULL;
		node->u.geometry->vectorField = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.geometry->n_vectorField = 0;
		node->u.geometry->primitive =NULL;
		node->u.geometry->labelAtt =NULL;
		node->u.geometry->pointSet =NULL;
		node->u.geometry->material =NULL;
		node->u.geometry->center =NULL;
		node->u.geometry->faceSet =NULL;
		node->u.geometry->visible = NULL;
		node->u.geometry->name = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_LINESET:
			if(node->u.geometry->lineSet != NULL)
			{
				node->error_message = strdup("geometry: lineSet can only be defined once");
				node->error = 1063;
			}
			else
				node->u.geometry->lineSet = node->children[i];
			break;
		    case LSMP_BNDBOX:
			if(node->u.geometry->bndbox != NULL)
			{
				node->error_message = strdup("geometry: bndbox can only be defined once");
				node->error = 1064;
			}
			else
				node->u.geometry->bndbox = node->children[i];
			break;
		    case LSMP_VECTORFIELD:
			node->u.geometry->vectorField[node->u.geometry->n_vectorField++] = node->children[i];
			break;
		    case LSMP_PRIMITIVE:
			if(node->u.geometry->primitive != NULL)
			{
				node->error_message = strdup("geometry: primitive can only be defined once");
				node->error = 1065;
			}
			else
				node->u.geometry->primitive = node->children[i];
			break;
		    case LSMP_LABELATT:
			if(node->u.geometry->labelAtt != NULL)
			{
				node->error_message = strdup("geometry: labelAtt can only be defined once");
				node->error = 1066;
			}
			else
				node->u.geometry->labelAtt = node->children[i];
			break;
		    case LSMP_POINTSET:
			if(node->u.geometry->pointSet != NULL)
			{
				node->error_message = strdup("geometry: pointSet can only be defined once");
				node->error = 1067;
			}
			else
				node->u.geometry->pointSet = node->children[i];
			break;
		    case LSMP_MATERIAL:
			if(node->u.geometry->material != NULL)
			{
				node->error_message = strdup("geometry: material can only be defined once");
				node->error = 1068;
			}
			else
				node->u.geometry->material = node->children[i];
			break;
		    case LSMP_CENTER:
			if(node->u.geometry->center != NULL)
			{
				node->error_message = strdup("geometry: center can only be defined once");
				node->error = 1069;
			}
			else
				node->u.geometry->center = node->children[i];
			break;
		    case LSMP_FACESET:
			if(node->u.geometry->faceSet != NULL)
			{
				node->error_message = strdup("geometry: faceSet can only be defined once");
				node->error = 1070;
			}
			else
				node->u.geometry->faceSet = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.geometry->lineSet == NULL)
		{
			node->error_message = strdup("geometry: lineSet must be defined");
			node->error = 1071;
		}
		if(node->u.geometry->primitive == NULL)
		{
			node->error_message = strdup("geometry: primitive must be defined");
			node->error = 1072;
		}
		if(node->u.geometry->pointSet == NULL)
		{
			node->error_message = strdup("geometry: pointSet must be defined");
			node->error = 1073;
		}
		if(node->u.geometry->faceSet == NULL)
		{
			node->error_message = strdup("geometry: faceSet must be defined");
			node->error = 1074;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"visible"))
				node->u.geometry->visible = node->attr[i+1];
			else if(!strcmp(node->attr[i],"name"))
				node->u.geometry->name = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("geometry: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"geometry: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1075;
			}
		}
		/** Check attributes **/
		/* visible [(show|hide)] ["show"] */
		if(node->u.geometry->visible == NULL) {}
		else if(!strcmp(node->u.geometry->visible,"show")) {}
		else if(!strcmp(node->u.geometry->visible,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("geometry: atribute visible (%s) not in specification (show|hide)\n") + strlen(node->u.geometry->visible));
			sprintf(node->error_message,"geometry: atribute visible (%s) not in specification (show|hide)\n",node->u.geometry->visible);
		}
		if(node->u.geometry->visible == NULL)
			node->u.geometry->visible = "show";
		/* name [CDATA] [#IMPLIED] */
		break;
	case LSMP_POINTSET:
		node->u.pointSet = (struct pointSet *) malloc(sizeof(struct pointSet));
		node->u.pointSet->points =NULL;
		node->u.pointSet->textures =NULL;
		node->u.pointSet->colors =NULL;
		node->u.pointSet->normals =NULL;
		node->u.pointSet->normal = NULL;
		node->u.pointSet->point = NULL;
		node->u.pointSet->dim = NULL;
		node->u.pointSet->normalArrow = NULL;
		node->u.pointSet->color = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_POINTS:
			if(node->u.pointSet->points != NULL)
			{
				node->error_message = strdup("pointSet: points can only be defined once");
				node->error = 1078;
			}
			else
				node->u.pointSet->points = node->children[i];
			break;
		    case LSMP_TEXTURES:
			if(node->u.pointSet->textures != NULL)
			{
				node->error_message = strdup("pointSet: textures can only be defined once");
				node->error = 1079;
			}
			else
				node->u.pointSet->textures = node->children[i];
			break;
		    case LSMP_COLORS:
			if(node->u.pointSet->colors != NULL)
			{
				node->error_message = strdup("pointSet: colors can only be defined once");
				node->error = 1080;
			}
			else
				node->u.pointSet->colors = node->children[i];
			break;
		    case LSMP_NORMALS:
			if(node->u.pointSet->normals != NULL)
			{
				node->error_message = strdup("pointSet: normals can only be defined once");
				node->error = 1081;
			}
			else
				node->u.pointSet->normals = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.pointSet->points == NULL)
		{
			node->error_message = strdup("pointSet: points must be defined");
			node->error = 1082;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"normal"))
				node->u.pointSet->normal = node->attr[i+1];
			else if(!strcmp(node->attr[i],"point"))
				node->u.pointSet->point = node->attr[i+1];
			else if(!strcmp(node->attr[i],"dim"))
				node->u.pointSet->dim = node->attr[i+1];
			else if(!strcmp(node->attr[i],"normalArrow"))
				node->u.pointSet->normalArrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"color"))
				node->u.pointSet->color = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("pointSet: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"pointSet: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1083;
			}
		}
		/** Check attributes **/
		/* normal [(show|hide)] ["hide"] */
		if(node->u.pointSet->normal == NULL) {}
		else if(!strcmp(node->u.pointSet->normal,"show")) {}
		else if(!strcmp(node->u.pointSet->normal,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("pointSet: atribute normal (%s) not in specification (show|hide)\n") + strlen(node->u.pointSet->normal));
			sprintf(node->error_message,"pointSet: atribute normal (%s) not in specification (show|hide)\n",node->u.pointSet->normal);
		}
		if(node->u.pointSet->normal == NULL)
			node->u.pointSet->normal = "hide";
		/* point [(show|hide)] ["show"] */
		if(node->u.pointSet->point == NULL) {}
		else if(!strcmp(node->u.pointSet->point,"show")) {}
		else if(!strcmp(node->u.pointSet->point,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("pointSet: atribute point (%s) not in specification (show|hide)\n") + strlen(node->u.pointSet->point));
			sprintf(node->error_message,"pointSet: atribute point (%s) not in specification (show|hide)\n",node->u.pointSet->point);
		}
		if(node->u.pointSet->point == NULL)
			node->u.pointSet->point = "show";
		/* dim [CDATA] [#REQUIRED] */
		if(node->u.pointSet->dim == NULL)
		{ node->error_message = strdup("pointSet: atribute dim must be specified\n"); node->error = 1088; }
		/* normalArrow [(show|hide)] ["hide"] */
		if(node->u.pointSet->normalArrow == NULL) {}
		else if(!strcmp(node->u.pointSet->normalArrow,"show")) {}
		else if(!strcmp(node->u.pointSet->normalArrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("pointSet: atribute normalArrow (%s) not in specification (show|hide)\n") + strlen(node->u.pointSet->normalArrow));
			sprintf(node->error_message,"pointSet: atribute normalArrow (%s) not in specification (show|hide)\n",node->u.pointSet->normalArrow);
		}
		if(node->u.pointSet->normalArrow == NULL)
			node->u.pointSet->normalArrow = "hide";
		/* color [(show|hide)] ["hide"] */
		if(node->u.pointSet->color == NULL) {}
		else if(!strcmp(node->u.pointSet->color,"show")) {}
		else if(!strcmp(node->u.pointSet->color,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("pointSet: atribute color (%s) not in specification (show|hide)\n") + strlen(node->u.pointSet->color));
			sprintf(node->error_message,"pointSet: atribute color (%s) not in specification (show|hide)\n",node->u.pointSet->color);
		}
		if(node->u.pointSet->color == NULL)
			node->u.pointSet->color = "hide";
		break;
	case LSMP_LINESET:
		node->u.lineSet = (struct lineSet *) malloc(sizeof(struct lineSet));
		node->u.lineSet->colors =NULL;
		node->u.lineSet->normals =NULL;
		node->u.lineSet->lines =NULL;
		node->u.lineSet->normal = NULL;
		node->u.lineSet->line = NULL;
		node->u.lineSet->startArrow = NULL;
		node->u.lineSet->arrow = NULL;
		node->u.lineSet->normalArrow = NULL;
		node->u.lineSet->color = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_COLORS:
			if(node->u.lineSet->colors != NULL)
			{
				node->error_message = strdup("lineSet: colors can only be defined once");
				node->error = 1093;
			}
			else
				node->u.lineSet->colors = node->children[i];
			break;
		    case LSMP_NORMALS:
			if(node->u.lineSet->normals != NULL)
			{
				node->error_message = strdup("lineSet: normals can only be defined once");
				node->error = 1094;
			}
			else
				node->u.lineSet->normals = node->children[i];
			break;
		    case LSMP_LINES:
			if(node->u.lineSet->lines != NULL)
			{
				node->error_message = strdup("lineSet: lines can only be defined once");
				node->error = 1095;
			}
			else
				node->u.lineSet->lines = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.lineSet->lines == NULL)
		{
			node->error_message = strdup("lineSet: lines must be defined");
			node->error = 1096;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"normal"))
				node->u.lineSet->normal = node->attr[i+1];
			else if(!strcmp(node->attr[i],"line"))
				node->u.lineSet->line = node->attr[i+1];
			else if(!strcmp(node->attr[i],"startArrow"))
				node->u.lineSet->startArrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"arrow"))
				node->u.lineSet->arrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"normalArrow"))
				node->u.lineSet->normalArrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"color"))
				node->u.lineSet->color = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("lineSet: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"lineSet: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1097;
			}
		}
		/** Check attributes **/
		/* normal [(show|hide)] ["hide"] */
		if(node->u.lineSet->normal == NULL) {}
		else if(!strcmp(node->u.lineSet->normal,"show")) {}
		else if(!strcmp(node->u.lineSet->normal,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("lineSet: atribute normal (%s) not in specification (show|hide)\n") + strlen(node->u.lineSet->normal));
			sprintf(node->error_message,"lineSet: atribute normal (%s) not in specification (show|hide)\n",node->u.lineSet->normal);
		}
		if(node->u.lineSet->normal == NULL)
			node->u.lineSet->normal = "hide";
		/* line [(show|hide)] ["show"] */
		if(node->u.lineSet->line == NULL) {}
		else if(!strcmp(node->u.lineSet->line,"show")) {}
		else if(!strcmp(node->u.lineSet->line,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("lineSet: atribute line (%s) not in specification (show|hide)\n") + strlen(node->u.lineSet->line));
			sprintf(node->error_message,"lineSet: atribute line (%s) not in specification (show|hide)\n",node->u.lineSet->line);
		}
		if(node->u.lineSet->line == NULL)
			node->u.lineSet->line = "show";
		/* startArrow [(show|hide)] ["hide"] */
		if(node->u.lineSet->startArrow == NULL) {}
		else if(!strcmp(node->u.lineSet->startArrow,"show")) {}
		else if(!strcmp(node->u.lineSet->startArrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("lineSet: atribute startArrow (%s) not in specification (show|hide)\n") + strlen(node->u.lineSet->startArrow));
			sprintf(node->error_message,"lineSet: atribute startArrow (%s) not in specification (show|hide)\n",node->u.lineSet->startArrow);
		}
		if(node->u.lineSet->startArrow == NULL)
			node->u.lineSet->startArrow = "hide";
		/* arrow [(show|hide)] ["hide"] */
		if(node->u.lineSet->arrow == NULL) {}
		else if(!strcmp(node->u.lineSet->arrow,"show")) {}
		else if(!strcmp(node->u.lineSet->arrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("lineSet: atribute arrow (%s) not in specification (show|hide)\n") + strlen(node->u.lineSet->arrow));
			sprintf(node->error_message,"lineSet: atribute arrow (%s) not in specification (show|hide)\n",node->u.lineSet->arrow);
		}
		if(node->u.lineSet->arrow == NULL)
			node->u.lineSet->arrow = "hide";
		/* normalArrow [(show|hide)] ["hide"] */
		if(node->u.lineSet->normalArrow == NULL) {}
		else if(!strcmp(node->u.lineSet->normalArrow,"show")) {}
		else if(!strcmp(node->u.lineSet->normalArrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("lineSet: atribute normalArrow (%s) not in specification (show|hide)\n") + strlen(node->u.lineSet->normalArrow));
			sprintf(node->error_message,"lineSet: atribute normalArrow (%s) not in specification (show|hide)\n",node->u.lineSet->normalArrow);
		}
		if(node->u.lineSet->normalArrow == NULL)
			node->u.lineSet->normalArrow = "hide";
		/* color [(show|hide)] ["hide"] */
		if(node->u.lineSet->color == NULL) {}
		else if(!strcmp(node->u.lineSet->color,"show")) {}
		else if(!strcmp(node->u.lineSet->color,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("lineSet: atribute color (%s) not in specification (show|hide)\n") + strlen(node->u.lineSet->color));
			sprintf(node->error_message,"lineSet: atribute color (%s) not in specification (show|hide)\n",node->u.lineSet->color);
		}
		if(node->u.lineSet->color == NULL)
			node->u.lineSet->color = "hide";
		break;
	case LSMP_FACESET:
		node->u.faceSet = (struct faceSet *) malloc(sizeof(struct faceSet));
		node->u.faceSet->faces =NULL;
		node->u.faceSet->textures =NULL;
		node->u.faceSet->edges =NULL;
		node->u.faceSet->boundaries =NULL;
		node->u.faceSet->colors =NULL;
		node->u.faceSet->colorsBack =NULL;
		node->u.faceSet->neighbours =NULL;
		node->u.faceSet->normals =NULL;
		node->u.faceSet->boundary = NULL;
		node->u.faceSet->normal = NULL;
		node->u.faceSet->colorBackLocal = NULL;
		node->u.faceSet->texture = NULL;
		node->u.faceSet->face = NULL;
		node->u.faceSet->colorBackGlobal = NULL;
		node->u.faceSet->normalArrow = NULL;
		node->u.faceSet->backface = NULL;
		node->u.faceSet->color = NULL;
		node->u.faceSet->edge = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_FACES:
			if(node->u.faceSet->faces != NULL)
			{
				node->error_message = strdup("faceSet: faces can only be defined once");
				node->error = 1110;
			}
			else
				node->u.faceSet->faces = node->children[i];
			break;
		    case LSMP_TEXTURES:
			if(node->u.faceSet->textures != NULL)
			{
				node->error_message = strdup("faceSet: textures can only be defined once");
				node->error = 1111;
			}
			else
				node->u.faceSet->textures = node->children[i];
			break;
		    case LSMP_EDGES:
			if(node->u.faceSet->edges != NULL)
			{
				node->error_message = strdup("faceSet: edges can only be defined once");
				node->error = 1112;
			}
			else
				node->u.faceSet->edges = node->children[i];
			break;
		    case LSMP_BOUNDARIES:
			if(node->u.faceSet->boundaries != NULL)
			{
				node->error_message = strdup("faceSet: boundaries can only be defined once");
				node->error = 1113;
			}
			else
				node->u.faceSet->boundaries = node->children[i];
			break;
		    case LSMP_COLORS:
			if(node->u.faceSet->colors != NULL)
			{
				node->error_message = strdup("faceSet: colors can only be defined once");
				node->error = 1114;
			}
			else
				node->u.faceSet->colors = node->children[i];
			break;
		    case LSMP_COLORSBACK:
			if(node->u.faceSet->colorsBack != NULL)
			{
				node->error_message = strdup("faceSet: colorsBack can only be defined once");
				node->error = 1115;
			}
			else
				node->u.faceSet->colorsBack = node->children[i];
			break;
		    case LSMP_NEIGHBOURS:
			if(node->u.faceSet->neighbours != NULL)
			{
				node->error_message = strdup("faceSet: neighbours can only be defined once");
				node->error = 1116;
			}
			else
				node->u.faceSet->neighbours = node->children[i];
			break;
		    case LSMP_NORMALS:
			if(node->u.faceSet->normals != NULL)
			{
				node->error_message = strdup("faceSet: normals can only be defined once");
				node->error = 1117;
			}
			else
				node->u.faceSet->normals = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.faceSet->faces == NULL)
		{
			node->error_message = strdup("faceSet: faces must be defined");
			node->error = 1118;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"boundary"))
				node->u.faceSet->boundary = node->attr[i+1];
			else if(!strcmp(node->attr[i],"normal"))
				node->u.faceSet->normal = node->attr[i+1];
			else if(!strcmp(node->attr[i],"colorBackLocal"))
				node->u.faceSet->colorBackLocal = node->attr[i+1];
			else if(!strcmp(node->attr[i],"texture"))
				node->u.faceSet->texture = node->attr[i+1];
			else if(!strcmp(node->attr[i],"face"))
				node->u.faceSet->face = node->attr[i+1];
			else if(!strcmp(node->attr[i],"colorBackGlobal"))
				node->u.faceSet->colorBackGlobal = node->attr[i+1];
			else if(!strcmp(node->attr[i],"normalArrow"))
				node->u.faceSet->normalArrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"backface"))
				node->u.faceSet->backface = node->attr[i+1];
			else if(!strcmp(node->attr[i],"color"))
				node->u.faceSet->color = node->attr[i+1];
			else if(!strcmp(node->attr[i],"edge"))
				node->u.faceSet->edge = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("faceSet: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"faceSet: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1119;
			}
		}
		/** Check attributes **/
		/* boundary [(show|hide)] ["hide"] */
		if(node->u.faceSet->boundary == NULL) {}
		else if(!strcmp(node->u.faceSet->boundary,"show")) {}
		else if(!strcmp(node->u.faceSet->boundary,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute boundary (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->boundary));
			sprintf(node->error_message,"faceSet: atribute boundary (%s) not in specification (show|hide)\n",node->u.faceSet->boundary);
		}
		if(node->u.faceSet->boundary == NULL)
			node->u.faceSet->boundary = "hide";
		/* normal [(show|hide)] ["hide"] */
		if(node->u.faceSet->normal == NULL) {}
		else if(!strcmp(node->u.faceSet->normal,"show")) {}
		else if(!strcmp(node->u.faceSet->normal,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute normal (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->normal));
			sprintf(node->error_message,"faceSet: atribute normal (%s) not in specification (show|hide)\n",node->u.faceSet->normal);
		}
		if(node->u.faceSet->normal == NULL)
			node->u.faceSet->normal = "hide";
		/* colorBackLocal [(show|hide)] ["hide"] */
		if(node->u.faceSet->colorBackLocal == NULL) {}
		else if(!strcmp(node->u.faceSet->colorBackLocal,"show")) {}
		else if(!strcmp(node->u.faceSet->colorBackLocal,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute colorBackLocal (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->colorBackLocal));
			sprintf(node->error_message,"faceSet: atribute colorBackLocal (%s) not in specification (show|hide)\n",node->u.faceSet->colorBackLocal);
		}
		if(node->u.faceSet->colorBackLocal == NULL)
			node->u.faceSet->colorBackLocal = "hide";
		/* texture [(show|hide)] ["hide"] */
		if(node->u.faceSet->texture == NULL) {}
		else if(!strcmp(node->u.faceSet->texture,"show")) {}
		else if(!strcmp(node->u.faceSet->texture,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute texture (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->texture));
			sprintf(node->error_message,"faceSet: atribute texture (%s) not in specification (show|hide)\n",node->u.faceSet->texture);
		}
		if(node->u.faceSet->texture == NULL)
			node->u.faceSet->texture = "hide";
		/* face [(show|hide)] ["show"] */
		if(node->u.faceSet->face == NULL) {}
		else if(!strcmp(node->u.faceSet->face,"show")) {}
		else if(!strcmp(node->u.faceSet->face,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute face (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->face));
			sprintf(node->error_message,"faceSet: atribute face (%s) not in specification (show|hide)\n",node->u.faceSet->face);
		}
		if(node->u.faceSet->face == NULL)
			node->u.faceSet->face = "show";
		/* colorBackGlobal [(show|hide)] ["hide"] */
		if(node->u.faceSet->colorBackGlobal == NULL) {}
		else if(!strcmp(node->u.faceSet->colorBackGlobal,"show")) {}
		else if(!strcmp(node->u.faceSet->colorBackGlobal,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute colorBackGlobal (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->colorBackGlobal));
			sprintf(node->error_message,"faceSet: atribute colorBackGlobal (%s) not in specification (show|hide)\n",node->u.faceSet->colorBackGlobal);
		}
		if(node->u.faceSet->colorBackGlobal == NULL)
			node->u.faceSet->colorBackGlobal = "hide";
		/* normalArrow [(show|hide)] ["hide"] */
		if(node->u.faceSet->normalArrow == NULL) {}
		else if(!strcmp(node->u.faceSet->normalArrow,"show")) {}
		else if(!strcmp(node->u.faceSet->normalArrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute normalArrow (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->normalArrow));
			sprintf(node->error_message,"faceSet: atribute normalArrow (%s) not in specification (show|hide)\n",node->u.faceSet->normalArrow);
		}
		if(node->u.faceSet->normalArrow == NULL)
			node->u.faceSet->normalArrow = "hide";
		/* backface [(show|hide)] ["show"] */
		if(node->u.faceSet->backface == NULL) {}
		else if(!strcmp(node->u.faceSet->backface,"show")) {}
		else if(!strcmp(node->u.faceSet->backface,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute backface (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->backface));
			sprintf(node->error_message,"faceSet: atribute backface (%s) not in specification (show|hide)\n",node->u.faceSet->backface);
		}
		if(node->u.faceSet->backface == NULL)
			node->u.faceSet->backface = "show";
		/* color [(show|hide)] ["hide"] */
		if(node->u.faceSet->color == NULL) {}
		else if(!strcmp(node->u.faceSet->color,"show")) {}
		else if(!strcmp(node->u.faceSet->color,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute color (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->color));
			sprintf(node->error_message,"faceSet: atribute color (%s) not in specification (show|hide)\n",node->u.faceSet->color);
		}
		if(node->u.faceSet->color == NULL)
			node->u.faceSet->color = "hide";
		/* edge [(show|hide)] ["show"] */
		if(node->u.faceSet->edge == NULL) {}
		else if(!strcmp(node->u.faceSet->edge,"show")) {}
		else if(!strcmp(node->u.faceSet->edge,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("faceSet: atribute edge (%s) not in specification (show|hide)\n") + strlen(node->u.faceSet->edge));
			sprintf(node->error_message,"faceSet: atribute edge (%s) not in specification (show|hide)\n",node->u.faceSet->edge);
		}
		if(node->u.faceSet->edge == NULL)
			node->u.faceSet->edge = "show";
		break;
	case LSMP_VECTORFIELD:
		node->u.vectorField = (struct vectorField *) malloc(sizeof(struct vectorField));
		node->u.vectorField->vectors =NULL;
		node->u.vectorField->colors =NULL;
		node->u.vectorField->arrow = NULL;
		node->u.vectorField->base = NULL;
		node->u.vectorField->material = NULL;
		node->u.vectorField->name = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_VECTORS:
			if(node->u.vectorField->vectors != NULL)
			{
				node->error_message = strdup("vectorField: vectors can only be defined once");
				node->error = 1140;
			}
			else
				node->u.vectorField->vectors = node->children[i];
			break;
		    case LSMP_COLORS:
			if(node->u.vectorField->colors != NULL)
			{
				node->error_message = strdup("vectorField: colors can only be defined once");
				node->error = 1141;
			}
			else
				node->u.vectorField->colors = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.vectorField->vectors == NULL)
		{
			node->error_message = strdup("vectorField: vectors must be defined");
			node->error = 1142;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"arrow"))
				node->u.vectorField->arrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"base"))
				node->u.vectorField->base = node->attr[i+1];
			else if(!strcmp(node->attr[i],"material"))
				node->u.vectorField->material = node->attr[i+1];
			else if(!strcmp(node->attr[i],"name"))
				node->u.vectorField->name = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("vectorField: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"vectorField: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1143;
			}
		}
		/** Check attributes **/
		/* arrow [(show|hide)] ["hide"] */
		if(node->u.vectorField->arrow == NULL) {}
		else if(!strcmp(node->u.vectorField->arrow,"show")) {}
		else if(!strcmp(node->u.vectorField->arrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("vectorField: atribute arrow (%s) not in specification (show|hide)\n") + strlen(node->u.vectorField->arrow));
			sprintf(node->error_message,"vectorField: atribute arrow (%s) not in specification (show|hide)\n",node->u.vectorField->arrow);
		}
		if(node->u.vectorField->arrow == NULL)
			node->u.vectorField->arrow = "hide";
		/* base [(vertex|element)] ["vertex"] */
		if(node->u.vectorField->base == NULL) {}
		else if(!strcmp(node->u.vectorField->base,"vertex")) {}
		else if(!strcmp(node->u.vectorField->base,"element")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("vectorField: atribute base (%s) not in specification (vertex|element)\n") + strlen(node->u.vectorField->base));
			sprintf(node->error_message,"vectorField: atribute base (%s) not in specification (vertex|element)\n",node->u.vectorField->base);
		}
		if(node->u.vectorField->base == NULL)
			node->u.vectorField->base = "vertex";
		/* material [(show|hide)] ["hide"] */
		if(node->u.vectorField->material == NULL) {}
		else if(!strcmp(node->u.vectorField->material,"show")) {}
		else if(!strcmp(node->u.vectorField->material,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("vectorField: atribute material (%s) not in specification (show|hide)\n") + strlen(node->u.vectorField->material));
			sprintf(node->error_message,"vectorField: atribute material (%s) not in specification (show|hide)\n",node->u.vectorField->material);
		}
		if(node->u.vectorField->material == NULL)
			node->u.vectorField->material = "hide";
		/* name [CDATA] [#IMPLIED] */
		break;
	case LSMP_POINTS:
		node->u.points = (struct points *) malloc(sizeof(struct points));
		node->u.points->thickness =NULL;
		node->u.points->colorTag =NULL;
		node->u.points->p = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.points->n_p = 0;
		node->u.points->labelAtt =NULL;
		node->u.points->color =NULL;
		node->u.points->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_THICKNESS:
			if(node->u.points->thickness != NULL)
			{
				node->error_message = strdup("points: thickness can only be defined once");
				node->error = 1150;
			}
			else
				node->u.points->thickness = node->children[i];
			break;
		    case LSMP_COLORTAG:
			if(node->u.points->colorTag != NULL)
			{
				node->error_message = strdup("points: colorTag can only be defined once");
				node->error = 1151;
			}
			else
				node->u.points->colorTag = node->children[i];
			break;
		    case LSMP_P:
			node->u.points->p[node->u.points->n_p++] = node->children[i];
			break;
		    case LSMP_LABELATT:
			if(node->u.points->labelAtt != NULL)
			{
				node->error_message = strdup("points: labelAtt can only be defined once");
				node->error = 1152;
			}
			else
				node->u.points->labelAtt = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.points->color != NULL)
			{
				node->error_message = strdup("points: color can only be defined once");
				node->error = 1153;
			}
			else
				node->u.points->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.points->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("points: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"points: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1154;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_P:
		node->u.p = (struct p *) malloc(sizeof(struct p));
		node->u.p->tag = NULL;
		node->u.p->name = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"tag"))
				node->u.p->tag = node->attr[i+1];
			else if(!strcmp(node->attr[i],"name"))
				node->u.p->name = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("p: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"p: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1155;
			}
		}
		/** Check attributes **/
		/* tag [CDATA] [#IMPLIED] */
		/* name [CDATA] [#IMPLIED] */
		break;
	case LSMP_COLORS:
		node->u.colors = (struct colors *) malloc(sizeof(struct colors));
		node->u.colors->c = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.colors->n_c = 0;
		node->u.colors->num = NULL;
		node->u.colors->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_C:
			node->u.colors->c[node->u.colors->n_c++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.colors->num = node->attr[i+1];
			else if(!strcmp(node->attr[i],"type"))
				node->u.colors->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("colors: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"colors: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1156;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		/* type [(rgb|rgba|grey)] ["rgb"] */
		if(node->u.colors->type == NULL) {}
		else if(!strcmp(node->u.colors->type,"rgb")) {}
		else if(!strcmp(node->u.colors->type,"rgba")) {}
		else if(!strcmp(node->u.colors->type,"grey")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("colors: atribute type (%s) not in specification (rgb|rgba|grey)\n") + strlen(node->u.colors->type));
			sprintf(node->error_message,"colors: atribute type (%s) not in specification (rgb|rgba|grey)\n",node->u.colors->type);
		}
		if(node->u.colors->type == NULL)
			node->u.colors->type = "rgb";
		break;
	case LSMP_C:
		node->u.c = (struct c *) malloc(sizeof(struct c));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("c: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"c: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1159;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_COLORSBACK:
		node->u.colorsBack = (struct colorsBack *) malloc(sizeof(struct colorsBack));
		node->u.colorsBack->c = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.colorsBack->n_c = 0;
		node->u.colorsBack->num = NULL;
		node->u.colorsBack->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_C:
			node->u.colorsBack->c[node->u.colorsBack->n_c++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.colorsBack->num = node->attr[i+1];
			else if(!strcmp(node->attr[i],"type"))
				node->u.colorsBack->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("colorsBack: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"colorsBack: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1160;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		/* type [(rgb|rgba|grey)] ["rgb"] */
		if(node->u.colorsBack->type == NULL) {}
		else if(!strcmp(node->u.colorsBack->type,"rgb")) {}
		else if(!strcmp(node->u.colorsBack->type,"rgba")) {}
		else if(!strcmp(node->u.colorsBack->type,"grey")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("colorsBack: atribute type (%s) not in specification (rgb|rgba|grey)\n") + strlen(node->u.colorsBack->type));
			sprintf(node->error_message,"colorsBack: atribute type (%s) not in specification (rgb|rgba|grey)\n",node->u.colorsBack->type);
		}
		if(node->u.colorsBack->type == NULL)
			node->u.colorsBack->type = "rgb";
		break;
	case LSMP_NORMALS:
		node->u.normals = (struct normals *) malloc(sizeof(struct normals));
		node->u.normals->n = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.normals->n_n = 0;
		node->u.normals->thickness =NULL;
		node->u.normals->length =NULL;
		node->u.normals->color =NULL;
		node->u.normals->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_N:
			node->u.normals->n[node->u.normals->n_n++] = node->children[i];
			break;
		    case LSMP_THICKNESS:
			if(node->u.normals->thickness != NULL)
			{
				node->error_message = strdup("normals: thickness can only be defined once");
				node->error = 1163;
			}
			else
				node->u.normals->thickness = node->children[i];
			break;
		    case LSMP_LENGTH:
			if(node->u.normals->length != NULL)
			{
				node->error_message = strdup("normals: length can only be defined once");
				node->error = 1164;
			}
			else
				node->u.normals->length = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.normals->color != NULL)
			{
				node->error_message = strdup("normals: color can only be defined once");
				node->error = 1165;
			}
			else
				node->u.normals->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.normals->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("normals: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"normals: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1166;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_N:
		node->u.n = (struct n *) malloc(sizeof(struct n));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("n: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"n: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1167;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_LINES:
		node->u.lines = (struct lines *) malloc(sizeof(struct lines));
		node->u.lines->thickness =NULL;
		node->u.lines->colorTag =NULL;
		node->u.lines->labelAtt =NULL;
		node->u.lines->l = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.lines->n_l = 0;
		node->u.lines->color =NULL;
		node->u.lines->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_THICKNESS:
			if(node->u.lines->thickness != NULL)
			{
				node->error_message = strdup("lines: thickness can only be defined once");
				node->error = 1168;
			}
			else
				node->u.lines->thickness = node->children[i];
			break;
		    case LSMP_COLORTAG:
			if(node->u.lines->colorTag != NULL)
			{
				node->error_message = strdup("lines: colorTag can only be defined once");
				node->error = 1169;
			}
			else
				node->u.lines->colorTag = node->children[i];
			break;
		    case LSMP_LABELATT:
			if(node->u.lines->labelAtt != NULL)
			{
				node->error_message = strdup("lines: labelAtt can only be defined once");
				node->error = 1170;
			}
			else
				node->u.lines->labelAtt = node->children[i];
			break;
		    case LSMP_L:
			node->u.lines->l[node->u.lines->n_l++] = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.lines->color != NULL)
			{
				node->error_message = strdup("lines: color can only be defined once");
				node->error = 1171;
			}
			else
				node->u.lines->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.lines->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("lines: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"lines: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1172;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_L:
		node->u.l = (struct l *) malloc(sizeof(struct l));
		node->u.l->arrow = NULL;
		node->u.l->tag = NULL;
		node->u.l->name = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"arrow"))
				node->u.l->arrow = node->attr[i+1];
			else if(!strcmp(node->attr[i],"tag"))
				node->u.l->tag = node->attr[i+1];
			else if(!strcmp(node->attr[i],"name"))
				node->u.l->name = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("l: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"l: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1173;
			}
		}
		/** Check attributes **/
		/* arrow [(show|hide)] ["hide"] */
		if(node->u.l->arrow == NULL) {}
		else if(!strcmp(node->u.l->arrow,"show")) {}
		else if(!strcmp(node->u.l->arrow,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("l: atribute arrow (%s) not in specification (show|hide)\n") + strlen(node->u.l->arrow));
			sprintf(node->error_message,"l: atribute arrow (%s) not in specification (show|hide)\n",node->u.l->arrow);
		}
		if(node->u.l->arrow == NULL)
			node->u.l->arrow = "hide";
		/* tag [CDATA] [#IMPLIED] */
		/* name [CDATA] [#IMPLIED] */
		break;
	case LSMP_FACES:
		node->u.faces = (struct faces *) malloc(sizeof(struct faces));
		node->u.faces->f = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.faces->n_f = 0;
		node->u.faces->colorTag =NULL;
		node->u.faces->colorBack =NULL;
		node->u.faces->labelAtt =NULL;
		node->u.faces->color =NULL;
		node->u.faces->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_F:
			node->u.faces->f[node->u.faces->n_f++] = node->children[i];
			break;
		    case LSMP_COLORTAG:
			if(node->u.faces->colorTag != NULL)
			{
				node->error_message = strdup("faces: colorTag can only be defined once");
				node->error = 1176;
			}
			else
				node->u.faces->colorTag = node->children[i];
			break;
		    case LSMP_COLORBACK:
			if(node->u.faces->colorBack != NULL)
			{
				node->error_message = strdup("faces: colorBack can only be defined once");
				node->error = 1177;
			}
			else
				node->u.faces->colorBack = node->children[i];
			break;
		    case LSMP_LABELATT:
			if(node->u.faces->labelAtt != NULL)
			{
				node->error_message = strdup("faces: labelAtt can only be defined once");
				node->error = 1178;
			}
			else
				node->u.faces->labelAtt = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.faces->color != NULL)
			{
				node->error_message = strdup("faces: color can only be defined once");
				node->error = 1179;
			}
			else
				node->u.faces->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.faces->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("faces: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"faces: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1180;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_F:
		node->u.f = (struct f *) malloc(sizeof(struct f));
		node->u.f->tag = NULL;
		node->u.f->name = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"tag"))
				node->u.f->tag = node->attr[i+1];
			else if(!strcmp(node->attr[i],"name"))
				node->u.f->name = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("f: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"f: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1181;
			}
		}
		/** Check attributes **/
		/* tag [CDATA] [#IMPLIED] */
		/* name [CDATA] [#IMPLIED] */
		break;
	case LSMP_EDGES:
		node->u.edges = (struct edges *) malloc(sizeof(struct edges));
		node->u.edges->e = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.edges->n_e = 0;
		node->u.edges->thickness =NULL;
		node->u.edges->colorTag =NULL;
		node->u.edges->labelAtt =NULL;
		node->u.edges->color =NULL;
		node->u.edges->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_E:
			node->u.edges->e[node->u.edges->n_e++] = node->children[i];
			break;
		    case LSMP_THICKNESS:
			if(node->u.edges->thickness != NULL)
			{
				node->error_message = strdup("edges: thickness can only be defined once");
				node->error = 1182;
			}
			else
				node->u.edges->thickness = node->children[i];
			break;
		    case LSMP_COLORTAG:
			if(node->u.edges->colorTag != NULL)
			{
				node->error_message = strdup("edges: colorTag can only be defined once");
				node->error = 1183;
			}
			else
				node->u.edges->colorTag = node->children[i];
			break;
		    case LSMP_LABELATT:
			if(node->u.edges->labelAtt != NULL)
			{
				node->error_message = strdup("edges: labelAtt can only be defined once");
				node->error = 1184;
			}
			else
				node->u.edges->labelAtt = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.edges->color != NULL)
			{
				node->error_message = strdup("edges: color can only be defined once");
				node->error = 1185;
			}
			else
				node->u.edges->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.edges->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("edges: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"edges: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1186;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_E:
		node->u.e = (struct e *) malloc(sizeof(struct e));
		node->u.e->tag = NULL;
		node->u.e->name = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"tag"))
				node->u.e->tag = node->attr[i+1];
			else if(!strcmp(node->attr[i],"name"))
				node->u.e->name = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("e: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"e: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1187;
			}
		}
		/** Check attributes **/
		/* tag [CDATA] [#IMPLIED] */
		/* name [CDATA] [#IMPLIED] */
		break;
	case LSMP_NEIGHBOURS:
		node->u.neighbours = (struct neighbours *) malloc(sizeof(struct neighbours));
		node->u.neighbours->nb = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.neighbours->n_nb = 0;
		node->u.neighbours->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_NB:
			node->u.neighbours->nb[node->u.neighbours->n_nb++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.neighbours->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("neighbours: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"neighbours: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1188;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_NB:
		node->u.nb = (struct nb *) malloc(sizeof(struct nb));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("nb: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"nb: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1189;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_VECTORS:
		node->u.vectors = (struct vectors *) malloc(sizeof(struct vectors));
		node->u.vectors->v = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.vectors->n_v = 0;
		node->u.vectors->thickness =NULL;
		node->u.vectors->length =NULL;
		node->u.vectors->color =NULL;
		node->u.vectors->num = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_V:
			node->u.vectors->v[node->u.vectors->n_v++] = node->children[i];
			break;
		    case LSMP_THICKNESS:
			if(node->u.vectors->thickness != NULL)
			{
				node->error_message = strdup("vectors: thickness can only be defined once");
				node->error = 1190;
			}
			else
				node->u.vectors->thickness = node->children[i];
			break;
		    case LSMP_LENGTH:
			if(node->u.vectors->length != NULL)
			{
				node->error_message = strdup("vectors: length can only be defined once");
				node->error = 1191;
			}
			else
				node->u.vectors->length = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.vectors->color != NULL)
			{
				node->error_message = strdup("vectors: color can only be defined once");
				node->error = 1192;
			}
			else
				node->u.vectors->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.vectors->num = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("vectors: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"vectors: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1193;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		break;
	case LSMP_V:
		node->u.v = (struct v *) malloc(sizeof(struct v));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("v: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"v: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1194;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_TEXTURES:
		node->u.textures = (struct textures *) malloc(sizeof(struct textures));
		node->u.textures->t = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.textures->n_t = 0;
		node->u.textures->image =NULL;
		node->u.textures->num = NULL;
		node->u.textures->dim = NULL;
		node->u.textures->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_T:
			node->u.textures->t[node->u.textures->n_t++] = node->children[i];
			break;
		    case LSMP_IMAGE:
			if(node->u.textures->image != NULL)
			{
				node->error_message = strdup("textures: image can only be defined once");
				node->error = 1195;
			}
			else
				node->u.textures->image = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"num"))
				node->u.textures->num = node->attr[i+1];
			else if(!strcmp(node->attr[i],"dim"))
				node->u.textures->dim = node->attr[i+1];
			else if(!strcmp(node->attr[i],"type"))
				node->u.textures->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("textures: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"textures: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1196;
			}
		}
		/** Check attributes **/
		/* num [CDATA] [#IMPLIED] */
		/* dim [CDATA] [#REQUIRED] */
		if(node->u.textures->dim == NULL)
		{ node->error_message = strdup("textures: atribute dim must be specified\n"); node->error = 1197; }
		/* type [(image)] ["image"] */
		if(node->u.textures->type == NULL) {}
		else if(!strcmp(node->u.textures->type,"image")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("textures: atribute type (%s) not in specification (image)\n") + strlen(node->u.textures->type));
			sprintf(node->error_message,"textures: atribute type (%s) not in specification (image)\n",node->u.textures->type);
		}
		if(node->u.textures->type == NULL)
			node->u.textures->type = "image";
		break;
	case LSMP_T:
		node->u.t = (struct t *) malloc(sizeof(struct t));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("t: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"t: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1200;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_BOUNDARIES:
		node->u.boundaries = (struct boundaries *) malloc(sizeof(struct boundaries));
		node->u.boundaries->thickness =NULL;
		node->u.boundaries->colorTag =NULL;
		node->u.boundaries->labelAtt =NULL;
		node->u.boundaries->color =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_THICKNESS:
			if(node->u.boundaries->thickness != NULL)
			{
				node->error_message = strdup("boundaries: thickness can only be defined once");
				node->error = 1201;
			}
			else
				node->u.boundaries->thickness = node->children[i];
			break;
		    case LSMP_COLORTAG:
			if(node->u.boundaries->colorTag != NULL)
			{
				node->error_message = strdup("boundaries: colorTag can only be defined once");
				node->error = 1202;
			}
			else
				node->u.boundaries->colorTag = node->children[i];
			break;
		    case LSMP_LABELATT:
			if(node->u.boundaries->labelAtt != NULL)
			{
				node->error_message = strdup("boundaries: labelAtt can only be defined once");
				node->error = 1203;
			}
			else
				node->u.boundaries->labelAtt = node->children[i];
			break;
		    case LSMP_COLOR:
			if(node->u.boundaries->color != NULL)
			{
				node->error_message = strdup("boundaries: color can only be defined once");
				node->error = 1204;
			}
			else
				node->u.boundaries->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("boundaries: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"boundaries: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1205;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_COLOR:
		node->u.color = (struct color *) malloc(sizeof(struct color));
		node->u.color->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"type"))
				node->u.color->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("color: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"color: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1206;
			}
		}
		/** Check attributes **/
		/* type [(rgb|rgba|grey)] ["rgb"] */
		if(node->u.color->type == NULL) {}
		else if(!strcmp(node->u.color->type,"rgb")) {}
		else if(!strcmp(node->u.color->type,"rgba")) {}
		else if(!strcmp(node->u.color->type,"grey")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("color: atribute type (%s) not in specification (rgb|rgba|grey)\n") + strlen(node->u.color->type));
			sprintf(node->error_message,"color: atribute type (%s) not in specification (rgb|rgba|grey)\n",node->u.color->type);
		}
		if(node->u.color->type == NULL)
			node->u.color->type = "rgb";
		break;
	case LSMP_COLORBACK:
		node->u.colorBack = (struct colorBack *) malloc(sizeof(struct colorBack));
		node->u.colorBack->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"type"))
				node->u.colorBack->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("colorBack: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"colorBack: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1209;
			}
		}
		/** Check attributes **/
		/* type [(rgb|rgba|grey)] ["rgb"] */
		if(node->u.colorBack->type == NULL) {}
		else if(!strcmp(node->u.colorBack->type,"rgb")) {}
		else if(!strcmp(node->u.colorBack->type,"rgba")) {}
		else if(!strcmp(node->u.colorBack->type,"grey")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("colorBack: atribute type (%s) not in specification (rgb|rgba|grey)\n") + strlen(node->u.colorBack->type));
			sprintf(node->error_message,"colorBack: atribute type (%s) not in specification (rgb|rgba|grey)\n",node->u.colorBack->type);
		}
		if(node->u.colorBack->type == NULL)
			node->u.colorBack->type = "rgb";
		break;
	case LSMP_COLORTAG:
		node->u.colorTag = (struct colorTag *) malloc(sizeof(struct colorTag));
		node->u.colorTag->type = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"type"))
				node->u.colorTag->type = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("colorTag: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"colorTag: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1212;
			}
		}
		/** Check attributes **/
		/* type [(rgb|rgba|grey)] ["rgb"] */
		if(node->u.colorTag->type == NULL) {}
		else if(!strcmp(node->u.colorTag->type,"rgb")) {}
		else if(!strcmp(node->u.colorTag->type,"rgba")) {}
		else if(!strcmp(node->u.colorTag->type,"grey")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("colorTag: atribute type (%s) not in specification (rgb|rgba|grey)\n") + strlen(node->u.colorTag->type));
			sprintf(node->error_message,"colorTag: atribute type (%s) not in specification (rgb|rgba|grey)\n",node->u.colorTag->type);
		}
		if(node->u.colorTag->type == NULL)
			node->u.colorTag->type = "rgb";
		break;
	case LSMP_PRIMITIVE:
		node->u.primitive = (struct primitive *) malloc(sizeof(struct primitive));
		node->u.primitive->cube =NULL;
		node->u.primitive->sphere =NULL;
		node->u.primitive->cylinder =NULL;
		node->u.primitive->cone =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_CUBE:
			if(node->u.primitive->cube != NULL)
			{
				node->error_message = strdup("primitive: cube can only be defined once");
				node->error = 1215;
			}
			else
				node->u.primitive->cube = node->children[i];
			break;
		    case LSMP_SPHERE:
			if(node->u.primitive->sphere != NULL)
			{
				node->error_message = strdup("primitive: sphere can only be defined once");
				node->error = 1216;
			}
			else
				node->u.primitive->sphere = node->children[i];
			break;
		    case LSMP_CYLINDER:
			if(node->u.primitive->cylinder != NULL)
			{
				node->error_message = strdup("primitive: cylinder can only be defined once");
				node->error = 1217;
			}
			else
				node->u.primitive->cylinder = node->children[i];
			break;
		    case LSMP_CONE:
			if(node->u.primitive->cone != NULL)
			{
				node->error_message = strdup("primitive: cone can only be defined once");
				node->error = 1218;
			}
			else
				node->u.primitive->cone = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.primitive->cube == NULL)
		{
			node->error_message = strdup("primitive: cube must be defined");
			node->error = 1219;
		}
		if(node->u.primitive->sphere == NULL)
		{
			node->error_message = strdup("primitive: sphere must be defined");
			node->error = 1220;
		}
		if(node->u.primitive->cylinder == NULL)
		{
			node->error_message = strdup("primitive: cylinder must be defined");
			node->error = 1221;
		}
		if(node->u.primitive->cone == NULL)
		{
			node->error_message = strdup("primitive: cone must be defined");
			node->error = 1222;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("primitive: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"primitive: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1223;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_CUBE:
		node->u.cube = (struct cube *) malloc(sizeof(struct cube));
		node->u.cube->lowerLeft =NULL;
		node->u.cube->upperRight =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_LOWERLEFT:
			if(node->u.cube->lowerLeft != NULL)
			{
				node->error_message = strdup("cube: lowerLeft can only be defined once");
				node->error = 1224;
			}
			else
				node->u.cube->lowerLeft = node->children[i];
			break;
		    case LSMP_UPPERRIGHT:
			if(node->u.cube->upperRight != NULL)
			{
				node->error_message = strdup("cube: upperRight can only be defined once");
				node->error = 1225;
			}
			else
				node->u.cube->upperRight = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.cube->lowerLeft == NULL)
		{
			node->error_message = strdup("cube: lowerLeft must be defined");
			node->error = 1226;
		}
		if(node->u.cube->upperRight == NULL)
		{
			node->error_message = strdup("cube: upperRight must be defined");
			node->error = 1227;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("cube: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"cube: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1228;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_LOWERLEFT:
		node->u.lowerLeft = (struct lowerLeft *) malloc(sizeof(struct lowerLeft));
		node->u.lowerLeft->p =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.lowerLeft->p != NULL)
			{
				node->error_message = strdup("lowerLeft: p can only be defined once");
				node->error = 1229;
			}
			else
				node->u.lowerLeft->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.lowerLeft->p == NULL)
		{
			node->error_message = strdup("lowerLeft: p must be defined");
			node->error = 1230;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("lowerLeft: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"lowerLeft: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1231;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_UPPERRIGHT:
		node->u.upperRight = (struct upperRight *) malloc(sizeof(struct upperRight));
		node->u.upperRight->p =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.upperRight->p != NULL)
			{
				node->error_message = strdup("upperRight: p can only be defined once");
				node->error = 1232;
			}
			else
				node->u.upperRight->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.upperRight->p == NULL)
		{
			node->error_message = strdup("upperRight: p must be defined");
			node->error = 1233;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("upperRight: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"upperRight: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1234;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_SPHERE:
		node->u.sphere = (struct sphere *) malloc(sizeof(struct sphere));
		node->u.sphere->midpoint =NULL;
		node->u.sphere->radius =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_MIDPOINT:
			if(node->u.sphere->midpoint != NULL)
			{
				node->error_message = strdup("sphere: midpoint can only be defined once");
				node->error = 1235;
			}
			else
				node->u.sphere->midpoint = node->children[i];
			break;
		    case LSMP_RADIUS:
			if(node->u.sphere->radius != NULL)
			{
				node->error_message = strdup("sphere: radius can only be defined once");
				node->error = 1236;
			}
			else
				node->u.sphere->radius = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.sphere->midpoint == NULL)
		{
			node->error_message = strdup("sphere: midpoint must be defined");
			node->error = 1237;
		}
		if(node->u.sphere->radius == NULL)
		{
			node->error_message = strdup("sphere: radius must be defined");
			node->error = 1238;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("sphere: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"sphere: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1239;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_MIDPOINT:
		node->u.midpoint = (struct midpoint *) malloc(sizeof(struct midpoint));
		node->u.midpoint->p =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.midpoint->p != NULL)
			{
				node->error_message = strdup("midpoint: p can only be defined once");
				node->error = 1240;
			}
			else
				node->u.midpoint->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.midpoint->p == NULL)
		{
			node->error_message = strdup("midpoint: p must be defined");
			node->error = 1241;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("midpoint: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"midpoint: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1242;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_CYLINDER:
		node->u.cylinder = (struct cylinder *) malloc(sizeof(struct cylinder));
		node->u.cylinder->bottom =NULL;
		node->u.cylinder->top =NULL;
		node->u.cylinder->radius = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);
		node->u.cylinder->n_radius = 0;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_BOTTOM:
			if(node->u.cylinder->bottom != NULL)
			{
				node->error_message = strdup("cylinder: bottom can only be defined once");
				node->error = 1243;
			}
			else
				node->u.cylinder->bottom = node->children[i];
			break;
		    case LSMP_TOP:
			if(node->u.cylinder->top != NULL)
			{
				node->error_message = strdup("cylinder: top can only be defined once");
				node->error = 1244;
			}
			else
				node->u.cylinder->top = node->children[i];
			break;
		    case LSMP_RADIUS:
			node->u.cylinder->radius[node->u.cylinder->n_radius++] = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.cylinder->bottom == NULL)
		{
			node->error_message = strdup("cylinder: bottom must be defined");
			node->error = 1245;
		}
		if(node->u.cylinder->top == NULL)
		{
			node->error_message = strdup("cylinder: top must be defined");
			node->error = 1246;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("cylinder: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"cylinder: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1247;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_BOTTOM:
		node->u.bottom = (struct bottom *) malloc(sizeof(struct bottom));
		node->u.bottom->p =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.bottom->p != NULL)
			{
				node->error_message = strdup("bottom: p can only be defined once");
				node->error = 1248;
			}
			else
				node->u.bottom->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.bottom->p == NULL)
		{
			node->error_message = strdup("bottom: p must be defined");
			node->error = 1249;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("bottom: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"bottom: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1250;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_TOP:
		node->u.top = (struct top *) malloc(sizeof(struct top));
		node->u.top->p =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.top->p != NULL)
			{
				node->error_message = strdup("top: p can only be defined once");
				node->error = 1251;
			}
			else
				node->u.top->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.top->p == NULL)
		{
			node->error_message = strdup("top: p must be defined");
			node->error = 1252;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("top: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"top: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1253;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_CONE:
		node->u.cone = (struct cone *) malloc(sizeof(struct cone));
		node->u.cone->bottom =NULL;
		node->u.cone->top =NULL;
		node->u.cone->radius =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_BOTTOM:
			if(node->u.cone->bottom != NULL)
			{
				node->error_message = strdup("cone: bottom can only be defined once");
				node->error = 1254;
			}
			else
				node->u.cone->bottom = node->children[i];
			break;
		    case LSMP_TOP:
			if(node->u.cone->top != NULL)
			{
				node->error_message = strdup("cone: top can only be defined once");
				node->error = 1255;
			}
			else
				node->u.cone->top = node->children[i];
			break;
		    case LSMP_RADIUS:
			if(node->u.cone->radius != NULL)
			{
				node->error_message = strdup("cone: radius can only be defined once");
				node->error = 1256;
			}
			else
				node->u.cone->radius = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.cone->bottom == NULL)
		{
			node->error_message = strdup("cone: bottom must be defined");
			node->error = 1257;
		}
		if(node->u.cone->top == NULL)
		{
			node->error_message = strdup("cone: top must be defined");
			node->error = 1258;
		}
		if(node->u.cone->radius == NULL)
		{
			node->error_message = strdup("cone: radius must be defined");
			node->error = 1259;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("cone: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"cone: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1260;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_RADIUS:
		node->u.radius = (struct radius *) malloc(sizeof(struct radius));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("radius: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"radius: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1261;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_THICKNESS:
		node->u.thickness = (struct thickness *) malloc(sizeof(struct thickness));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("thickness: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"thickness: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1262;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_LENGTH:
		node->u.length = (struct length *) malloc(sizeof(struct length));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("length: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"length: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1263;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_BNDBOX:
		node->u.bndbox = (struct bndbox *) malloc(sizeof(struct bndbox));
		node->u.bndbox->p =NULL;
		node->u.bndbox->visible = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.bndbox->p != NULL)
			{
				node->error_message = strdup("bndbox: p can only be defined once");
				node->error = 1264;
			}
			else
				node->u.bndbox->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.bndbox->p == NULL)
		{
			node->error_message = strdup("bndbox: p must be defined");
			node->error = 1265;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"visible"))
				node->u.bndbox->visible = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("bndbox: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"bndbox: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1266;
			}
		}
		/** Check attributes **/
		/* visible [(show|hide)] ["show"] */
		if(node->u.bndbox->visible == NULL) {}
		else if(!strcmp(node->u.bndbox->visible,"show")) {}
		else if(!strcmp(node->u.bndbox->visible,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("bndbox: atribute visible (%s) not in specification (show|hide)\n") + strlen(node->u.bndbox->visible));
			sprintf(node->error_message,"bndbox: atribute visible (%s) not in specification (show|hide)\n",node->u.bndbox->visible);
		}
		if(node->u.bndbox->visible == NULL)
			node->u.bndbox->visible = "show";
		break;
	case LSMP_CENTER:
		node->u.center = (struct center *) malloc(sizeof(struct center));
		node->u.center->p =NULL;
		node->u.center->visible = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_P:
			if(node->u.center->p != NULL)
			{
				node->error_message = strdup("center: p can only be defined once");
				node->error = 1269;
			}
			else
				node->u.center->p = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.center->p == NULL)
		{
			node->error_message = strdup("center: p must be defined");
			node->error = 1270;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"visible"))
				node->u.center->visible = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("center: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"center: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1271;
			}
		}
		/** Check attributes **/
		/* visible [(show|hide)] ["show"] */
		if(node->u.center->visible == NULL) {}
		else if(!strcmp(node->u.center->visible,"show")) {}
		else if(!strcmp(node->u.center->visible,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("center: atribute visible (%s) not in specification (show|hide)\n") + strlen(node->u.center->visible));
			sprintf(node->error_message,"center: atribute visible (%s) not in specification (show|hide)\n",node->u.center->visible);
		}
		if(node->u.center->visible == NULL)
			node->u.center->visible = "show";
		break;
	case LSMP_LABELATT:
		node->u.labelAtt = (struct labelAtt *) malloc(sizeof(struct labelAtt));
		node->u.labelAtt->yOffset =NULL;
		node->u.labelAtt->xOffset =NULL;
		node->u.labelAtt->font = NULL;
		node->u.labelAtt->horAlign = NULL;
		node->u.labelAtt->verAlign = NULL;
		node->u.labelAtt->visible = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_YOFFSET:
			if(node->u.labelAtt->yOffset != NULL)
			{
				node->error_message = strdup("labelAtt: yOffset can only be defined once");
				node->error = 1274;
			}
			else
				node->u.labelAtt->yOffset = node->children[i];
			break;
		    case LSMP_XOFFSET:
			if(node->u.labelAtt->xOffset != NULL)
			{
				node->error_message = strdup("labelAtt: xOffset can only be defined once");
				node->error = 1275;
			}
			else
				node->u.labelAtt->xOffset = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"font"))
				node->u.labelAtt->font = node->attr[i+1];
			else if(!strcmp(node->attr[i],"horAlign"))
				node->u.labelAtt->horAlign = node->attr[i+1];
			else if(!strcmp(node->attr[i],"verAlign"))
				node->u.labelAtt->verAlign = node->attr[i+1];
			else if(!strcmp(node->attr[i],"visible"))
				node->u.labelAtt->visible = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("labelAtt: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"labelAtt: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1276;
			}
		}
		/** Check attributes **/
		/* font [(text|fixed|header2|header4|menu)] ["text"] */
		if(node->u.labelAtt->font == NULL) {}
		else if(!strcmp(node->u.labelAtt->font,"text")) {}
		else if(!strcmp(node->u.labelAtt->font,"fixed")) {}
		else if(!strcmp(node->u.labelAtt->font,"header2")) {}
		else if(!strcmp(node->u.labelAtt->font,"header4")) {}
		else if(!strcmp(node->u.labelAtt->font,"menu")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("labelAtt: atribute font (%s) not in specification (text|fixed|header2|header4|menu)\n") + strlen(node->u.labelAtt->font));
			sprintf(node->error_message,"labelAtt: atribute font (%s) not in specification (text|fixed|header2|header4|menu)\n",node->u.labelAtt->font);
		}
		if(node->u.labelAtt->font == NULL)
			node->u.labelAtt->font = "text";
		/* horAlign [(head|center|tail)] ["head"] */
		if(node->u.labelAtt->horAlign == NULL) {}
		else if(!strcmp(node->u.labelAtt->horAlign,"head")) {}
		else if(!strcmp(node->u.labelAtt->horAlign,"center")) {}
		else if(!strcmp(node->u.labelAtt->horAlign,"tail")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("labelAtt: atribute horAlign (%s) not in specification (head|center|tail)\n") + strlen(node->u.labelAtt->horAlign));
			sprintf(node->error_message,"labelAtt: atribute horAlign (%s) not in specification (head|center|tail)\n",node->u.labelAtt->horAlign);
		}
		if(node->u.labelAtt->horAlign == NULL)
			node->u.labelAtt->horAlign = "head";
		/* verAlign [(top|middle|bottom)] ["top"] */
		if(node->u.labelAtt->verAlign == NULL) {}
		else if(!strcmp(node->u.labelAtt->verAlign,"top")) {}
		else if(!strcmp(node->u.labelAtt->verAlign,"middle")) {}
		else if(!strcmp(node->u.labelAtt->verAlign,"bottom")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("labelAtt: atribute verAlign (%s) not in specification (top|middle|bottom)\n") + strlen(node->u.labelAtt->verAlign));
			sprintf(node->error_message,"labelAtt: atribute verAlign (%s) not in specification (top|middle|bottom)\n",node->u.labelAtt->verAlign);
		}
		if(node->u.labelAtt->verAlign == NULL)
			node->u.labelAtt->verAlign = "top";
		/* visible [(show|hide)] ["show"] */
		if(node->u.labelAtt->visible == NULL) {}
		else if(!strcmp(node->u.labelAtt->visible,"show")) {}
		else if(!strcmp(node->u.labelAtt->visible,"hide")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("labelAtt: atribute visible (%s) not in specification (show|hide)\n") + strlen(node->u.labelAtt->visible));
			sprintf(node->error_message,"labelAtt: atribute visible (%s) not in specification (show|hide)\n",node->u.labelAtt->visible);
		}
		if(node->u.labelAtt->visible == NULL)
			node->u.labelAtt->visible = "show";
		break;
	case LSMP_XOFFSET:
		node->u.xOffset = (struct xOffset *) malloc(sizeof(struct xOffset));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("xOffset: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"xOffset: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1285;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_YOFFSET:
		node->u.yOffset = (struct yOffset *) malloc(sizeof(struct yOffset));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("yOffset: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"yOffset: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1286;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_MATERIAL:
		node->u.material = (struct material *) malloc(sizeof(struct material));
		node->u.material->ambientIntensity =NULL;
		node->u.material->diffuse =NULL;
		node->u.material->shininess =NULL;
		node->u.material->emissive =NULL;
		node->u.material->specular =NULL;
		node->u.material->transparency =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_AMBIENTINTENSITY:
			if(node->u.material->ambientIntensity != NULL)
			{
				node->error_message = strdup("material: ambientIntensity can only be defined once");
				node->error = 1287;
			}
			else
				node->u.material->ambientIntensity = node->children[i];
			break;
		    case LSMP_DIFFUSE:
			if(node->u.material->diffuse != NULL)
			{
				node->error_message = strdup("material: diffuse can only be defined once");
				node->error = 1288;
			}
			else
				node->u.material->diffuse = node->children[i];
			break;
		    case LSMP_SHININESS:
			if(node->u.material->shininess != NULL)
			{
				node->error_message = strdup("material: shininess can only be defined once");
				node->error = 1289;
			}
			else
				node->u.material->shininess = node->children[i];
			break;
		    case LSMP_EMISSIVE:
			if(node->u.material->emissive != NULL)
			{
				node->error_message = strdup("material: emissive can only be defined once");
				node->error = 1290;
			}
			else
				node->u.material->emissive = node->children[i];
			break;
		    case LSMP_SPECULAR:
			if(node->u.material->specular != NULL)
			{
				node->error_message = strdup("material: specular can only be defined once");
				node->error = 1291;
			}
			else
				node->u.material->specular = node->children[i];
			break;
		    case LSMP_TRANSPARENCY:
			if(node->u.material->transparency != NULL)
			{
				node->error_message = strdup("material: transparency can only be defined once");
				node->error = 1292;
			}
			else
				node->u.material->transparency = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.material->ambientIntensity == NULL)
		{
			node->error_message = strdup("material: ambientIntensity must be defined");
			node->error = 1293;
		}
		if(node->u.material->diffuse == NULL)
		{
			node->error_message = strdup("material: diffuse must be defined");
			node->error = 1294;
		}
		if(node->u.material->shininess == NULL)
		{
			node->error_message = strdup("material: shininess must be defined");
			node->error = 1295;
		}
		if(node->u.material->emissive == NULL)
		{
			node->error_message = strdup("material: emissive must be defined");
			node->error = 1296;
		}
		if(node->u.material->specular == NULL)
		{
			node->error_message = strdup("material: specular must be defined");
			node->error = 1297;
		}
		if(node->u.material->transparency == NULL)
		{
			node->error_message = strdup("material: transparency must be defined");
			node->error = 1298;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("material: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"material: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1299;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_AMBIENTINTENSITY:
		node->u.ambientIntensity = (struct ambientIntensity *) malloc(sizeof(struct ambientIntensity));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("ambientIntensity: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"ambientIntensity: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1300;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_DIFFUSE:
		node->u.diffuse = (struct diffuse *) malloc(sizeof(struct diffuse));
		node->u.diffuse->color =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_COLOR:
			if(node->u.diffuse->color != NULL)
			{
				node->error_message = strdup("diffuse: color can only be defined once");
				node->error = 1301;
			}
			else
				node->u.diffuse->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.diffuse->color == NULL)
		{
			node->error_message = strdup("diffuse: color must be defined");
			node->error = 1302;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("diffuse: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"diffuse: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1303;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_EMISSIVE:
		node->u.emissive = (struct emissive *) malloc(sizeof(struct emissive));
		node->u.emissive->color =NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_COLOR:
			if(node->u.emissive->color != NULL)
			{
				node->error_message = strdup("emissive: color can only be defined once");
				node->error = 1304;
			}
			else
				node->u.emissive->color = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.emissive->color == NULL)
		{
			node->error_message = strdup("emissive: color must be defined");
			node->error = 1305;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("emissive: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"emissive: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1306;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_SHININESS:
		node->u.shininess = (struct shininess *) malloc(sizeof(struct shininess));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("shininess: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"shininess: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1307;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_SPECULAR:
		node->u.specular = (struct specular *) malloc(sizeof(struct specular));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("specular: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"specular: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1308;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_TRANSPARENCY:
		node->u.transparency = (struct transparency *) malloc(sizeof(struct transparency));

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    default:
		    }
		}
		/** check number of children **/
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("transparency: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"transparency: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1309;
			}
		}
		/** Check attributes **/
		break;
	case LSMP_IMAGE:
		node->u.image = (struct image *) malloc(sizeof(struct image));
		node->u.image->url =NULL;
		node->u.image->repeat = NULL;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
		    case LSMP_URL:
			if(node->u.image->url != NULL)
			{
				node->error_message = strdup("image: url can only be defined once");
				node->error = 1310;
			}
			else
				node->u.image->url = node->children[i];
			break;
		    default:
		    }
		}
		/** check number of children **/
		if(node->u.image->url == NULL)
		{
			node->error_message = strdup("image: url must be defined");
			node->error = 1311;
		}
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
			else if(!strcmp(node->attr[i],"repeat"))
				node->u.image->repeat = node->attr[i+1];
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen("image: Un matched attributes: %s\n")+strlen(node->attr[i]));
				sprintf(node->error_message,"image: Un matched attributes: %s\n",node->attr[i]);
				node->error = 1312;
			}
		}
		/** Check attributes **/
		/* repeat [(no|s|t|st)] ["no"] */
		if(node->u.image->repeat == NULL) {}
		else if(!strcmp(node->u.image->repeat,"no")) {}
		else if(!strcmp(node->u.image->repeat,"s")) {}
		else if(!strcmp(node->u.image->repeat,"t")) {}
		else if(!strcmp(node->u.image->repeat,"st")) {}
		else
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen("image: atribute repeat (%s) not in specification (no|s|t|st)\n") + strlen(node->u.image->repeat));
			sprintf(node->error_message,"image: atribute repeat (%s) not in specification (no|s|t|st)\n",node->u.image->repeat);
		}
		if(node->u.image->repeat == NULL)
			node->u.image->repeat = "no";
		break;
	}
}
