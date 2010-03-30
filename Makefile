CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= $(patsubst %.c,%,$(wildcard *.c))
LDFLAGS := -lpthread

all: $(ALGOS)

qsort: qsort.c
	@$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)