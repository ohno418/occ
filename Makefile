CC=gcc
CFLAGS=-Wall

occ: main.o tokenize.o parse.o codegen.o type.o debug.o
	$(CC) -o occ main.o tokenize.o parse.o codegen.o type.o debug.o

main.o: main.c

tokenize.o: tokenize.c

parse.o: parse.c

codegen.o: codegen.c

debug.o: debug.c

type.o: type.c

.PHONY: test
test: occ
	./test.sh

.PHONY: clean
clean:
	rm -f occ tmp tmp.s \
		main.o tokenize.o parse.o codegen.o type.o debug.o
