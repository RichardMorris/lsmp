include ../makefile.host

OBJS = jvxChecker.o xmlmain.o xmlcore.o  
PROG = xmlmain
all: ${OBJS} ${PROG}
jvxParser.c : jvx.dtd genParser.pl
	perl genParser.pl jvx.dtd
jvx.h : jvx.dtd genParser.pl
	perl genParser.pl jvx.dtd
jvxChecker.c : jvx.dtd genChecker.pl
	perl genChecker.pl jvx.dtd
xmlcore.c : jvx.h
${PROG}: ${OBJS}
	$(CC) ${OBJS} ../CVcommon.o ${SOMELIBS_SD_JVX} -o ${PROG}

clean:
	rm -f *.o
distclean: clean
	rm -f $(PROG)$(EXEC_SUFFIX)
