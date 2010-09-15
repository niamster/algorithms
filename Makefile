CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= qsort htable-list htable-tree binary-tree binary-tree-avl
LDFLAGS := -lpthread

RND_CNT := 1000000
TST_RND_CNT := 100

TST_GEN_GRAPH := yes

TST_REGEN_RND := no

TST_HASH_SIZE := 1024

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
	$(Q)dd if=/dev/urandom of=$(RND_U32_OUT) bs=$(RND_CNT) count=4 status=noxfer >/dev/null 2>&1
rnd-32b:
	$(Q)dd if=/dev/urandom of=$(RND_32B_OUT) bs=$(RND_CNT) count=32 status=noxfer >/dev/null 2>&1

test-qsort: qsort
	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "qsort-1"
	$(Q)./qsort -s QS1 -i $@.$(TST_RND_CNT).rnd --dump
	$(Q)echo "qsort-2"
	$(Q)./qsort -s QS2 -i $@.$(TST_RND_CNT).rnd --dump

test-htable: htable-list htable-tree htable-tree-avl
	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=32 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "htable-list-additive"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f additive -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-list-additive.$(TST_RND_CNT).dot; else ./htable-list -f additive -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-list-additive.$(TST_RND_CNT).png $@-list-additive.$(TST_RND_CNT).dot; fi
	$(Q)echo "htable-list-rotating"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f rotating -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-list-rotating.$(TST_RND_CNT).dot; else ./htable-list -f rotating -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-list-rotating.$(TST_RND_CNT).png $@-list-rotating.$(TST_RND_CNT).dot; fi
	$(Q)echo "htable-list-sfh"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f sfh -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-list-sfh.$(TST_RND_CNT).dot; else ./htable-list -f sfh -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-list-sfh.$(TST_RND_CNT).png $@-list-sfh.$(TST_RND_CNT).dot; fi
	$(Q)echo "htable-list-bob-jenkin"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f bob-jenkin -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-list-bob-jenkin.$(TST_RND_CNT).dot; else ./htable-list -f bob-jenkin -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)echo "htable-list-sdbm"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f sdbm -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-list-sdbm.$(TST_RND_CNT).dot; else ./htable-list -f sdbm -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-list-sdbm.$(TST_RND_CNT).png $@-list-sdbm.$(TST_RND_CNT).dot; fi

	$(Q)echo "htable-tree-additive"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-tree -f additive -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-tree-additive.$(TST_RND_CNT).dot; else ./htable-tree -f additive -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-tree-additive.$(TST_RND_CNT).png $@-tree-additive.$(TST_RND_CNT).dot; fi
	$(Q)echo "htable-tree-avl-additive"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-tree-avl -f additive -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd -g $@-tree-avl-additive.$(TST_RND_CNT).dot; else ./htable-tree-avl -f additive -s $(TST_HASH_SIZE) -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-tree-avl-additive.$(TST_RND_CNT).png $@-tree-avl-additive.$(TST_RND_CNT).dot; fi

test-binary-tree: binary-tree binary-tree-avl
	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "binary-tree"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree -i $@.$(TST_RND_CNT).rnd -g $@.$(TST_RND_CNT).dot; else ./binary-tree -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@.$(TST_RND_CNT).png $@.$(TST_RND_CNT).dot; fi
	$(Q)echo "binary-tree-avl"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree-avl -i $@.$(TST_RND_CNT).rnd -g $@-avl.$(TST_RND_CNT).dot; else ./binary-tree-avl -i $@.$(TST_RND_CNT).rnd; fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $@-avl.$(TST_RND_CNT).png $@-avl.$(TST_RND_CNT).dot; fi

clean:
	$(Q)rm -rf $(ALGOS)
