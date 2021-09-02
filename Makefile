occ: occ.c
	gcc -o occ occ.c

.PHONY: test
test: occ
	./test.sh

.PHONY: clean
clean:
	rm -f occ
