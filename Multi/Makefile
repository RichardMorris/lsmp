include ../makefile.host
#TESTLIBS = -L. -L${SRC_LIB} -L/usr/local/lib -lMulti -leqn  -lm -lc
TESTLIBS = -L. -lMulti $(SOMELIBS_SD_NG_NM)

MULOBJS = Multi.o Multidiff.o Mvrpe.o MultiC.o
TRANSOBJS = MTrans.o
#CFLAGS =  -O2  -I

all:  libMulti.a libMTrans.a
install: all
	cp Multi.h MTrans.h ${SRC_INC}
	cp libMulti.a libMTrans.a ${SRC_LIB}
TransMain: MTrans.o TransMain.o  libMulti.a
	$(CC)  TransMain.o MTrans.o ${TESTLIBS} -o TransMain
MultiMain: MultiMain.o  libMulti.a
	$(CC)  MultiMain.o  ${TESTLIBS} -o MultiMain
libMulti.a: $(MULOBJS)
	$(AR) $(ARFLAGS) $@ $(MULOBJS)
libMTrans.a: $(TRANSOBJS)
	$(AR) $(ARFLAGS) $@ $(TRANSOBJS)

clean:
	rm -f *.o *.a 
distclean: clean
	rm -f TransMain$(EXEC_SUFFIX) MultiMain$(EXEC_SUFFIX)
