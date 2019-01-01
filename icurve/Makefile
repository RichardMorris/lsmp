#GEOM = ../../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

PROG = icurveCV
OBJSCGIVRML =   icurveCV.o
OBJS =  icurve.o
#COPTS = -O2 -I../include -I/usr/local/include

all: ${PROG}
lint:
	lint  -I../include ${OBJS:.o=.c)
${PROG}: ${OBJSCGIVRML}
	$(CC) ${OBJSCGIVRML} ../jvx/xmlcore.o ../jvx/jvxChecker.o ../CVcommon.o ${SOMELIBS_SD_JVX} -o ${PROG}

clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)




