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


#ifndef LSMP_XML_PARSER_H
#define LSMP_XML_PARSER_H
#include <stdio.h>
#include <string.h>

union node_specific_info {
struct jvx_model *jvx_model;
struct meta *meta;
struct version *version;
struct title *title;
struct authors *authors;
struct author *author;
struct firstname *firstname;
struct lastname *lastname;
struct affiliation *affiliation;
struct organization *organization;
struct address *address;
struct line *line;
struct email *email;
struct url *url;
struct description *description;
struct abstract *abstract;
struct detail *detail;
struct msc2000 *msc2000;
struct primary *primary;
struct secondary *secondary;
struct keywords *keywords;
struct keyword *keyword;
struct software *software;
struct geometries *geometries;
struct geometry *geometry;
struct pointSet *pointSet;
struct lineSet *lineSet;
struct faceSet *faceSet;
struct vectorField *vectorField;
struct points *points;
struct p *p;
struct colors *colors;
struct c *c;
struct colorsBack *colorsBack;
struct normals *normals;
struct n *n;
struct lines *lines;
struct l *l;
struct faces *faces;
struct f *f;
struct edges *edges;
struct e *e;
struct neighbours *neighbours;
struct nb *nb;
struct vectors *vectors;
struct v *v;
struct textures *textures;
struct t *t;
struct boundaries *boundaries;
struct color *color;
struct colorBack *colorBack;
struct colorTag *colorTag;
struct primitive *primitive;
struct cube *cube;
struct lowerLeft *lowerLeft;
struct upperRight *upperRight;
struct sphere *sphere;
struct midpoint *midpoint;
struct cylinder *cylinder;
struct bottom *bottom;
struct top *top;
struct cone *cone;
struct radius *radius;
struct thickness *thickness;
struct length *length;
struct bndbox *bndbox;
struct center *center;
struct labelAtt *labelAtt;
struct xOffset *xOffset;
struct yOffset *yOffset;
struct material *material;
struct ambientIntensity *ambientIntensity;
struct diffuse *diffuse;
struct emissive *emissive;
struct shininess *shininess;
struct specular *specular;
struct transparency *transparency;
struct image *image;
} LSMP_jvx_node;

typedef enum { LSMP_LSMP_ERROR,
LSMP_JVX_MODEL,
LSMP_META,
LSMP_VERSION,
LSMP_TITLE,
LSMP_AUTHORS,
LSMP_AUTHOR,
LSMP_FIRSTNAME,
LSMP_LASTNAME,
LSMP_AFFILIATION,
LSMP_ORGANIZATION,
LSMP_ADDRESS,
LSMP_LINE,
LSMP_EMAIL,
LSMP_URL,
LSMP_DESCRIPTION,
LSMP_ABSTRACT,
LSMP_DETAIL,
LSMP_MSC2000,
LSMP_PRIMARY,
LSMP_SECONDARY,
LSMP_KEYWORDS,
LSMP_KEYWORD,
LSMP_SOFTWARE,
LSMP_GEOMETRIES,
LSMP_GEOMETRY,
LSMP_POINTSET,
LSMP_LINESET,
LSMP_FACESET,
LSMP_VECTORFIELD,
LSMP_POINTS,
LSMP_P,
LSMP_COLORS,
LSMP_C,
LSMP_COLORSBACK,
LSMP_NORMALS,
LSMP_N,
LSMP_LINES,
LSMP_L,
LSMP_FACES,
LSMP_F,
LSMP_EDGES,
LSMP_E,
LSMP_NEIGHBOURS,
LSMP_NB,
LSMP_VECTORS,
LSMP_V,
LSMP_TEXTURES,
LSMP_T,
LSMP_BOUNDARIES,
LSMP_COLOR,
LSMP_COLORBACK,
LSMP_COLORTAG,
LSMP_PRIMITIVE,
LSMP_CUBE,
LSMP_LOWERLEFT,
LSMP_UPPERRIGHT,
LSMP_SPHERE,
LSMP_MIDPOINT,
LSMP_CYLINDER,
LSMP_BOTTOM,
LSMP_TOP,
LSMP_CONE,
LSMP_RADIUS,
LSMP_THICKNESS,
LSMP_LENGTH,
LSMP_BNDBOX,
LSMP_CENTER,
LSMP_LABELATT,
LSMP_XOFFSET,
LSMP_YOFFSET,
LSMP_MATERIAL,
LSMP_AMBIENTINTENSITY,
LSMP_DIFFUSE,
LSMP_EMISSIVE,
LSMP_SHININESS,
LSMP_SPECULAR,
LSMP_TRANSPARENCY,
LSMP_IMAGE,
LSMP_LSMP_NODE } LSMP_jvx_type;

