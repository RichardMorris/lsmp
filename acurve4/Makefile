#include ../globals
CC = gcc
#CFLAGS =  -g -I/usr/local/mingw/include -mno-cygwin -I../include
CFLAGS =  -g -I../include
MANPAGES1 = $(MAN1DIR)/acurve4.doc
BINARYS = $(BINDIR)/acurve4
OBJ =  main.o boxcleaver.o bern.o cells.o topology.o plotbox.o 
OLDOBJ =  main.o panels.o oldbox.o oldbern.o cells.o topology.o plotbox.o help.o
INCLUDES = cells.h bern.h
LIB =  -L../lib -leqn -lm 
#LIB =  -L/usr/local/mingw/lib -mno-cygwin -L../lib -leqn -lm 
OLDLIB = -L../lib -lformslx -leqn -lforms -lfm_s -lgl_s -lm  -lc_s

all: acurve4

acurve4: $(OBJ) $(INCLUDES) 
	gcc $(LDFLAGS) $(OBJ) $(LIB) -o $@
oldint: $(OLDOBJ) $(INCLUDES)
	cc $(LDFLAGS) $(OLDOBJ) $(LIB) -o $@

install: $(MANPAGES1) $(BINARYS)

$(BINARYS) $(MANPAGES1): $$(@F)
	cp $? $@

acurve4.doc: acurve4.cm
	createman < acurve4.cm > acurve4.doc
acurve4.help: acurve4.doc
	ul -t dumb acurve4.doc > acurve4.help
help.c: acurve4.help help.sed
	sed -f help.sed acurve4.help > help.c
panels.c: fd_panels.c pan.sed
	sed -f pan.sed fd_panels.c > panels.c


clean:
	rm -f $(OBJS)

distclean: clean
	rm -f $(BINARYS) $(MANPAGES1)
