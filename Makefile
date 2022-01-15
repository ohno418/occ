CC=gcc

occ: main.c
	$(CC) -o occ main.c

.PHONY: test
test: occ
	./test.sh

.PHONY: clean
clean:
	rm -f occ tmp tmp.s
