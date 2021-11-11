CC=gcc
CCFLAGS=-Wall
NTESTS=500

.PHONY: runtimes clean

runtimes: zad3-sys.run zad3-lib.run ntimes.run
	@echo -e "Testing for dane.txt file"
	@echo System version:
	@./ntimes.run $(NTESTS) ./zad3-sys.run
	@echo Lib version:
	@./ntimes.run $(NTESTS) ./zad3-lib.run



zad3-sys.run: zad3-lib-sys.c 
	$(CC) -DSYSFUN $(CCFLAGS) $^ -o $@

zad3-lib.run: zad3-lib-sys.c 
	$(CC) $(CCFLAGS) $^ -o $@

ntimes.run: ../ntimes.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run		|| true
	rm -f *.o		|| true
	rm -f *.out		|| true
	rm -f a.txt		|| true
	rm -f b.txt		|| true
	rm -f c.txt		|| true