typedef struct xml_tree
{
	LSMP_jvx_type type;
	int error;
	char *error_message;
	int n_attr,n_child,alloc_child;
	char *name;
	char **attr;
	char *data;
	struct xml_tree **children;
	union node_specific_info u;
	void *app_info;
} xml_tree;

extern int create_node_specific_info(xml_tree *node);

/*
<!ELEMENT jvx-model			(meta*,version?,title,authors?,description?,geometries)>
*/
struct jvx_model {
	xml_tree **meta;	/* 0 or more */
	int n_meta;
	xml_tree *description;	/* optional */
	xml_tree *geometries;	/* required */
	xml_tree *title;	/* required */
	xml_tree *version;	/* optional */
	xml_tree *authors;	/* optional */

};

/*
<!ELEMENT meta					EMPTY>
*/
struct meta {

	char *date;	/* (#IMPLIED) <!-- Date when this model file was generated. --> */
	char *generator;	/* (#IMPLIED) <!-- Name and version of program which generated this model file. --> */
};

/*
<!ELEMENT version				(#PCDATA)>									<!-- Version of this model file. -->
*/
struct version {
	char *pcdata;

	char *type;	/* (dump|beta|final) ("dump") <!-- Status of this model file. --> */
};

/*
<!ELEMENT title				(#PCDATA)>									<!-- Multi-word title of this model, initial letters are uppercase. -->
*/
struct title {
	char *pcdata;

};

/*
<!ELEMENT authors				(author+)>									<!-- List of authors who created, computed, designed the models. -->
*/
struct authors {
	xml_tree **author;	/* at least one*/
	int n_author;

};

/*
<!ELEMENT author				(firstname,lastname,affiliation,email,url?)>		<!-- Contact address of this author. -->
*/
struct author {
	xml_tree *lastname;	/* required */
	xml_tree *url;	/* optional */
	xml_tree *firstname;	/* required */
	xml_tree *email;	/* required */
	xml_tree *affiliation;	/* required */

};

/*
<!ELEMENT firstname			(#PCDATA)>									<!-- First name, middle name, and title of this author, initial letters are uppercase. -->
*/
struct firstname {
	char *pcdata;

};

/*
<!ELEMENT lastname			(#PCDATA)>									<!-- Last name of this author, initial letters are uppercase. -->
*/
struct lastname {
	char *pcdata;

};

/*
<!ELEMENT affiliation		(organization,address)>
*/
struct affiliation {
	xml_tree *organization;	/* required */
	xml_tree *address;	/* required */

};

/*
<!ELEMENT organization		(#PCDATA)>									<!-- Empty or university or company where author works, initial letters are uppercase. -->
*/
struct organization {
	char *pcdata;

};

/*
<!ELEMENT address				(line+)>										<!-- Postal address for surface mail, initial letters are uppercase. -->
*/
struct address {
	xml_tree **line;	/* at least one*/
	int n_line;

};

/*
<!ELEMENT line					(#PCDATA)>									<!-- Separator for each address line, initial letters are uppercase. -->
*/
struct line {
	char *pcdata;

};

/*
<!ELEMENT email				(#PCDATA)>									<!-- Email address of this author, all letters are lowercase. -->
*/
struct email {
	char *pcdata;

};

/*
<!ELEMENT url					(#PCDATA)>									<!-- Home page of this author. -->
*/
struct url {
	char *pcdata;

};

/*
<!ELEMENT description		(abstract,detail,msc2000,keywords,software)>		<!-- This element contains a subset of the same element in eg-model.dtd. -->
*/
struct description {
	xml_tree *abstract;	/* required */
	xml_tree *msc2000;	/* required */
	xml_tree *keywords;	/* required */
	xml_tree *software;	/* required */
	xml_tree *detail;	/* required */

};

