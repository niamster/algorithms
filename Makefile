CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= $(patsubst %.c,%,$(wildcard *.c))
LDFLAGS := -lpthread

ifeq ($(DEBUG), 1)
CFLAGS := -O0 -g -DDEBUG=1
endif

ifeq ($(MAKE_VERBOSE), 1)
Q :=
else
Q := @
endif

all: $(ALGOS)

qsort: qsort.c
	$(Q)$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

htable: htable.c
	$(Q)$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	$(Q)rm -rf $(ALGOS)
