.PHONY: all
all: a.out
	@./a.out

a.out: main.c
	cc main.c

.PHONY: clean
clean:
	rm a.out