/*
<!ELEMENT abstract			(#PCDATA)>									<!-- One sentence should cover main ideas of this model submission. -->
*/
struct abstract {
	char *pcdata;

};

/*
<!ELEMENT detail				(#PCDATA)>									<!-- Further detail information, for example, parameter settings used for generating the model. -->
*/
struct detail {
	char *pcdata;

};

/*
<!ELEMENT msc2000				(primary,secondary*)>					<!-- Mathematical Subject Classification 2000. -->
*/
struct msc2000 {
	xml_tree *primary;	/* required */
	xml_tree **secondary;	/* 0 or more */
	int n_secondary;

};

/*
<!ELEMENT primary				(#PCDATA)>									<!-- Main subject classification. -->
*/
struct primary {
	char *pcdata;

};

/*
<!ELEMENT secondary			(#PCDATA)>									<!-- Further subject classifications. -->
*/
struct secondary {
	char *pcdata;

};

/*
<!ELEMENT keywords			(keyword+)>									<!-- List of self-defined keywords characterizing this model. -->
*/
struct keywords {
	xml_tree **keyword;	/* at least one*/
	int n_keyword;

};

/*
<!ELEMENT keyword				(#PCDATA)>									<!-- Each keyword is self-defined, initial letters are uppercase. -->
*/
struct keyword {
	char *pcdata;

};

/*
<!ELEMENT software			(#PCDATA)>									<!-- Software including version number which generated this geometry model. -->
*/
struct software {
	char *pcdata;

};

/*
<!ELEMENT geometries			(geometry+)>								<!-- A set of different geometries. -->
*/
struct geometries {
	xml_tree **geometry;	/* at least one*/
	int n_geometry;

};

/*
<!ELEMENT geometry			(((pointSet,vectorField*)| 					 (pointSet,lineSet,vectorField*)| 					 (pointSet,faceSet,vectorField*)| 					 (primitive)), 					bndbox?,center?,labelAtt?,material?)>
*/
struct geometry {
	xml_tree *lineSet;	/* required */
	xml_tree *bndbox;	/* optional */
	xml_tree **vectorField;	/* 0 or more */
	int n_vectorField;
	xml_tree *primitive;	/* required */
	xml_tree *labelAtt;	/* optional */
	xml_tree *pointSet;	/* required */
	xml_tree *material;	/* optional */
	xml_tree *center;	/* optional */
	xml_tree *faceSet;	/* required */

	char *visible;	/* (show|hide) ("show") <!-- Show geometry in a scene, or hide it. --> */
	char *name;	/* (#IMPLIED) <!-- Name of geometry, should be unique. --> */
};

/*
<!ELEMENT pointSet			(points,colors?,normals?,textures?)>
*/
struct pointSet {
	xml_tree *points;	/* required */
	xml_tree *textures;	/* optional */
	xml_tree *colors;	/* optional */
	xml_tree *normals;	/* optional */

	char *normal;	/* (show|hide) ("hide") <!-- Show normal of each vertex. --> */
	char *point;	/* (show|hide) ("show") <!-- Show vertices as round circles. --> */
	char *dim;	/* (#REQUIRED) <!-- Number of components of each vertex, must be uniform for all vertices. --> */
	char *normalArrow;	/* (show|hide) ("hide") <!-- Show arrow of vertex normals. --> */
	char *color;	/* (show|hide) ("hide") <!-- Show individual colors of each vertex. --> */
};

/*
<!ELEMENT lineSet				(lines,colors?,normals?)>
*/
struct lineSet {
	xml_tree *colors;	/* optional */
	xml_tree *normals;	/* optional */
	xml_tree *lines;	/* required */

	char *normal;	/* (show|hide) ("hide") <!-- Show normal of each line. --> */
	char *line;	/* (show|hide) ("show") <!-- Show edges of line. --> */
	char *startArrow;	/* (show|hide) ("hide") <!-- Show arrow at first vertex of line. --> */
	char *arrow;	/* (show|hide) ("hide") <!-- Show arrow at last vertex of line. --> */
	char *normalArrow;	/* (show|hide) ("hide") <!-- Show arrow of line normals. --> */
	char *color;	/* (show|hide) ("hide") <!-- Show individual edge color of lines. --> */
};

