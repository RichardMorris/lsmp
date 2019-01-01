include ../makefile.host

OBJSCGIVRML =  mappingCV.o JvxMap.o
#COPTS = -O2 -I../include  -I/usr/local/include 
PROG = mappingCV

all: mappingCV
lint:
	lint  -I../include ${OBJS:.o=.c)
${PROG}: ${OBJSCGIVRML}
	$(CC) ${OBJSCGIVRML} ../jvx/xmlcore.o ../jvx/jvxChecker.o ../CVcommon.o ${SOMELIBS_SD_JVX} -o ${PROG}

clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)
