CC=gcc
CCFLAGS=-Wall
NTESTS=50

.PHONY: runtimes clean

runtimes: zad2-sys.run zad2-lib.run ntimes.run
	@echo -e "Testing for small files"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad2-sys.run a test-files/small_2
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad2-lib.run a test-files/small_2

	@echo -e "Testing for medium files"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad2-sys.run a test-files/medium_2
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad2-lib.run a test-files/medium_2

	@echo -e "Testing for large files"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad2-sys.run a test-files/large_2
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad2-lib.run a test-files/large_2



zad2-sys.run: zad2-lib-sys.c 
	$(CC) -DSYSFUN $(CCFLAGS) $^ -o $@

zad2-lib.run: zad2-lib-sys.c 
	$(CC) $(CCFLAGS) $^ -o $@

ntimes.run: ../ntimes.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run		|| true
	rm -f *.o		|| true
	rm -f *.out		|| true