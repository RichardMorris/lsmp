#! /bin/perl -w
# Generates a C Header file for parsing jvx files using the dtd
#
# before parsing make sure 
# perl genParser.pl jvx.dtd

$dtd = $ARGV[0];
$header = $dtd;
$parser = $dtd;
$basename = $dtd;
$header =~ s/\..*/.h/;
$parser =~ s/\..*/Checker.c/;
$basename =~ s/\..*//;
$errno = 1000;
$debug = 0;

print "Reading $dtd generating hash structures\n";
open(IN,$dtd) or die "Nay open $dtd;";
open(HEADER,">$header") or die "Nay open $header;";
open(PARSER,">$parser") or die "Nay open $parser;";

print HEADER  <<EOF;
/*
 * Automatically by org.pfaf/lsmp/jvx/genParser.pl generated from:
 *
EOF
print PARSER "#include \"$header\"\n";
print PARSER  <<EOF;
/*
 * Automatically by org.pfaf/lsmp/jvx/genParser.pl generated from:
 *
EOF

# %elements 
# { jvx_model => 
#   children => 
#	{ child_name => type}
#   pcdata => PCDATA
#   attributes =>
#       { att_name => { spec => $spec , default => $def }
#   comment => 

%elements = ();
@elements = ();
$ele_count = 0;
while(<IN>)
{
	chomp;

	if(m/<!ELEMENT/)
	{
		$matching_elements = 1;	
		$ele_line = $_;
	} 
	elsif( $matching_elements )
	{
		$ele_line .= " " . $_;
	}
		
	
	if( $matching_elements && $ele_line =~ m/<!ELEMENT\s+([^\s]*)\s+([^>]*)>\s*(.*)/)
	{
		$name = $1; $specs = $2; $rest = $3;
		$matching_elements = 0;
		$origname = $name;
		$name =~ s/-/_/g;
		$specs =~ s/\s//g;
		$specs =~ s/[\(\)>]//g;
		$specs =~ s/\|/,/g;
		$rest =~ s/^\s*//;
		%specs = ();
		push @elements, $name;
		foreach $spec ( split(/,/,$specs) )
		{
			$spec =~ m/(\w*)(.*)/;
			$specs{$1} = $2;
		}
		foreach $key (keys %specs)
		{
			$kids{$name}{$key} = $specs{$key};
		}
		$origname{$name} = $origname;
		$comments{$name} = $rest;
		$lines{$name} =  $ele_line;
		$ele_line = "";
	}
	if(m/<!ATTLIST\s+([^\s]*)\s+([^\s]*)\s+([^\s]*)\s+([^>]*)>\s*(.*)/)
	{
		$name = $1;
		$att = $2;
		$spec = $3;
		$default = $4;
		$rest = $5;

		$atts{$name}{$att} = { spec => $spec, def => $default, comment => $rest };
	}
	if(m/<!ENTITY\s+([^\s]*)\s+([^>]*)/)
	{
		$name = $1;
		$details = $2;
		if( $name !~ m/history/ )
		{
			print HEADER  "$name\t$details\n";
			print PARSER  "$name\t$details\n";
		}
	}
}

#/********** DTD parsed now print output ************************/

print HEADER "*/\n\n";
print PARSER "*/\n\n";

print HEADER <<EOF;

#ifndef LSMP_XML_PARSER_H
#define LSMP_XML_PARSER_H
#include <stdio.h>
#include <string.h>

EOF

print HEADER "union node_specific_info {\n";
foreach $name (@elements)
{
	print HEADER "struct $name *$name;\n";
}
print HEADER "} LSMP\_$basename\_node;\n\n";

print HEADER "typedef enum { LSMP_LSMP_ERROR,\n";
foreach $name (@elements)
{
	$ucname = uc $name;
	print HEADER "LSMP_$ucname,\n";
}
print HEADER "LSMP_LSMP_NODE } LSMP\_$basename\_type;\n";

print HEADER <<EOF;

