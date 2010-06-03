CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= qsort htable-list htable-tree binary-tree binary-tree-avl
LDFLAGS := -lpthread

RND_CNT := 1000000
TST_RND_CNT := 1000

RND_U32_OUT := u32.$(RND_CNT).rnd
RND_32B_OUT  := 32b.$(RND_CNT).rnd

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

htable-tree: htable.c binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DHTABLE_TREE $(CFLAGS) -o $@ $(LDFLAGS)

htable-tree-avl: htable.c binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DHTABLE_TREE -DBINARY_TREE_AVL $(CFLAGS) -o $@ $(LDFLAGS)

binary-tree: binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DBINARY_TREE_MAIN $(CFLAGS) -o $@ $(LDFLAGS)

binary-tree-avl: binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DBINARY_TREE_MAIN -DBINARY_TREE_AVL $(CFLAGS) -o $@ $(LDFLAGS)

.PHONY: rnd.u32 rnd.32b
rnd-u32:
	$(Q)dd if=/dev/urandom of=$(RND_U32_OUT) bs=$(RND_CNT) count=4 status=noxfer
rnd-32b:
	$(Q)dd if=/dev/urandom of=$(RND_32B_OUT) bs=$(RND_CNT) count=32 status=noxfer

test-qsort: qsort
	$(Q)dd if=/dev/urandom of=$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer
	$(Q)./qsort -s QS1 -i $@.$(TST_RND_CNT).rnd --dump
	$(Q)./qsort -s QS2 -i $@.$(TST_RND_CNT).rnd --dump

test-htable: htable-list htable-tree htable-tree-avl
	$(Q)dd if=/dev/urandom of=$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=32 status=noxfer
	$(Q)./htable-list -f simple -s 10 -i $@.$(TST_RND_CNT).rnd -g $@-list.$(TST_RND_CNT).dot
	$(Q)dot -Tpng -o $@-list.$(TST_RND_CNT).png $@-list.$(TST_RND_CNT).dot
	$(Q)./htable-tree -f simple -s 10 -i $@.$(TST_RND_CNT).rnd -g $@-tree.$(TST_RND_CNT).dot
	$(Q)dot -Tpng -o $@-tree.$(TST_RND_CNT).png $@-tree.$(TST_RND_CNT).dot
	$(Q)./htable-tree-avl -f simple -s 10 -i $@.$(TST_RND_CNT).rnd -g $@-tree-avl.$(TST_RND_CNT).dot
	$(Q)dot -Tpng -o $@-tree-avl.$(TST_RND_CNT).png $@-tree-avl.$(TST_RND_CNT).dot

test-binary-tree: binary-tree binary-tree-avl
	$(Q)dd if=/dev/urandom of=$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer
	$(Q)./binary-tree -i $@.$(TST_RND_CNT).rnd -g $@.$(TST_RND_CNT).dot
	$(Q)dot -Tpng -o $@.$(TST_RND_CNT).png $@.$(TST_RND_CNT).dot
	$(Q)./binary-tree-avl -i $@.$(TST_RND_CNT).rnd -g $@-avl.$(TST_RND_CNT).dot
	$(Q)dot -Tpng -o $@-avl.$(TST_RND_CNT).png $@-avl.$(TST_RND_CNT).dot

clean:
	$(Q)rm -rf $(ALGOS)
