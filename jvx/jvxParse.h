/*
<!ELEMENT jvxmodel			(meta*,version?,title,authors?,description?,geometries)>
*/
typedef struct jvx_model		
{
	struct meta **meta;
	struct version *version; /* optional */
	struct title *title;	 /* required */
	struct authors *authors;
	struct description *description; /* optional */
	struct geometries *geometries;	/* required */
} jvx_model;

/*
<!ELEMENT meta					EMPTY>
<!ATTLIST meta					generator CDATA #IMPLIED>				<!-- Name and version of program which generated this model file. -->
<!ATTLIST meta					date CDATA #IMPLIED>						<!-- Date when this model file was generated. -->
*/
typedef struct meta
{
	char *generator;	/* required */
	char *data;		/* required */
} meta;

/*
<!ELEMENT version				(#PCDATA)>									
<!-- Version of this model file. -->
<!ATTLIST version type (dump|beta|final) "dump">
<!-- Status of this model file. -->
*/
typedef struct version				
{
	char *PCDATA;
	char *type;
}
version;

/*
<!ELEMENT authors				(author+)>									
<!-- List of authors who created, computed, designed the models. -->
*/
typedef struct authors			
{
	char **author;	/* at least one */
} authors			;

/*
<!ELEMENT author				(firstname,lastname,affiliation,email,url?)>
<!-- Contact address of this author. -->
*/
typedef struct author			
{
	struct firstname *firstname;
	struct lastname *lastname;
	struct affiliation *affiliation;
	struct email *email;
	struct url, *url;
} author;

/*
<!ELEMENT firstname			(#PCDATA)>									
<!-- First name, middle name, and title of this author, initial letters are uppercase. -->
*/
typedef struct firstname			
{
	char *PCDATA;
} firstname			;

/*
<!ELEMENT lastname			(#PCDATA)>									
<!-- Last name of this author, initial letters are uppercase. -->
*/
typedef struct lastname			
{
	char *PCDATA;
} lastname			;

/*
<!ELEMENT affiliation		(organization,address)>
*/
typedef struct affiliation	
{
	struct organization *organization;
	struct address *address;
} affiliation	;

/*
<!ELEMENT organization		(#PCDATA)>									
<!-- Empty or university or company where author works, initial letters are uppercase. -->
*/
typedef struct organization		
{
	char *PCDATA;
} organization;

/*
<!ELEMENT address				(line+)>										<!-- Postal address for surface mail, initial letters are uppercase. -->
<!-- Postal address for surface mail, initial letters are uppercase. -->
*/
typedef struct address			
{
	struct line **line+;
} address;

/*
<!ELEMENT line					(#PCDATA)>									
<!-- Separator for each address line, initial letters are uppercase. -->
*/
typedef struct line					
{
	char *PCDATA;
} line					;

/*
<!ELEMENT email				(#PCDATA)>									
<!-- Email address of this author, all letters are lowercase. -->
*/
typedef struct email				
{
	char *PCDATA;
} email				;

/*
<!ELEMENT url					(#PCDATA)>									
<!-- Home page of this author. -->
*/
typedef struct url					
{
	char *PCDATA;
} url					;



/*
<!ELEMENT description		(abstract,detail,msc2000,keywords,software)>
<!-- This element contains a subset of the same element in eg-model.dtd. -->
*/
typedef struct description	
{
	struct abstract *abstract;
	struct detail *detail;
	struct msc2000 *msc2000;
	struct keywords *keywords;
	struct software *software;
} description	;

																					<!-- This element remain fully valid when being copied into an eg-model description. -->
/*
<!ELEMENT abstract			(#PCDATA)>									
<!-- One sentence should cover main ideas of this model submission. -->
*/
typedef struct abstract			
{
	char *PCDATA;
} abstract			;

/*
<!ELEMENT detail				(#PCDATA)>									
<!-- Further detail information, for example, parameter settings used for generating the model. -->
*/
typedef struct detail				
{
	char *PCDATA;
} detail				;

																					<!-- This element contains a subset of the same element in eg-model.dtd to avoid overlap with 'p' element. -->
/*
<!ELEMENT msc2000				(primary,secondary*)>					
<!-- Mathematical Subject Classification 2000. -->
*/
typedef struct msc2000			
{
	struct primary *primary;
	struct secondary *secondary;
} msc2000			;