/*
<!ELEMENT faceSet				(faces,neighbours?,edges?,colors?,colorsBack?,normals?,textures?,boundaries?)>
*/
struct faceSet {
	xml_tree *faces;	/* required */
	xml_tree *textures;	/* optional */
	xml_tree *edges;	/* optional */
	xml_tree *boundaries;	/* optional */
	xml_tree *colors;	/* optional */
	xml_tree *colorsBack;	/* optional */
	xml_tree *neighbours;	/* optional */
	xml_tree *normals;	/* optional */

	char *boundary;	/* (show|hide) ("hide") <!-- Show boundary of surface, determined by neighbour information. --> */
	char *normal;	/* (show|hide) ("hide") <!-- Show normal vectors at center of elements. --> */
	char *colorBackLocal;	/* (show|hide) ("hide") <!-- Enable and show individual colors of backface elements. --> */
	char *texture;	/* (show|hide) ("hide") <!-- Show texture on surface. --> */
	char *face;	/* (show|hide) ("show") <!-- Show filled elements, i.e. face, of a surface. --> */
	char *colorBackGlobal;	/* (show|hide) ("hide") <!-- Enable and show global backface color of backfacing elements. --> */
	char *normalArrow;	/* (show|hide) ("hide") <!-- Show arrow of element normal vectors. --> */
	char *backface;	/* (show|hide) ("show") <!-- Show backfacing elements, i.e. disable backface culling. --> */
	char *color;	/* (show|hide) ("hide") <!-- Show individual colors of elements, otherwise global color is used. --> */
	char *edge;	/* (show|hide) ("show") <!-- Show edge of elements as line. --> */
};

/*
<!ELEMENT vectorField		(vectors,colors?)>
*/
struct vectorField {
	xml_tree *vectors;	/* required */
	xml_tree *colors;	/* optional */

	char *arrow;	/* (show|hide) ("hide")  */
	char *base;	/* (vertex|element) ("vertex")  */
	char *material;	/* (show|hide) ("hide")  */
	char *name;	/* (#IMPLIED)  */
};

/*
<!ELEMENT points				(p*,thickness?,color?,colorTag?,labelAtt?)>
*/
struct points {
	xml_tree *thickness;	/* optional */
	xml_tree *colorTag;	/* optional */
	xml_tree **p;	/* 0 or more */
	int n_p;
	xml_tree *labelAtt;	/* optional */
	xml_tree *color;	/* optional */

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT p						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct p {
	char *pcdata;

	char *tag;	/* (#IMPLIED)  */
	char *name;	/* (#IMPLIED)  */
};

/*
<!ELEMENT colors				(c*)>											<!-- Individual colors of elements. -->
*/
struct colors {
	xml_tree **c;	/* 0 or more */
	int n_c;

	char *num;	/* (#IMPLIED)  */
	char *type;	/* (rgb|rgba|grey) ("rgb")  */
};

/*
<!ELEMENT c						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct c {
	char *pcdata;

};

/*
<!ELEMENT colorsBack			(c*)>											<!-- Individual colors of backfacing elements. -->
*/
struct colorsBack {
	xml_tree **c;	/* 0 or more */
	int n_c;

