CC=gcc
CCFLAGS=-Wall
NTESTS=500

.PHONY: runtimes clean

runtimes: zad4-sys.run zad4-lib.run ntimes.run
	@echo -e "Testing for small file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad4-sys.run test-files/small result iwillbereplaced REPLACED!
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad4-lib.run test-files/small result iwillbereplaced REPLACED!

	@echo -e "Testing for medium file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad4-sys.run test-files/medium result iwillbereplaced REPLACED!
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad4-lib.run test-files/medium result iwillbereplaced REPLACED!

	@echo -e "Testing for large file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad4-sys.run test-files/large result iwillbereplaced REPLACED!
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad4-lib.run test-files/large result iwillbereplaced REPLACED!


zad4-sys.run: zad4-lib-sys.c 
	$(CC) -DSYSFUN $(CCFLAGS) $^ -o $@

zad4-lib.run: zad4-lib-sys.c 
	$(CC) $(CCFLAGS) $^ -o $@

ntimes.run: ../ntimes.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run		|| true
	rm -f *.o		|| true
	rm -f *.out		|| true
	rm -f result	|| true