typedef struct xml_tree
{
	LSMP\_$basename\_type type;
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

EOF

#foreach $name (@elements)
#{
#	print "$name\n";
#	foreach $child (keys %{ $kids{$name} } )
#	{
#		print "\t$child $kids{$name}{$child}\n";
#	}
#	foreach $att (keys %{ $atts{$name} } )
#	{
#		print "\t$att $atts{$name}{$att}{spec}";
#		print " $atts{$name}{$att}{def}\n";
#	}
#}
 
foreach $name (@elements)
{
	print HEADER  "/*\n$lines{$name}\n*/\n";
	print HEADER  "struct $name {\n";

	foreach $child (keys %{ $kids{$name} } )
	{
		$key = $child;
		$type = $kids{$name}{$child};

		if($key eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
			print HEADER  "\tchar *pcdata;\n";
		}
		elsif($type eq "+")
		{
			print HEADER  "\txml_tree **$key;\t/* at least one*/\n";
			print HEADER  "\tint n_$key;\n";
		}
		elsif($type eq "*")	
		{
			print HEADER  "\txml_tree **$key;\t/* 0 or more */\n";
			print HEADER  "\tint n_$key;\n";
		}
		elsif($type eq "")	
		{
			print HEADER  "\txml_tree *$key;\t/* required */\n";
		}
		elsif($type eq "?")	
		{
			print HEADER  "\txml_tree *$key;\t/* optional */\n";
		}
		else
		{
			print "ERROR $key $type\n";
		}
	}
	print HEADER "\n";

	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		if($spec eq "CDATA" )
		{	print HEADER  "\tchar *$att;\t/* ($def) $rest */\n"; }
		else
		{	print HEADER  "\tchar *$att;\t/* $spec ($def) $rest */\n"; }
	}

	print HEADER "};\n\n";
}

print HEADER <<EOF;
#endif
EOF

print PARSER <<EOF;

