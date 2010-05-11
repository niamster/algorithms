CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= qsort htable-list
LDFLAGS := -lpthread

RND_U32_CNT := 1000000
RND_U32_OUT := rnd.u32

RND_32B_CNT  := 1000000
RND_32B_OUT  := rnd.32b

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

htable-list: htable.c
	$(Q)$(CC) $^ -DHTABLE_LIST $(CFLAGS) -o $@ $(LDFLAGS)

.PHONY: rnd.u32 rnd.32b
rnd.u32:
	$(Q)dd if=/dev/urandom of=$(RND_U32_OUT) bs=$(RND_U32_CNT) count=4 &>/dev/null
rnd.32b:
	$(Q)dd if=/dev/urandom of=$(RND_32B_OUT) bs=$(RND_32B_CNT) count=32 &>/dev/null

clean:
	$(Q)rm -rf $(ALGOS)
