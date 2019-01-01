#GEOM = ../Geomview
#include ${GEOM}/makefiles/Makedefs.global
include ../makefile.host

OBJSCGIVRML =  psurfCV.o 
#COPTS = -O2 -I../include  -I/usr/local/include 
PROG = psurfCV

all: psurfCV
lint:
	lint  -I../include ${OBJS:.o=.c)
psurfCV: ${OBJSCGIVRML}
	$(CC) ${OBJSCGIVRML}  ../CVcommon.o ${SOMELIBS_SD_NG} -o psurfCV

clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)
