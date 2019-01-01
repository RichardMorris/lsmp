To compile under a particular platform create a symbolic link from
the appropriate makefile to makefile.host:
        rm makefile.host
        ln -s makefile.sgi makefile.host
then type make. 

Potential dificulties may occur in the lex scanner/yacc parser. Have a look
in the eqn/makefile.$(CPU) file for various ways to control the compilation of these.

At the end of the compilation process various executables will be created
	asurf/asurfCV		The algebraic surfaces webserver
	impsurf/impsurfCV	The implicit surface webserver 
	acurve/acurveCV		The algebraic curve webserver
	acurve3/acurve3CV	The algebraic curve in 3D webserver
	psurf/psurfCV		The parameterised surface webserver 
	bin/eqntool		A test program for the equation handeling
				can be used as a command line calculator
				use bin/eqntool -h for some incomprehensible help
	Multi/MultiMain		Test programs for multi-line equation 
	Multi/TransMain		Test programs for multi-line equation

The first five are the most intresting. These are intended to work as CGI programs
recieving POST requests. To mimick this two files
	asurf/cgitest 	and impsurf/cgitest 
have been created, which contain sample definitions. To use these two environment variables
need to be set:
	export REQUEST_METHOD=POST
	export CONTENT_LENGTH=163
under sh/bash or
	setenv REQUEST_METHOD POST
	setenv CONTENT_LENGTH 163
under csh/tcsh (for the impsurf the content length should be 165). Then do
	cd asurf
	asurfCV < cgitest > test.jvx
the jvx file created will have three non standard lines as headers, these will need to
be removed to allow the files to be read into javaview.

After testing that the executables work you should copy then to the appropriate directory
	cp asurf/asurfCV impsurf/impsurfCV  cgi-bin
in for the cgi-bin of the webserver. Both programs quietly write files
	asurf.error
	asurf.log
in the current working directory for the webserver. The two files need to be world
writable so that the the webserver can write to them. Do something like
	cd cgi-bin
	echo > asurf.error
	echo > asurf.log
	chmod a+w asurf.error asurf.log
to create and set up the files. The asurf.log file records the cgi requests and the
asurf.error records any errors which may occur. If you don't want these files to be created
look in the function read_cgi() in
	asurf/asurfCV.c	and impsurf/impsurfCV.c
to supress the creation of the log file and add a line
	freopen("/dev/null","a",stderr);
at the begining of main() routine in these two files (last function in the files).

If you want to remove the timeout limit or change the maximum allowable cpu time
uncomment the 
#define NO_TIMEOUT
line at the begining of asurf/asurfCV.c and impsurf/imsurfCV.c files or edit 
the TIMEOUT_LIMIT macro (expressed in milliseconds).

To set up the client end the html files
	AsurfClient.html
	ImpsurfClient.html
(found in the AsurfClient.zip archive) need to be edited so that the AsurfServer applet
parameter is set correctly. You may also want to edit the ARCHIVE and cabbae parameters.
	