/*
<!ELEMENT primary				(#PCDATA)>									
<!-- Main subject classification. -->
*/
typedef struct primary				
{
	char *PCDATA;
} primary				;

/*
<!ELEMENT secondary			(#PCDATA)>									
<!-- Further subject classifications. -->
*/
typedef struct secondary			
{
	char *PCDATA;
} secondary			;

/*
<!ELEMENT keywords			(keyword+)>									
<!-- List of self-defined keywords characterizing this model. -->
*/
typedef struct keywords		
{
	struct keyword *keyword;
} keywords		;

/*
<!ELEMENT keyword				(#PCDATA)>									
<!-- Each keyword is self-defined, initial letters are uppercase. -->
*/
typedef struct keyword				
{
	char *PCDATA;
} keyword				;

/*
<!ELEMENT software			(#PCDATA)>									
<!-- Software including version number which generated this geometry model. -->
*/
typedef struct software			
{
	char *PCDATA;
} software			;



/*
<!ELEMENT geometries			(geometry+)>								
<!-- A set of different geometries. -->
*/
typedef struct geometries		
{
	struct geometry *geometry;
} geometries		;


/*
<!ELEMENT geometry			(((pointSet,vectorField*)|
		  (pointSet,lineSet,vectorField*)|
		  (pointSet,faceSet,vectorField*)|
		  (primitive)),
		 bndbox?,center?,labelAtt?,material?)>
*/
typedef struct geometry			(
{
	struct pointSet *pointSet;
	struct vectorField *vectorField;
	struct lineSet *lineSet;
	struct faceSet *faceSet;
	struct primitive *primitive;
	struct bndbox? *bndbox?;
	struct center? *center?;
	struct labelAtt? *labelAtt?;
	struct material *material;

<!ATTLIST geometry			name CDATA #IMPLIED>						<!-- Name of geometry, should be unique. -->
<!ATTLIST geometry			visible (show|hide) "show">			<!-- Show geometry in a scene, or hide it. -->
} geometry			(;






/*
<!ELEMENT pointSet			(points,colors?,normals?,textures?)>
<!ATTLIST pointSet			dim	CDATA #REQUIRED>					<!-- Number of components of each vertex, must be uniform for all vertices. -->
<!ATTLIST pointSet			point (show|hide) "show">				<!-- Show vertices as round circles. -->
<!ATTLIST pointSet			color (show|hide) "hide">				<!-- Show individual colors of each vertex. -->
<!ATTLIST pointSet			normal (show|hide) "hide">				<!-- Show normal of each vertex. -->
<!ATTLIST pointSet			normalArrow (show|hide) "hide">		<!-- Show arrow of vertex normals. -->
*/
typedef struct pointSet		
{
	struct points *points;
	struct colors? *colors?;
	struct normals? *normals?;
	struct textures? *textures?;
<!ATTLIST pointSet			dim	CDATA #REQUIRED>					<!-- Number of components of each vertex, must be uniform for all vertices. -->
<!ATTLIST pointSet			point (show|hide) "show">				<!-- Show vertices as round circles. -->
<!ATTLIST pointSet			color (show|hide) "hide">				<!-- Show individual colors of each vertex. -->
<!ATTLIST pointSet			normal (show|hide) "hide">				<!-- Show normal of each vertex. -->
<!ATTLIST pointSet			normalArrow (show|hide) "hide">		<!-- Show arrow of vertex normals. -->
} pointSet		;



/*
<!ELEMENT lineSet				(lines,colors?,normals?)>
<!ATTLIST lineSet				arrow (show|hide) "hide">				<!-- Show arrow at last vertex of line. -->
<!ATTLIST lineSet				startArrow (show|hide) "hide">		<!-- Show arrow at first vertex of line. -->
<!ATTLIST lineSet				line (show|hide) "show">				<!-- Show edges of line. -->
<!ATTLIST lineSet				color (show|hide) "hide">				<!-- Show individual edge color of lines. -->
<!ATTLIST lineSet				normal (show|hide) "hide">				<!-- Show normal of each line. -->
<!ATTLIST lineSet				normalArrow (show|hide) "hide">		<!-- Show arrow of line normals. -->
*/
typedef struct lineSet			
{
	struct lines *lines;
	struct colors? *colors?;
	struct normals *normals;
<!ATTLIST lineSet				arrow (show|hide) "hide">				<!-- Show arrow at last vertex of line. -->
<!ATTLIST lineSet				startArrow (show|hide) "hide">		<!-- Show arrow at first vertex of line. -->
<!ATTLIST lineSet				line (show|hide) "show">				<!-- Show edges of line. -->
<!ATTLIST lineSet				color (show|hide) "hide">				<!-- Show individual edge color of lines. -->
<!ATTLIST lineSet				normal (show|hide) "hide">				<!-- Show normal of each line. -->
<!ATTLIST lineSet				normalArrow (show|hide) "hide">		<!-- Show arrow of line normals. -->
} lineSet			;



/*
<!ELEMENT faceSet				(faces,neighbours?,edges?,colors?,colorsBack?,normals?,textures?,boundaries?)>
<!ATTLIST faceSet				face	(show|hide) "show">				<!-- Show filled elements, i.e. face, of a surface. -->
<!ATTLIST faceSet				edge	(show|hide) "show">				<!-- Show edge of elements as line. -->
<!ATTLIST faceSet				color (show|hide) "hide">				<!-- Show individual colors of elements, otherwise global color is used. -->
<!ATTLIST faceSet				colorBackGlobal (show|hide) "hide">	<!-- Enable and show global backface color of backfacing elements. -->
<!ATTLIST faceSet				colorBackLocal (show|hide) "hide">	<!-- Enable and show individual colors of backface elements. -->
<!ATTLIST faceSet				normal (show|hide) "hide">				<!-- Show normal vectors at center of elements. -->
<!ATTLIST faceSet				normalArrow (show|hide) "hide">		<!-- Show arrow of element normal vectors. -->
<!ATTLIST faceSet				texture (show|hide) "hide">			<!-- Show texture on surface. -->
<!ATTLIST faceSet				backface (show|hide) "show">			<!-- Show backfacing elements, i.e. disable backface culling. -->
<!ATTLIST faceSet				boundary (show|hide) "hide"> 			<!-- Show boundary of surface, determined by neighbour information. -->
*/
typedef struct faceSet			
{
	struct faces *faces;
	struct neighbours? *neighbours?;
	struct edges? *edges?;
	struct colors? *colors?;
	struct colorsBack? *colorsBack?;
	struct normals? *normals?;
	struct textures? *textures?;
	struct boundaries? *boundaries?;
<!ATTLIST faceSet				face	(show|hide) "show">				<!-- Show filled elements, i.e. face, of a surface. -->
<!ATTLIST faceSet				edge	(show|hide) "show">				<!-- Show edge of elements as line. -->
<!ATTLIST faceSet				color (show|hide) "hide">				<!-- Show individual colors of elements, otherwise global color is used. -->
<!ATTLIST faceSet				colorBackGlobal (show|hide) "hide">	<!-- Enable and show global backface color of backfacing elements. -->
<!ATTLIST faceSet				colorBackLocal (show|hide) "hide">	<!-- Enable and show individual colors of backface elements. -->
<!ATTLIST faceSet				normal (show|hide) "hide">				<!-- Show normal vectors at center of elements. -->
<!ATTLIST faceSet				normalArrow (show|hide) "hide">		<!-- Show arrow of element normal vectors. -->
<!ATTLIST faceSet				texture (show|hide) "hide">			<!-- Show texture on surface. -->
<!ATTLIST faceSet				backface (show|hide) "show">			<!-- Show backfacing elements, i.e. disable backface culling. -->
<!ATTLIST faceSet				boundary (show|hide) "hide"> 			<!-- Show boundary of surface, determined by neighbour information. -->
} faceSet			;



/*
<!ELEMENT vectorField		(vectors,colors?)>
<!ATTLIST vectorField		name 	CDATA #IMPLIED>
<!ATTLIST vectorField		arrow (show|hide) "hide">
<!ATTLIST vectorField		base 	(vertex|element) "vertex">
*/
typedef struct vectorField	
{
	vectors,
	colors?,
<!ATTLIST vectorField		name 	CDATA #IMPLIED>
<!ATTLIST vectorField		arrow (show|hide) "hide">
<!ATTLIST vectorField		base 	(vertex|element) "vertex">
} vectorField	;





/*
<!ELEMENT points				(p*,thickness?,color?,colorTag?,labelAtt?)>
<!ATTLIST points				num	CDATA #IMPLIED>
*/
typedef struct points			
{
	struct p* *p*;
	struct thickness? *thickness?;
	struct color? *color?;
	struct colorTag? *colorTag?;
	struct labelAtt? *labelAtt?;
<!ATTLIST points				num	CDATA #IMPLIED>
} points			;



/*
<!ELEMENT p						(#PCDATA)>									<!-- Components separated by blanks -->
<!ATTLIST p						name	CDATA #IMPLIED>
<!ATTLIST p						tag	CDATA #IMPLIED>
*/
typedef struct p						
{
	char *PCDATA;
<!ATTLIST p						name	CDATA #IMPLIED>
<!ATTLIST p						tag	CDATA #IMPLIED>
} p						;



/*
<!ELEMENT colors				(c*)>											
<!-- Individual colors of elements. -->
<!ATTLIST colors				type	(rgb|rgba|grey) "rgb">
<!ATTLIST colors				num	CDATA #IMPLIED>
*/
typedef struct colors			
{
	struct c* *c*;
<!ATTLIST colors				type	(rgb|rgba|grey) "rgb">
<!ATTLIST colors				num	CDATA #IMPLIED>
} colors			;



/*
<!ELEMENT c						(#PCDATA)>									
<!-- Components separated by blanks -->
*/
typedef struct c						
{
	char *PCDATA;
} c						;


/*
<!ELEMENT colorsBack			(c*)>
<!-- Individual colors of backfacing elements. -->
<!ATTLIST colorsBack			type	(rgb|rgba|grey) "rgb">
<!ATTLIST colorsBack			num	CDATA #IMPLIED>
*/
typedef struct colorsBack		
{
	struct c* *c*;
<!ATTLIST colorsBack			type	(rgb|rgba|grey) "rgb">
<!ATTLIST colorsBack			num	CDATA #IMPLIED>
} colorsBack		;



/*
<!ELEMENT normals				(n*,thickness?,length?,color?)>
<!ATTLIST lines				num	CDATA #IMPLIED>
*/
typedef struct normals			
{
	n*,
	thickness?,
	length?,
	color?)
<!ATTLIST lines				num	CDATA #IMPLIED>
} normals			;



/*
<!ELEMENT n						(#PCDATA)>									
<!-- Components separated by blanks -->
*/
typedef struct n						
{
	char *PCDATA;
} n						;


/*
<!ELEMENT lines				(l*,thickness?,color?,colorTag?,labelAtt?)>
<!ATTLIST lines				num	CDATA #IMPLIED>
*/
typedef struct lines			
{
	struct l* *l*;
	struct thickness? *thickness?;
	struct color? *color?;
	struct colorTag? *colorTag?;
	struct labelAtt? *labelAtt?;
} lines			;



/*
<!ELEMENT l						(#PCDATA)>									
<!-- Components separated by blanks -->
<!ATTLIST l						name	CDATA #IMPLIED>
<!ATTLIST l						tag	CDATA #IMPLIED>
<!ATTLIST l						arrow (show|hide) "hide">				<!-- Not implemented yet -->
*/
typedef struct l						
{
	char *PCDATA;
} l						;



/*
<!ELEMENT faces				(f*,color?,colorBack?,colorTag?,labelAtt?)>
<!ATTLIST faces				num	CDATA #IMPLIED>
*/
typedef struct faces			
{
	struct f* *f*;
	struct color? *color?;
	struct colorBack? *colorBack?;
	struct colorTag? *colorTag?;
	struct labelAtt? *labelAtt?;
<!ATTLIST faces				num	CDATA #IMPLIED>
} faces			;



/*
<!ELEMENT f						(#PCDATA)>									
<!-- Components separated by blanks -->
<!ATTLIST f						name	CDATA #IMPLIED>
<!ATTLIST f						tag	CDATA #IMPLIED>
*/
typedef struct f						
{
	char *PCDATA;
<!ATTLIST f						name	CDATA #IMPLIED>
<!ATTLIST f						tag	CDATA #IMPLIED>
} f						;



/*
<!ELEMENT edges				(e*,thickness?,color?,colorTag?,labelAtt?)>	
<!-- Often not explicitly specified -->
<!ATTLIST edges				num	CDATA #IMPLIED>
*/
typedef struct edges			
{
	struct e* *e*;
	struct thickness? *thickness?;
	struct color? *color?;
	struct colorTag? *colorTag?;
	struct labelAtt? *labelAtt?;
<!ATTLIST edges				num	CDATA #IMPLIED>
} edges			;



/*
<!ELEMENT e						(#PCDATA)>									
<!-- Components separated by blanks -->
<!ATTLIST e						name	CDATA #IMPLIED>
<!ATTLIST e						tag	CDATA #IMPLIED>
*/
typedef struct e						
{
	char *PCDATA;
<!ATTLIST e						name	CDATA #IMPLIED>
<!ATTLIST e						tag	CDATA #IMPLIED>
} e						;



/*
<!ELEMENT neighbours			(nb*)>
<!ATTLIST neighbours			num	CDATA #IMPLIED>
*/
typedef struct neighbours		
{
	struct nb **nb;
<!ATTLIST neighbours			num	CDATA #IMPLIED>
} neighbours		;



/*
<!ELEMENT nb					(#PCDATA)>									
<!-- Components separated by blanks -->
*/
typedef struct nb					
{
	char *PCDATA;
} nb					;


/*
<!ELEMENT vectors				(v*,thickness?,length?,color?)>
*/
typedef struct vectors			
{
	struct v* *v*;
	struct thickness? *thickness?;
	struct length? *length?;
	struct color? *color?;
} vectors			;


<!ATTLIST vectors				num	CDATA #IMPLIED>

/*
<!ELEMENT v						(#PCDATA)>									
<!-- Components separated by blanks -->
*/
typedef struct v						
{
	char *PCDATA;
} v						;


/*
<!ELEMENT textures			(t*,image?)>
<!ATTLIST textures			dim	CDATA #REQUIRED>
<!ATTLIST textures			num	CDATA #IMPLIED>
<!ATTLIST textures			type	(image) "image">
*/
typedef struct textures		
{
	t*,
	image,
<!ATTLIST textures			dim	CDATA #REQUIRED>
<!ATTLIST textures			num	CDATA #IMPLIED>
<!ATTLIST textures			type	(image) "image">
} textures		;



/*
<!ELEMENT t						(#PCDATA)>									
<!-- Components separated by blanks -->
*/
typedef struct t						
{
	char *PCDATA;
} t						;


/*
<!ELEMENT boundaries			(thickness?,color?,colorTag?,labelAtt?)>	
<!-- Child elements are still in development -->
*/
typedef struct boundaries		
{
	struct thickness? *thickness?;
	struct color? *color?;
	struct colorTag? *colorTag?;
	struct labelAtt? *labelAtt?;
} boundaries		;


/*
<!ELEMENT color				(#PCDATA)>
*/
typedef struct color				
{
	char *PCDATA;
<!ATTLIST color				type	(rgb|rgba|grey) "rgb">
} color				;



/*
<!ELEMENT colorBack			(#PCDATA)>
*/
typedef struct colorBack			
{
	char *PCDATA;
<!ATTLIST colorBack			type	(rgb|rgba|grey) "rgb">
} colorBack			;



/*
<!ELEMENT colorTag			(#PCDATA)>
*/
typedef struct colorTag			
{
	char *PCDATA;
<!ATTLIST colorTag			type	(rgb|rgba|grey) "rgb">
} colorTag			;





/*
<!ELEMENT primitive			(cube|sphere|cylinder|cone)>
*/
typedef struct primitive		
{
	struct cube *cube;
	struct sphere *sphere;
	struct cylinder *cylinder;
	struct cone *cone;
} primitive		;




/*
<!ELEMENT cube					(lowerLeft,upperRight)>
*/
typedef struct cube				
{
	struct lowerLeft *lowerLeft;
	struct upperRight *upperRight;
} cube				;


/*
<struct !ELEMENT lowerLeft			(p)>											
<!-- Lower left vertex of hyper cube -->
*/
typedef struct lowerLeft		
{
	struct p *p;
} lowerLeft		;


/*
<!ELEMENT upperRight			(p)>
<!-- Upper right vertex of hyper cube -->
*/
typedef struct upperRight		
{
	struct p *p;
} upperRight		;




/*
<!ELEMENT sphere				(midpoint,radius)>
*/
typedef struct sphere			
{
	struct midpoint *midpoint;
	struct radius *radius;
} sphere			;


/*
<!ELEMENT midpoint			(p)>
<!-- Center of n-dimensional sphere -->
*/
typedef struct midpoint		
{
	struct p *p;
} midpoint		;




/*
<!ELEMENT cylinder			(bottom,top,radius*)>
*/
typedef struct cylinder		
{
	struct bottom *bottom;
	struct top *top;
	struct radius* *radius*;
} cylinder		;


/*
<!ELEMENT bottom				(p)>
<!-- Base center of n-dimensional cylinder -->
*/
typedef struct bottom			
{
	struct p *p;
} bottom			;


/*
<!ELEMENT top					(p)>
<!-- Top center of n-dimensional cylinder -->
*/
typedef struct top				
{
	struct p *p;
} top				;




/*
<!ELEMENT cone					(bottom,top,radius)>
*/
typedef struct cone				
{
	struct bottom *bottom;
	struct top *top;
	struct radius *radius;
} cone				;




/*
<!ELEMENT radius				(#PCDATA)>									
<!-- Double value, radius of circles and spheres -->
*/
typedef struct radius				
{
	char *PCDATA;
} radius				;


/*
<!ELEMENT thickness			(#PCDATA)>									<!-- Double value, thickness of lines and points -->
*/
typedef struct thickness			
{
	char *PCDATA;
} thickness			;


/*
<!ELEMENT length				(#PCDATA)>									
<!-- Double value, length of lines -->
*/
typedef struct length				
{
	char *PCDATA;
} length				;




/*
<!ELEMENT bndbox				(p,p)>
<!ATTLIST bndbox				visible (show|hide) "show">
*/
typedef struct bndbox			
{
	struct p *p;
	struct p *p;
<!ATTLIST bndbox				visible (show|hide) "show">
} bndbox			;





/*
<!ELEMENT center				(p)>
<!ATTLIST center				visible (show|hide) "show">
*/
typedef struct center			
{
	struct p *p;
} center			;

/*
<!ELEMENT labelAtt			(xOffset?,yOffset?)>
<!ATTLIST labelAtt			visible (show|hide) "show">
<!ATTLIST labelAtt			horAlign (head|center|tail) "head">
<!ATTLIST labelAtt			verAlign (top|middle|bottom) "top">
<!ATTLIST labelAtt			font (text|fixed|header2|header4|menu) "text">
*/
typedef struct labelAtt		
{
	struct xOffset? *xOffset?;
	struct yOffset? *yOffset?;
<!ATTLIST labelAtt			visible (show|hide) "show">
<!ATTLIST labelAtt			horAlign (head|center|tail) "head">
<!ATTLIST labelAtt			verAlign (top|middle|bottom) "top">
<!ATTLIST labelAtt			font (text|fixed|header2|header4|menu) "text">
} labelAtt		;



/*
<!ELEMENT xOffset				(#PCDATA)>
*/
typedef struct xOffset				
{
(
	char *PCDATA;
)>
} xOffset				;


/*
<!ELEMENT yOffset				(#PCDATA)>
*/
typedef struct yOffset				
{
	char *PCDATA;
} yOffset				;




/*
<!ELEMENT material			(ambientIntensity,diffuse,emissive,
					shininess,specular,transparency)>
*/
typedef struct material		
{
	struct ambientIntensity *ambientIntensity;
	struct diffuse *diffuse;
	struct emissive *emissive;
	struct shininess *shininess;
	struct specular *specular;
	struct transparency *transparency;
} material		;



/*
<!ELEMENT ambientIntensity	(#PCDATA)>									
<!-- Double value in [0.,1.] -->
*/
typedef struct ambientIntensity	
{
	char *PCDATA;
} ambientIntensity	;


/*
<!ELEMENT diffuse				(color)>
*/
typedef struct diffuse			
{
	struct color *color;
} diffuse			;


/*
<!ELEMENT emissive			(color)>
*/
typedef struct emissive		
{
	struct color *color;
} emissive		;


/*
<!ELEMENT shininess			(#PCDATA)>									
<!-- Double value in [0.,1.] -->
*/
typedef struct shininess			
{
	char *PCDATA;
} shininess			;


/*
<!ELEMENT specular			(#PCDATA)>			
<!-- Double value in [0.,1.] -->
*/
typedef struct specular			
{
	char *PCDATA;
} specular;


/*
<!ELEMENT transparency		(#PCDATA)>
<!-- Double value in [0.,1.] -->
*/
typedef struct transparency		
{
	char *PCDATA;
} transparency;




/*
<!ELEMENT image				(url)>
<!ATTLIST image				repeat (no|s|t|st) "no">
*/
typedef struct image			
{
	struct url *url;
<!ATTLIST image				repeat (no|s|t|st) "no">
} image;



