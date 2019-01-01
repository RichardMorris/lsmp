#GEOM = ../../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

OBJS =  asurf.o boxclev.o bern.o cells.o topology.o plotbox.o
OBJS2 =  nographics.o boxclev.o bern.o cells.o topology.o plotbox.o
OBJSCGIVRML =  asurfCV.o boxclev.o bern.o cells.o topology.o plotjvx.o facets.o SturmSequences.o
OBJS_OL =  asurfOL.o boxclev.o bern.o cells.o topology.o plotjvx.o facets.o SturmSequences.o

all: asurfCV asurfOL
lint:
	lint  -I../include ${OBJS:.o=.c)
asurfbase: ${OBJS}
	$(CC) ${OBJS} ../geomcomm.o  ${SOMELIBS_SD} -o asurfbase
nographics: ${OBJS2}
	$(CC) ${OBJS2}  ${SOMELIBS_SD_NG} -o nographics

asurfOL.o: asurfCV.c
	cp asurfCV.c asurfOL.c
	${CC} ${CFLAGS} -DUSE_KNOWN_SINGS -c asurfOL.c -o asurfOL.o
asurfCV: ${OBJSCGIVRML}
	$(CC) ${OBJSCGIVRML} ../CVcommon.o ${SOMELIBS_SD_NG_NM} -o asurfCV
asurfOL: ${OBJS_OL}
	$(CC) ${OBJS_OL} ../CVcommon.o ../jvx/xmlcore.o ../jvx/jvxChecker.o ${SOMELIBS_SD_NM_JVX} -o asurfOL
clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)
