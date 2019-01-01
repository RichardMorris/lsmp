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
$parser =~ s/\..*/Parser.c/;
$basename =~ s/\..*//;

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
$ele_line = "";
$matching_elements = 0;
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
print HEADER "*/\n\n";
print PARSER "*/\n\n";

print HEADER <<EOF;

#include <stdio.h>
#include <string.h>

typedef struct xml_tree
{
	int n_attr,n_child,alloc_child;
	char *name;
	char **attr;
	char *data;
	struct xml_tree **children;
} xml_tree;

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
	print HEADER "struct s_$name;\n";
}
foreach $name (@elements)
{
	print HEADER  "/*\n$lines{$name}\n*/\n";
	print HEADER  "typedef struct {\n";
	print HEADER  "\tint LSMPerror;\n";

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
			print HEADER  "\tstruct s_$key **$key;\t/* at least one*/\n";
			print HEADER  "\tint n_$key;\n";
		}
		elsif($type eq "*")	
		{
			print HEADER  "\tstruct s_$key **$key;\t/* 0 or more */\n";
			print HEADER  "\tint n_$key;\n";
		}
		elsif($type eq "")	
		{
			print HEADER  "\tstruct s_$key *$key;\t/* required */\n";
		}
		elsif($type eq "?")	
		{
			print HEADER  "\tstruct s_$key *$key;\t/* optional */\n";
		}
		else
		{
			print "ERROR ($key) ($type)\n";
		}
	}
	print HEADER "\n";

	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		if($spec eq "CDATA" )
		{	print HEADER  "\tchar *$att;\t/* ($default) $rest */\n"; }
		else
		{	print HEADER  "\tchar *$att;\t/* $spec ($default) $rest */\n"; }
	}

	print HEADER "} s_$name;\n\n";
	print HEADER "extern s_$name *create_$name(xml_tree *XMLnode);\n";
	print HEADER "extern int check_$name(s_$name *LSMPnode);\n";
}

print HEADER "typedef union {\n";
foreach $name (@elements)
{
	print HEADER "s_$name *$name;\n";
}
print HEADER "} LSMP\_$basename\_node;\n";

