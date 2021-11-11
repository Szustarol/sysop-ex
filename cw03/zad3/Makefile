CC= gcc
CCFLAGS= -Wall

.PHONY: clean  test

clean: 
	rm -f *.run || true

test: zad.run
	./zad.run sampleDirectory text 3

zad.run: zad3.c
	$(CC) $(CCFLAGS) $< -o $@ 

