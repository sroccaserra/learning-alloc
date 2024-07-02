.PHONY: all
all: a.out
	@./a.out

a.out: main.c arena.h
	cc main.c

.PHONY: clean
clean:
	rm a.out
