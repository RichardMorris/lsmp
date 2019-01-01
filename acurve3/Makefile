#GEOM = ../../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

PROG = acurve3CV
OBJSCGIVRML =  acurve3CV.o boxcleaver.o bern.o cells.o topology.o plotvrml.o
COPTS = -O2 -I../include  -I/usr/local/include 

all: ${PROG}
lint:
	lint  -I../include ${OBJS:.o=.c)
${PROG}: ${OBJSCGIVRML}
	$(CC) ${OBJSCGIVRML}  ${SOMELIBS_SD_NG_NM} ../CVcommon.o -o acurve3CV

clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)

