#GEOM = ../../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

#CLIPOBJS =  clipmain.o clip.o intersect.o GEOMintersect.o converge.o \
#	ridge_intersect.o unor_intersect.o ../map.o

PROG = intersectCV
OBJSCGIVRML =   intersectCV.o converge.o intersect.o JVXintersect.o JVXclip.o ridge_intersect.o unor_intersect.o

all: ${PROG}
lint:
	lint  -I../include ${OBJS:.o=.c)
${PROG}: ${OBJSCGIVRML}
	$(CC) ${OBJSCGIVRML} ../jvx/xmlcore.o ../jvx/jvxChecker.o ../CVcommon.o ${SOMELIBS_SD_JVX} -o ${PROG}

clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)

