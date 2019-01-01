include makefile.host

all: CVcommon.o
	cd eqn; make install
	cd Multi; make install
	cd acurve; make acurveCV
	cd acurve3; make acurve3CV
	cd impsurf; make impsurfCV
	cd psurf; make psurfCV
	cd jvx; make all
	cd asurf; make all
	cd mapping; make mappingCV
	cd intersect; make intersectCV
	cd icurve; make icurveCV
clean:
	cd eqn; make clean
	cd asurf; make clean
	cd psurf; make clean
	cd acurve; make clean
	cd acurve3; make clean
	cd Multi; make clean
	cd impsurf; make clean
	cd mapping; make clean
	cd intersect; make clean
	cd icurve; make clean
	cd jvx; make clean
	rm -f CVcommon.o
	rm -f bin/eqntool$(EXEC_SUFFIX) lib/libeqn.a include/eqn.h lib/libMulti.a lib/libMTrans.a
	rm -f */asurf.error */asurf.log
distclean:
	cd eqn; make distclean
	cd asurf; make distclean
	cd psurf; make distclean
	cd acurve; make distclean
	cd acurve3; make distclean
	cd Multi; make distclean
	cd impsurf; make distclean
	cd mapping; make distclean
	cd intersect; make distclean
	cd icurve; make distclean
	cd jvx; make distclean
	rm -f CVcommon.o
	rm -f bin/eqntool$(EXEC_SUFFIX) lib/libeqn.a include/eqn.h lib/libMulti.a lib/libMTrans.a
	rm -f */asurf.error */asurf.log
strip:
	strip asurf/asurfCV$(EXEC_SUFFIX) acurve/acurveCV$(EXEC_SUFFIX) acurve3/acurve3CV$(EXEC_SUFFIX) psurf/psurfCV$(EXEC_SUFFIX) impsurf/impsurfCV$(EXEC_SUFFIX) mapping/mappingCV$(EXEC_SUFFIX)
	