int create_node_specific_info(xml_tree *node)
{
	int i;

	node->error_message = NULL; node->error = 0;
EOF
if($debug)
{
print PARSER <<EOF;
	fprintf(stderr,"create_node_specific_info(%s)\\n",node->name);
EOF
}
print PARSER <<EOF;
	if(0) {}
EOF

foreach $name (@elements)
{
	$ucname = uc $name;
	print PARSER "\telse if(!strcmp(node->name,\"$origname{$name}\")) node->type = LSMP_$ucname;\n";
}
print PARSER<<EOF;
	else
	{
		node->error_message = (char *) calloc(sizeof(char),
			strlen("$name: bad element name ") + strlen(node->name) +1 );
		node->type = LSMP_LSMP_ERROR;
		node->error = $errno;
		sprintf(node->error_message,"$name: bad element name %s",node->name);
	}

	switch(node->type)
	{
EOF
++$errno;
foreach $name (@elements)
{
	$ucname = uc $name;
	print PARSER "\tcase LSMP_$ucname:\n";
	print PARSER "\t\tnode->u.$name = (struct $name *) malloc(sizeof(struct $name));\n"; 
	foreach $child (keys %{ $kids{$name} } )
	{
		$type = $kids{$name}{$child};
		if($child eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
		}
		elsif($type eq "+" || $type eq "*")
		{
			print PARSER  "\t\tnode->u.$name->$child = (xml_tree **) calloc(sizeof(xml_tree *),node->n_child);\n";
			print PARSER  "\t\tnode->u.$name->n_$child = 0;\n";
		}
		elsif($type eq "" || $type eq "?")	
		{
			print PARSER  "\t\tnode->u.$name->$child =NULL;\n";
		}
		else
		{
			print "ERROR $child $type\n";
			print PARSER  "\t\tnode->error = $errno;\n";
		}
	}

	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		print PARSER "\t\tnode->u.$name->$att = NULL;\n";
	}

if($debug)
{
print PARSER <<EOF;
fprintf(stderr,"copy child\\n");
EOF
}
	print PARSER <<EOF;

		/** Copy child pointers **/
		for(i=0;i<node->n_child;++i)
		{
		    create_node_specific_info(node->children[i]);
		    switch(node->children[i]->type)
		    {
EOF
	foreach $child (keys %{ $kids{$name} } )
	{
		$ucchild = uc $child;
		$type = $kids{$name}{$child};
		if($child eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
		}
		elsif($type eq "+" || $type eq "*")
		{
			print PARSER <<EOF;
		    case LSMP_$ucchild:
			node->u.$name->$child\[node->u.$name->n_$child\++] = node->children[i];
			break;
EOF
		}
		elsif($type eq "" || $type eq "?")	
		{
			print PARSER <<EOF;
		    case LSMP_$ucchild:
			if(node->u.$name->$child != NULL)
			{
				node->error_message = strdup("$name: $child can only be defined once");
				node->error = $errno;
			}
			else
				node->u.$name->$child = node->children[i];
			break;
EOF
			++$errno;
		}
	}

	print PARSER <<EOF;
		    default:
		    }
		}
		/** check number of children **/
EOF

if($debug)
{
print PARSER <<EOF;
fprintf(stderr,"Check child\\n");
EOF
}		
	foreach $child (keys %{ $kids{$name} } )
	{
		$ucchild = uc $child;
		$type = $kids{$name}{$child};
		if($child eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
		}
		elsif($type eq "+")
		{
			print PARSER <<EOF;
		if(node->u.$name->n_$child < 1)
		{
			node->error = $errno;
			node->error_message = strdup("$name $child must be defined at least once");
		}
EOF
			++$errno;
		}
		elsif($type eq "")	
		{
			print PARSER <<EOF;
		if(node->u.$name->$child == NULL)
		{
			node->error_message = strdup("$name: $child must be defined");
			node->error = $errno;
		}
EOF
			++$errno;
		}
	}

if($debug)
{
print PARSER <<EOF;
fprintf(stderr,"copy Att\\n");
EOF
}
	print PARSER <<EOF;
		/* copy attributes */
		for(i=0;i<node->n_attr;i+=2)
		{
			if(0) {}
EOF
	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		print PARSER <<EOF;
			else if(!strcmp(node->attr[i],\"$att\"))
				node->u.$name->$att = node->attr\[i+1\];
EOF
	}
	print PARSER <<EOF;
			else
			{
				node->error_message = (char *) calloc(sizeof(char),
					strlen(\"$name: Un matched attributes: %s\\n\")+strlen(node->attr\[i\]));
				sprintf(node->error_message,\"$name: Un matched attributes: %s\\n\",node->attr\[i\]);
				node->error = $errno;
			}
		}
EOF
if($debug)
{
print PARSER <<EOF;
fprintf(stderr,"Check Att\\n");
EOF
}
	++$errno;

	print PARSER "\t\t/** Check attributes **/\n";

	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		print PARSER "\t\t/* $att [$spec] [$def] */\n";
		if($spec eq "CDATA")
		{
		}
		elsif($spec eq "NMTOKEN" || $spec eq "NMTOKENS" || $spec eq "ENTITY" 
			|| $spec eq "ENTITIES" || $spec eq "ID" || $spec eq "IDREF" 
			|| $spec eq "IDREFS" || $spec eq "NOTATION" )
		{
			print "Sorry can't sus $spec attribute types\n";
			print PARSER  "\t\t{ node->error_message = strdup(\"$name: sorry can't handle $spec attribute types\\n\"); node->error = $errno; }\n";
			++$errno;
		}		
		else
		{
			$spec =~ s/^\((.*)\)$/$1/;
			print PARSER  "\t\tif(node->u.$name->$att == NULL) {}\n";
			foreach $alt (split /\|/,$spec)
			{
				print PARSER  "\t\telse if(!strcmp(node->u.$name->$att,\"$alt\")) {}\n";
			}
			print PARSER  "\t\telse\n"; 
			print PARSER  <<EOF;
		{ node->error_message = (char *) calloc(sizeof(char),
			strlen(\"$name: atribute $att (%s) not in specification ($spec)\\n\") + strlen(node->u.$name->$att));
			sprintf(node->error_message,\"$name: atribute $att (%s) not in specification ($spec)\\n\",node->u.$name->$att);
		}
EOF
			++$errno;
		}
		
				
		if($def eq "#IMPLIED")
		{
		}
		elsif($def eq "#REQUIRED")
		{
			print PARSER  "\t\tif(node->u.$name->$att == NULL)\n";
			print PARSER  "\t\t{ node->error_message = strdup(\"$name: atribute $att must be specified\\n\"); node->error = $errno; }\n";
			++$errno;
		}
		elsif($def eq "#FIXED")
		{
			print "Sorry can't sus fixed elements\n";
			print PARSER  "\t\t{ node->error_message = strdup(\"$name: sorry can't handle fixed elements\\n\"); node->error = $errno; }\n";
			++$errno;
		}
		else
		{
			print PARSER  "\t\tif(node->u.$name->$att == NULL)\n";
			print PARSER  "\t\t\tnode->u.$name->$att = $def;\n";
			++$errno;

		}	
	}

	print PARSER <<EOF;
		break;
EOF
}

print PARSER <<EOF;
	}
EOF

if($debug)
{
print PARSER <<EOF;
fprintf(stderr,"Finished %s\\n",node->name);
EOF
}

print PARSER <<EOF;
}
EOF

