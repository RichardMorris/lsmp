#GEOM = ../../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

OBJS =  impsurfCV.o boxclever.o eval.o newcells.o topology.o plotvrml.o
#OBJS =  impsurfCV.o boxclever.o eval.o newcells.o topology.o plotbox.o
#COPTS =  -O2 -I../include  -I/usr/local/include 
PROG = impsurfCV
all: impsurfCV
lint:
	lint  -I../include ${OBJS:.o=.c)
impsurfCV: ${OBJS}
	$(CC) ${OBJS}  ${SOMELIBS_SD_NG} ../CVcommon.o -o impsurfCV
clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)
