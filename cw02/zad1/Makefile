CC=gcc
CCFLAGS=-Wall
NTESTS=50

.PHONY: runtimes clean

runtimes: zad1-sys.run zad1-lib.run ntimes.run
	@echo -e "Testing for small files"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad1-sys.run test-files/small_1 test-files/small_2
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad1-lib.run test-files/small_1 test-files/small_2

	@echo -e "Testing for medium files"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad1-sys.run test-files/medium_1 test-files/medium_2
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad1-lib.run test-files/medium_1 test-files/medium_2

	@echo -e "Testing for large files"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad1-sys.run test-files/large_1 test-files/large_2
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad1-lib.run test-files/large_1 test-files/large_2



zad1-sys.run: zad1-sys.c zad1-functions.o
	$(CC) $(CCFLAGS) $^ -o $@

zad1-lib.run: zad1-lib.c zad1-functions.o
	$(CC) $(CCFLAGS) $^ -o $@

zad1-functions.o: zad1-functions.c zad1-functions.h 
	$(CC) $(CCFLAGS) -c $< -o $@

ntimes.run: ../ntimes.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run		|| true
	rm -f *.o		|| true
	rm -f *.out		|| true