	char *num;	/* (#IMPLIED)  */
	char *type;	/* (rgb|rgba|grey) ("rgb")  */
};

/*
<!ELEMENT normals				(n*,thickness?,length?,color?)>
*/
struct normals {
	xml_tree **n;	/* 0 or more */
	int n_n;
	xml_tree *thickness;	/* optional */
	xml_tree *length;	/* optional */
	xml_tree *color;	/* optional */

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT n						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct n {
	char *pcdata;

};

/*
<!ELEMENT lines				(l*,thickness?,color?,colorTag?,labelAtt?)>
*/
struct lines {
	xml_tree *thickness;	/* optional */
	xml_tree *colorTag;	/* optional */
	xml_tree *labelAtt;	/* optional */
	xml_tree **l;	/* 0 or more */
	int n_l;
	xml_tree *color;	/* optional */

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT l						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct l {
	char *pcdata;

	char *arrow;	/* (show|hide) ("hide") <!-- Not implemented yet --> */
	char *tag;	/* (#IMPLIED)  */
	char *name;	/* (#IMPLIED)  */
};

/*
<!ELEMENT faces				(f*,color?,colorBack?,colorTag?,labelAtt?)>
*/
struct faces {
	xml_tree **f;	/* 0 or more */
	int n_f;
	xml_tree *colorTag;	/* optional */
	xml_tree *colorBack;	/* optional */
	xml_tree *labelAtt;	/* optional */
	xml_tree *color;	/* optional */

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT f						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct f {
	char *pcdata;

	char *tag;	/* (#IMPLIED)  */
	char *name;	/* (#IMPLIED)  */
};

/*
<!ELEMENT edges				(e*,thickness?,color?,colorTag?,labelAtt?)>	<!-- Often not explicitly specified -->
*/
struct edges {
	xml_tree **e;	/* 0 or more */
	int n_e;
	xml_tree *thickness;	/* optional */
	xml_tree *colorTag;	/* optional */
	xml_tree *labelAtt;	/* optional */
	xml_tree *color;	/* optional */

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT e						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct e {
	char *pcdata;

	char *tag;	/* (#IMPLIED)  */
	char *name;	/* (#IMPLIED)  */
};

/*
<!ELEMENT neighbours			(nb*)>
*/
struct neighbours {
	xml_tree **nb;	/* 0 or more */
	int n_nb;

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT nb					(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct nb {
	char *pcdata;

};

/*
<!ELEMENT vectors				(v*,thickness?,length?,color?)>
*/
struct vectors {
	xml_tree **v;	/* 0 or more */
	int n_v;
	xml_tree *thickness;	/* optional */
	xml_tree *length;	/* optional */
	xml_tree *color;	/* optional */

	char *num;	/* (#IMPLIED)  */
};

/*
<!ELEMENT v						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct v {
	char *pcdata;

};

/*
<!ELEMENT textures			(t*,image?)>
*/
struct textures {
	xml_tree **t;	/* 0 or more */
	int n_t;
	xml_tree *image;	/* optional */

	char *num;	/* (#IMPLIED)  */
	char *dim;	/* (#REQUIRED)  */
	char *type;	/* (image) ("image")  */
};

/*
<!ELEMENT t						(#PCDATA)>									<!-- Components separated by blanks -->
*/
struct t {
	char *pcdata;

};

/*
<!ELEMENT boundaries			(thickness?,color?,colorTag?,labelAtt?)>	<!-- Child elements are still in development -->
*/
struct boundaries {
	xml_tree *thickness;	/* optional */
	xml_tree *colorTag;	/* optional */
	xml_tree *labelAtt;	/* optional */
	xml_tree *color;	/* optional */

};

/*
<!ELEMENT color				(#PCDATA)>
*/
struct color {
	char *pcdata;

	char *type;	/* (rgb|rgba|grey) ("rgb")  */
};

/*
<!ELEMENT colorBack			(#PCDATA)>
*/
struct colorBack {
	char *pcdata;

	char *type;	/* (rgb|rgba|grey) ("rgb")  */
};

/*
<!ELEMENT colorTag			(#PCDATA)>
*/
struct colorTag {
	char *pcdata;

	char *type;	/* (rgb|rgba|grey) ("rgb")  */
};

/*
<!ELEMENT primitive			(cube|sphere|cylinder|cone)>
*/
struct primitive {
	xml_tree *cube;	/* required */
	xml_tree *sphere;	/* required */
	xml_tree *cylinder;	/* required */
	xml_tree *cone;	/* required */

};

/*
<!ELEMENT cube					(lowerLeft,upperRight)>
*/
struct cube {
	xml_tree *lowerLeft;	/* required */
	xml_tree *upperRight;	/* required */

};

/*
<!ELEMENT lowerLeft			(p)>											<!-- Lower left vertex of hyper cube -->
*/
struct lowerLeft {
	xml_tree *p;	/* required */

};

/*
<!ELEMENT upperRight			(p)>											<!-- Upper right vertex of hyper cube -->
*/
struct upperRight {
	xml_tree *p;	/* required */

};

/*
<!ELEMENT sphere				(midpoint,radius)>
*/
struct sphere {
	xml_tree *midpoint;	/* required */
	xml_tree *radius;	/* required */

};

/*
<!ELEMENT midpoint			(p)>											<!-- Center of n-dimensional sphere -->
*/
struct midpoint {
	xml_tree *p;	/* required */

};

/*
<!ELEMENT cylinder			(bottom,top,radius*)>
*/
struct cylinder {
	xml_tree *bottom;	/* required */
	xml_tree *top;	/* required */
	xml_tree **radius;	/* 0 or more */
	int n_radius;

};

/*
<!ELEMENT bottom				(p)>											<!-- Base center of n-dimensional cylinder -->
*/
struct bottom {
	xml_tree *p;	/* required */

};

/*
<!ELEMENT top					(p)>											<!-- Top center of n-dimensional cylinder -->
*/
struct top {
	xml_tree *p;	/* required */

};

/*
<!ELEMENT cone					(bottom,top,radius)>
*/
struct cone {
	xml_tree *bottom;	/* required */
	xml_tree *top;	/* required */
	xml_tree *radius;	/* required */

};

/*
<!ELEMENT radius				(#PCDATA)>									<!-- Double value, radius of circles and spheres -->
*/
struct radius {
	char *pcdata;

};

/*
<!ELEMENT thickness			(#PCDATA)>									<!-- Double value, thickness of lines and points -->
*/
struct thickness {
	char *pcdata;

};

/*
<!ELEMENT length				(#PCDATA)>									<!-- Double value, length of lines -->
*/
struct length {
	char *pcdata;

};

/*
<!ELEMENT bndbox				(p,p)>
*/
struct bndbox {
	xml_tree *p;	/* required */

	char *visible;	/* (show|hide) ("show")  */
};

/*
<!ELEMENT center				(p)>
*/
struct center {
	xml_tree *p;	/* required */

	char *visible;	/* (show|hide) ("show")  */
};

/*
<!ELEMENT labelAtt			(xOffset?,yOffset?)>
*/
struct labelAtt {
	xml_tree *yOffset;	/* optional */
	xml_tree *xOffset;	/* optional */

	char *font;	/* (text|fixed|header2|header4|menu) ("text")  */
	char *horAlign;	/* (head|center|tail) ("head")  */
	char *verAlign;	/* (top|middle|bottom) ("top")  */
	char *visible;	/* (show|hide) ("show")  */
};

/*
<!ELEMENT xOffset				(#PCDATA)>
*/
struct xOffset {
	char *pcdata;

};

/*
<!ELEMENT yOffset				(#PCDATA)>
*/
struct yOffset {
	char *pcdata;

};

/*
<!ELEMENT material			(ambientIntensity,diffuse,emissive,shininess,specular,transparency)>
*/
struct material {
	xml_tree *ambientIntensity;	/* required */
	xml_tree *diffuse;	/* required */
	xml_tree *shininess;	/* required */
	xml_tree *emissive;	/* required */
	xml_tree *specular;	/* required */
	xml_tree *transparency;	/* required */

};

/*
<!ELEMENT ambientIntensity	(#PCDATA)>									<!-- Double value in [0.,1.] -->
*/
struct ambientIntensity {
	char *pcdata;

};

/*
<!ELEMENT diffuse				(color)>
*/
struct diffuse {
	xml_tree *color;	/* required */

};

/*
<!ELEMENT emissive			(color)>
*/
struct emissive {
	xml_tree *color;	/* required */

};

/*
<!ELEMENT shininess			(#PCDATA)>									<!-- Double value in [0.,1.] -->
*/
struct shininess {
	char *pcdata;

};

/*
<!ELEMENT specular			(#PCDATA)>									<!-- Double value in [0.,1.] -->
*/
struct specular {
	char *pcdata;

};

/*
<!ELEMENT transparency		(#PCDATA)>									<!-- Double value in [0.,1.] -->
*/
struct transparency {
	char *pcdata;

};

/*
<!ELEMENT image				(url)>
*/
struct image {
	xml_tree *url;	/* required */

	char *repeat;	/* (no|s|t|st) ("no")  */
};

#endif