foreach $name (@elements)
{
	print PARSER  "/*\n$lines{$name}\n*/\n";
	print PARSER "s_$name *create_$name(xml_tree *XMLnode)\n{\n";
	print PARSER "\ts_$name *LSMPnode;\n";
	print PARSER "\tint LSMP_i;\n\n";
	print PARSER "\tLSMPnode = (s_$name *) malloc(sizeof(s_$name));\n";
	print PARSER "\tLSMPnode->LSMPerror = 0;\n";

	print PARSER "/* Allocate children */\n";
	foreach $child (keys %{ $kids{$name} } )
	{
		$key = $child;
		$type = $kids{$name}{$child};

		if($key eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
			print PARSER  "\tLSMPnode->pcdata = strdup(XMLnode->data);\n";
		}
		elsif($type eq "+")
		{
			print PARSER  "\tLSMPnode->$key = (struct s_$key **) calloc(sizeof(s_$key *),XMLnode->n_child);\n";
			print PARSER  "\tLSMPnode->n_$key = 0;\n";
		}
		elsif($type eq "*")	
		{
			print PARSER  "\tLSMPnode->$key = (struct s_$key **) calloc(sizeof(s_$key *),XMLnode->n_child);\n";
			print PARSER  "\tLSMPnode->n_$key = 0;\n";
		}
		elsif($type eq "" || $type eq "?")	
		{
			print PARSER  "\tLSMPnode->$key =NULL;\n";
		}
		else
		{
			print "ERROR ($key) ($type)\n";
		}
	}
	print PARSER "\n/* Initalise Attributes */\n";

	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		print PARSER "\tLSMPnode->$att = NULL;\n";
	}

	print PARSER "/* copy children */\n";

	print PARSER <<EOF;
	for(LSMP_i=0;LSMP_i<XMLnode->n_child;++LSMP_i)
	{
		if(0) {}
EOF
	foreach $child (keys %{ $kids{$name} } )
	{
		$key = $child;
		$type = $kids{$name}{$child};

		if($key eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
		}
		elsif($type eq "+" || $type eq "*")
		{
			print PARSER <<EOF;
		else if(!strcmp(XMLnode->children[LSMP_i]->name,\"$key\"))
			LSMPnode->$key\[LSMPnode->n_$key++\] = (struct s_$key *) create_$key\(XMLnode->children\[LSMP_i\]\);
EOF
		}
		elsif($type eq "" || $type eq "?")	
		{
			print PARSER <<EOF;
		else if(!strcmp(XMLnode->children\[LSMP_i\]->name,\"$key\"))
		{
			if(LSMPnode->$key != NULL)
			{
				fprintf(stderr,"Two occurances of $key\\n");
				LSMPnode->LSMPerror = 1;
			}
			else
				LSMPnode->$key = (struct s_$key *) create_$key\(XMLnode->children[LSMP_i]\);
		}
EOF
		}
	}
	print PARSER <<EOF;
		else
		{
			fprintf(stderr,"Un matched child element: %s\\n",XMLnode->children\[LSMP_i\]->name);
			LSMPnode->LSMPerror = 1;
		}
	}
EOF

	print PARSER "/* copy attributes */\n";

		print PARSER <<EOF;
	for(LSMP_i=0;LSMP_i<XMLnode->n_attr;LSMP_i+=2)
	{
		if(0) {}
EOF
	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		print PARSER <<EOF;
		else if(!strcmp(XMLnode->attr[LSMP_i],\"$att\"))
			LSMPnode->$att = strdup(XMLnode->attr\[LSMP_i+1\]\);
EOF
	}
	print PARSER <<EOF;
		else
		{
			fprintf(stderr,\"Un matched attributes: %s\\n\",XMLnode->attr\[LSMP_i\]);
			LSMPnode->LSMPerror = 1;
		}
	}
EOF

	print PARSER "/* test children */\n";
	foreach $child (keys %{ $kids{$name} } )
	{
		$key = $child;
		$type = $kids{$name}{$child};

		if($key eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
			print PARSER  "\tif(LSMPnode->pcdata == NULL)\n";
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR null pcdata\\n\"); LSMPnode->LSMPerror = 1; }\n";
			
		}
		elsif($type eq "+")
		{
			print PARSER  "\tif(LSMPnode->n_$key == 0)\n";
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR at least one $key required\\n\"); LSMPnode->LSMPerror = 1; }\n";
		}
		elsif($type eq "?")	
		{
			print PARSER  "\tif(LSMPnode->$key == NULL)\n";
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR at least one $key required\\n\"); LSMPnode->LSMPerror = 1; }\n";
		}
		elsif($type eq "" || $type eq "*")	
		{
		}
	}
	print PARSER "\n/* Checking Attributes */\n";

	foreach $att (keys %{ $atts{$name} } )
	{
		$spec = $atts{$name}{$att}{spec};
		$def = $atts{$name}{$att}{def};
		$rest = $atts{$name}{$att}{comment};

		print PARSER "/* $att [$spec] [$def] */\n";
		if($spec eq "CDATA")
		{
		}
		elsif($spec eq "NMTOKEN" || $spec eq "NMTOKENS" || $spec eq "ENTITY" 
			|| $spec eq "ENTITIES" || $spec eq "ID" || $spec eq "IDREF" 
			|| $spec eq "IDREFS" || $spec eq "NOTATION" )
		{
			print "Sorry can't sus $spec attribute types\n";
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR sorry can't handle $spec attribute types\\n\"); LSMPnode->LSMPerror = 1; }\n";
		}			
		else
		{
			$spec =~ s/^\((.*)\)$/$1/;
			print PARSER  "\tif(LSMPnode->$att == NULL) {}\n";
			foreach $alt (split /\|/,$spec)
			{
				print PARSER  "\telse if(!strcmp(LSMPnode->$att,\"$alt\")) {}\n";
			}
			print PARSER  "\telse\n"; 
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR atribute $att (%s) not in specification ($spec)\\n\",LSMPnode->$att); LSMPnode->LSMPerror = 1; }\n";
		}
		
				
		if($def eq "#IMPLIED")
		{
		}
		elsif($def eq "#REQUIRED")
		{
			print PARSER  "\tif(LSMPnode->$att == NULL)\n";
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR atribute $att must be specified\\n\"); LSMPnode->LSMPerror = 1; }\n";
		}
		elsif($def eq "#FIXED")
		{
			print "Sorry can't sus fixed elements\n";
			print PARSER  "\t\t{ fprintf(stderr,\"ERROR sorry can't handle fixed elements\\n\"); LSMPnode->LSMPerror = 1; }\n";
		}
		else
		{
		}	
	}
	print PARSER "\treturn LSMPnode;\n"; 
	print PARSER "} /* end create_$name */\n\n";

#	last if(++$my_count>2);
}

# check_element

foreach $name (@elements)
{
	print PARSER  "/*\n$lines{$name}\n*/\n";
	print PARSER "int check_$name(s_$name *LSMPnode)\n{\n";
	print PARSER "\tint LSMP_i, LSMP_ok;\n\n";

	print PARSER "\tLSMP_ok = LSMPnode->LSMPerror;\n\n";
	print PARSER "/* check children */\n";
	foreach $child (keys %{ $kids{$name} } )
	{
		$key = $child;
		$type = $kids{$name}{$child};

		if($child eq "EMPTY" ) { }
		elsif($type eq "#PCDATA" )
		{	
		}
		elsif($type eq "+" || $type eq "*")
		{
			print PARSER  "\tfor(LSMP_i=0;LSMP_i<LSMPnode->n_$key;++LSMP_i)\n";
			print PARSER  "\t\tLSMP_ok =  LSMP_ok && check_$key((s_$key *) LSMPnode->$key\[LSMP_i]);\n";
		}
		elsif($type eq "?" || $type eq "")	
		{
			print PARSER  "\tif(LSMPnode->$key != NULL)\n";
			print PARSER  "\t\tLSMP_ok = LSMP_ok && check_$key((s_$key *) LSMPnode->$key);\n";
		}
	}
	print PARSER "\treturn LSMP_ok;\n";
	print PARSER "} /* end check_$name */\n\n";
}


