CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= qsort htable-list
LDFLAGS := -lpthread

RND_U32_CNT := 1000000
RND_U32_OUT := u32.$(RND_U32_CNT).rnd

RND_32B_CNT  := 1000000
RND_32B_OUT  := 32b.$(RND_32B_CNT).rnd

ifeq ($(DEBUG), 1)
CFLAGS := -O0 -g -DDEBUG=1
endif

ifeq ($(MAKE_VERBOSE), 1)
Q :=
else
Q := @
endif

HELPERS = __helpers.c dot.c

all: $(ALGOS)

qsort: qsort.c $(HELPERS)
	$(Q)$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

htable-list: htable.c $(HELPERS)
	$(Q)$(CC) $^ -DHTABLE_LIST $(CFLAGS) -o $@ $(LDFLAGS)

.PHONY: rnd.u32 rnd.32b
rnd-u32:
	$(Q)dd if=/dev/urandom of=$(RND_U32_OUT) bs=$(RND_U32_CNT) count=4 &>/dev/null
rnd-32b:
	$(Q)dd if=/dev/urandom of=$(RND_32B_OUT) bs=$(RND_32B_CNT) count=32 &>/dev/null

test-qsort: qsort
	$(Q)dd if=/dev/urandom of=u32.100.rnd bs=100 count=4 &>/dev/null
	$(Q)./qsort -s QS1 -i 32b.100.rnd --dump
	$(Q)./qsort -s QS2 -i 32b.100.rnd --dump

test-htable-list: htable-list
	$(Q)dd if=/dev/urandom of=32b.100.rnd bs=100 count=32 &>/dev/null
	$(Q)./htable-list -f simple -s 10 -i 32b.100.rnd -g graph.100.dot
	$(Q)dot -Tpng -o htable-list.100.png graph.100.dot

clean:
	$(Q)rm -rf $(ALGOS)
