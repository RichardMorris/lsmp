include ../makefile.host
include makefile.$(CPU)
YFLAGS  = -dvl
#LFLAGS	= -sv
INCLUDES = $(SRC_INC)
LIBRARYS = ${SRC_LIB}
BINARIES = ${SRC_BIN}

OBJ = yacc.o $(FLEXOBJ) eqnbase.o eqnexpd.o eqnnames.o eqnfunct.o eqndiff.o eqnrpe.o \
	eqnpoly.o eqnsubst.o eqnvrpe.o eqncmp.o eqnC.o
 
.SUFFIXES: .doc .help .cm
all: libeqn.a eqntool
oldall: libeqn.a $(INCLUDES) eqn.doc eqn5.doc eqntool eqntool.doc
eqntool: eqntool.o libeqn.a
	$(CC) -O2 eqntool.o $(DIST_LIB) -L. -leqn -lm  -o eqntool $(CFLAGS)
test: test.o $(OBJ)
	$(CC)  test.o $(OBJ) $(DIST_LIB) -lm -o test
test2: test2.o $(OBJ)
	$(CC)  test2.o $(OBJ) $(DIST_LIB) -lm -o test2

eqnbase.o: eqnbase.c
	$(CC) $(CFLAGS) -o eqnbase.o -c eqnbase.c $(FLEX_DEF)

yacc.o: yacc.c
	$(CC) $(CFLAGS) -o yacc.o -c yacc.c $(YYPARSE_DEF)

yacc.c: yacc.y
#	yacc -dvl yacc.y
#	mv -f yacc.tab.c yacc.c
#	mv -f yacc.tab.h y.tab.h

install: all
	cp eqn.h $(INCLUDES)
	cp libeqn.a $(LIBRARYS)
	cp eqntool$(EXEC_SUFFIX) $(BINARIES)
lint:
	lint $(OBJ2:.o=.c)

$(MANPAGES3) $(INCLUDES): $$(@F)
	cp $? $@
$(LIBRARYS) $(BINARIES): $$(@F)
	cp $? $@

libeqn.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
.doc.help:
	ul -t dumb $< > $@
.cm.doc:
	createman < $< > $@

clean:
	rm -f *.o libeqn.a  yacc.c flex.c lex.c ytab.h y.tab.h y.output yacc.output
distclean: clean
	rm -f eqntool$(EXEC_SUFFIX)
