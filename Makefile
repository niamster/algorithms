CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= $(patsubst %.c,%, $(wildcard [^_]*.c))
LDFLAGS := -lpthread

RND_U32_CNT := 1000000
RND_U32_OUT := rnd.u32

ifeq ($(DEBUG), 1)
CFLAGS := -O0 -g -DDEBUG=1
endif

ifeq ($(MAKE_VERBOSE), 1)
Q :=
else
Q := @
endif

all: $(ALGOS)

qsort: qsort.c __helpers.c
	$(Q)$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

.PHONY: rnd
rnd:
	$(Q)dd if=/dev/urandom of=$(RND_U32_OUT) bs=$(RND_U32_CNT) count=4 &>/dev/null

clean:
	$(Q)rm -rf $(ALGOS)
