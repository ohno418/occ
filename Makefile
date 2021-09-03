OBJS=occ.o tokenize.o parse.o codegen.o

occ: $(OBJS)
	$(CC) -o occ $(OBJS)

$(OBJS): occ.h

.PHONY: test
test: occ
	./test.sh

.PHONY: clean
clean:
	rm -f occ $(OBJS) tmp*
