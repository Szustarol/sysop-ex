CC=gcc
CCFLAGS=-Wall
NTESTS=20

.PHONY: runtimes clean

runtimes: zad5-sys.run zad5-lib.run ntimes.run
	@echo -e "Testing for small file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad5-sys.run test-files/small result 
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad5-lib.run test-files/small result 

	@echo -e "Testing for medium file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad5-sys.run test-files/medium result 
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad5-lib.run test-files/medium result 

	@echo -e "Testing for large file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad5-sys.run test-files/large result 
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad5-lib.run test-files/large result 


zad5-sys.run: zad5-lib-sys.c 
	$(CC) -DSYSFUN $(CCFLAGS) $^ -o $@

zad5-lib.run: zad5-lib-sys.c 
	$(CC) $(CCFLAGS) $^ -o $@

ntimes.run: ../ntimes.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run		|| true
	rm -f *.o		|| true
	rm -f *.out		|| true
	rm -f result	|| true