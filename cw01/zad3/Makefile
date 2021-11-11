CC=gcc
CCFLAGS=-Wall -g3 $(O)

run-tests: merger-test-static.run merger-test-shared.run merger-test-dynamic.run
	echo OPTIMIZATION FLAG:  $(O) > test_result.txt
	echo "STATIC LINKAGE:" >> test_result.txt
	echo exit | ./merger-test-static.run >> test_result.txt
	echo "SHARED LINKAGE:" >> test_result.txt
	echo exit | ./merger-test-shared.run >> test_result.txt
	echo "DYNAMIC LINKAGE:" >> test_result.txt
	echo exit | ./merger-test-dynamic.run >>test_result.txt
	echo "TESTS DONE"
	echo "Remember to clean if you want to use another optimization flag"

merger-test-static.run: merger.c libBlockTable.a MergerParsers.o MergerTester.o
	$(CC) -DTEST=1  $(CCFLAGS) $^ -o $@

merger-static.run: merger.c libBlockTable.a MergerParsers.o
	$(CC) $(CCFLAGS) $^ -o $@

merger-test-shared.run: merger.c MergerParsers.o MergerTester.o | libBlockTable.so.1.0.0
	$(CC) -DTEST=1 -lBlockTable -L ./ $(CCFLAGS) $^ -o $@

merger-shared.run: merger.c MergerParsers.o | libBlockTable.so.1.0.0
	$(CC) $(CCFLAGS) -lBlockTable -L ./ $^ -o $@

merger-test-dynamic.run: merger.c DynamicLoader.o MergerParsers-dynamic.o MergerTester-dynamic.o | libBlockTable.so.1.0.0
	$(CC) -DTEST=1 -DDYNAMIC=1 $(CCFLAGS) $^ -o $@ -ldl

merger-dynamic.run: merger.c DynamicLoader.o MergerParsers-dynamic.o | libBlockTable.so.1.0.0
	$(CC) -DDYNAMIC=1 $(CCFLAGS) $^ -o $@ -ldl

MergerTester.o: merger-tests/mergerTester.c merger-tests/mergerTester.h
	$(CC) -c $(CCFLAGS) $< -o $@

MergerTester-dynamic.o: merger-tests/mergerTester.c merger-tests/mergerTester.h
	$(CC) -DDYNAMIC=1 -c $(CCFLAGS) $< -o $@

MergerParsers.o: mergerInputParser.c mergerInputParser.h 
	$(CC) -c $(CCFLAGS) $< -o $@

MergerParsers-dynamic.o: mergerInputParser.c mergerInputParser.h 
	$(CC) -DDYNAMIC=1 -c $(CCFLAGS) $< -o $@	

libBlockTable.so.1.0.0: BlockTable-fpic.o
	$(CC) -shared -Wl,-soname,libBlockTable.so.1 -o libBlockTable.so.1.0.0 $< -lc
	ln -s $@ libBlockTable.so
	ln -s $@ libBlockTable.so.1

libBlockTable.a: BlockTable.o
	ar rcs libBlockTable.a $<

BlockTable-fpic.o: BlockTable.c BlockTable.h 
	$(CC) -c -fPIC $(CCFLAGS) $< -o $@

BlockTable.o: BlockTable.c BlockTable.h 
	$(CC) -c $(CCFLAGS) $< -o $@

DynamicLoader.o: dynamicLoader.c dynamicLoader.h
	$(CC) -c $(CCFLAGS) $< -o $@


clean:
	rm -f *.o 		|| true
	rm -f *.run 	|| true
	rm -f *.a 		|| true
	rm -f *.so 		|| true
	rm -f *.so.* 	|| true
	rm -f tempfile* || true
