.PHONY: all
all: a.out
	@./a.out

a.out: src/main.c src/arena.h
	cc src/main.c

.PHONY: clean
clean:
	rm a.out
