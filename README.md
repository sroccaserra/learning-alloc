## References

K&R, Section 5.4, Address Arithmetic

A rudimentary storage allocator.

```c
#define ALLOCSIZE 10000 /* size of available space */

static char allocbuf[ALLOCSIZE];  /* storage for alloc */
static char *allocp = allocbuf;   /* next free position */

char *alloc(int n)  /* return pointer to n characters */
{
    if (allocbuf + ALLOCSIZE - allocp >= n) { /* it fits */
        allocp += n;
        return allocp - n; /* old p */
    } else      /* not enough room */
        return 0;
}

void afree(char *p)  /* free storage pointed to by p */
{
    if (p >= allocbuf && p < allocbuf + ALLOCSIZE)
        allocp = p;
}
```

K&R, Section 8.7, Example--A Storage Allocator

To simplify alignment, all blocks are multiples of the header size, and the
header is aligned properly.

```c
typedef long Align;  /* for alignment to long boundary */

union header {       /* block header: */
    struct {
        union header *ptr; /* next block if on free list */
        unsigned size;     /* size of this block */
    } s;
    Align x;         /* force alignment of blocks */
};

typedef union header Header;
```

## Links

- <https://www.gingerbill.org/series/memory-allocation-strategies/>
