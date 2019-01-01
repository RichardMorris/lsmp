#GEOM = ../../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

PROG = acurveCV
OBJS =  acurve.o mar.o
OBJSCV =  acurveCV.o mar.o
OBJS2 =  acurve.o bern.o cells.o facecleaver.o plotface.o topology.o
#ALLLIBS = -L../lib -L/usr/local/lib -L/usr/X11R6/lib -leqn -ltk -ltcl -lX11 -ldl -lm -lsocket
#COPTS = -O2 -I../include  -I/usr/local/include 

all: ${PROG}
lint:
	lint  -I../include ${OBJS:.o=.c)
${PROG}: ${OBJSCV}
	$(CC) ${OBJSCV}   ${SOMELIBS_SD_NG_NM} ../CVcommon.o -o ${PROG}  
clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)
