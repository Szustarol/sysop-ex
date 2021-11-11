CC=gcc
CCFLAGS=-Wall

.DEFAULT_GOAL := zad1.run

.PHONY:clean run

run: zad1.run
	./$< 10

zad1.run: zad1.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run		|| true