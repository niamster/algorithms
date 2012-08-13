CC		:= gcc
CFLAGS	:= -O3
ALGOS	:= sort \
		binary-search \
		htable-list htable-tree htable-tree-avl wq \
		binary-tree binary-tree-avl binary-tree-rb binary-tree-random binary-tree-treap \
		btree \
		bitfield \
		kalman moving-average alpha-beta alpha-beta-gamma
LDFLAGS := -lpthread

RND_CNT := 1000000
TST_RND_CNT := 100

TST_GEN_GRAPH := yes

TST_REGEN_RND := no

TST_HASH_SIZE := 256

RFLAGS        :=

RND_U32_OUT := u32.$(RND_CNT).rnd
RND_32B_OUT := 32b.$(RND_CNT).rnd

OUT_DIR	:= /tmp/algos

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

sort: sort.c qsort.c hsort.c msort.c $(HELPERS)
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

binary-tree-rb: binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DBINARY_TREE_MAIN -DBINARY_TREE_RB $(CFLAGS) -o $@ $(LDFLAGS)

binary-tree-random: binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DBINARY_TREE_MAIN -DBINARY_TREE_RANDOM -DRANDOM_PREGEN=0xFFFF $(CFLAGS) -o $@ $(LDFLAGS)

binary-tree-treap: binary_tree.c $(HELPERS)
	$(Q)$(CC) $^ -DBINARY_TREE_MAIN -DBINARY_TREE_TREAP -DRANDOM_PREGEN=0xFFFF $(CFLAGS) -o $@ $(LDFLAGS)

btree: btree.c $(HELPERS)
	$(Q)$(CC) $^ -DBTREE_MAIN $(CFLAGS) -o $@ $(LDFLAGS)

binary-search: binary_search.c qsort.c $(HELPERS)
	$(Q)$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

wq: workqueue.c notification.c $(HELPERS)
	$(Q)$(CC) -DWORKQUEUE_MAIN $^ $(CFLAGS) -o $@ $(LDFLAGS)

bitfield: bitfield.h bitfield.c $(HELPERS)
	$(Q)$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

kalman: kalman.c $(HELPERS)
	$(Q)$(CC) -DKALMAN_MAIN $^ $(CFLAGS) -o $@ $(LDFLAGS)

moving-average: moving_average.c $(HELPERS)
	$(Q)$(CC) -DMOVING_AVERAGE_MAIN $^ $(CFLAGS) -o $@ $(LDFLAGS)

alpha-beta: alpha_beta.c $(HELPERS)
	$(Q)$(CC) -DALPHA_BETA_MAIN $^ $(CFLAGS) -o $@ $(LDFLAGS)

alpha-beta-gamma: alpha_beta_gamma.c $(HELPERS)
	$(Q)$(CC) -DALPHA_BETA_GAMMA_MAIN $^ $(CFLAGS) -o $@ $(LDFLAGS)

.PHONY: rnd.u32 rnd.32b
rnd-u32:
	$(Q)dd if=/dev/urandom of=$(RND_U32_OUT) bs=$(RND_CNT) count=4 status=noxfer >/dev/null 2>&1
rnd-32b:
	$(Q)dd if=/dev/urandom of=$(RND_32B_OUT) bs=$(RND_CNT) count=32 status=noxfer >/dev/null 2>&1

test-qsort: sort
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "qsort-1 single threaded"
	$(Q)./sort -s QS1 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "qsort-1"
	$(Q)./sort -s QS1 -t2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "qsort-2 single threaded"
	$(Q)./sort -s QS2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "qsort-2"
	$(Q)./sort -s QS2 -t2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="

test-hsort: sort
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "hsort"
	$(Q)./sort -s HS -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="

test-msort: sort
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "msort-1 single threaded"
	$(Q)./sort -s MS1 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "msort-1"
	$(Q)./sort -s MS1 -t2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "msort-2"
	$(Q)./sort -s MS2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="

test-sort: sort
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "qsort-1 single threaded"
	$(Q)./sort -s QS1 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "qsort-1"
	$(Q)./sort -s QS1 -t2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "qsort-2 single threaded"
	$(Q)./sort -s QS2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "qsort-2"
	$(Q)./sort -s QS2 -t2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "hsort"
	$(Q)./sort -s HS -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "msort-1 single threaded"
	$(Q)./sort -s MS1 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "msort-1"
	$(Q)./sort -s MS1 -t2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="
	$(Q)echo "msort-2"
	$(Q)./sort -s MS2 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)
	$(Q)echo "==========================================="

test-htable: htable-list htable-tree htable-tree-avl
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=32 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "htable-list-additive"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f additive -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-list-additive.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-list -f additive -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-list-additive.$(TST_RND_CNT).png $(OUT_DIR)/$@-list-additive.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "htable-list-rotating"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f rotating -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-list-rotating.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-list -f rotating -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-list-rotating.$(TST_RND_CNT).png $(OUT_DIR)/$@-list-rotating.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "htable-list-sfh"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f sfh -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-list-sfh.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-list -f sfh -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-list-sfh.$(TST_RND_CNT).png $(OUT_DIR)/$@-list-sfh.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "htable-list-bob-jenkin"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f bob-jenkin -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-list-bob-jenkin.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-list -f bob-jenkin -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-list-bob-jenkin.$(TST_RND_CNT).png $(OUT_DIR)/$@-list-bob-jenkin.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "htable-list-sdbm"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-list -f sdbm -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-list-sdbm.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-list -f sdbm -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-list-sdbm.$(TST_RND_CNT).png $(OUT_DIR)/$@-list-sdbm.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="

	$(Q)echo "htable-tree-additive"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-tree -f additive -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-tree-additive.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-tree -f additive -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-tree-additive.$(TST_RND_CNT).png $(OUT_DIR)/$@-tree-additive.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "htable-tree-avl-additive"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./htable-tree-avl -f additive -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-tree-avl-additive.$(TST_RND_CNT).dot $(RFLAGS); else ./htable-tree-avl -f additive -s $(TST_HASH_SIZE) -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-tree-avl-additive.$(TST_RND_CNT).png $(OUT_DIR)/$@-tree-avl-additive.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="

test-binary-tree: binary-tree binary-tree-avl binary-tree-rb binary-tree-random binary-tree-treap
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "binary-tree"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@.$(TST_RND_CNT).dot $(RFLAGS); else ./binary-tree -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@.$(TST_RND_CNT).png $(OUT_DIR)/$@.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "binary-tree-avl"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree-avl -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-avl.$(TST_RND_CNT).dot $(RFLAGS); else ./binary-tree-avl -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-avl.$(TST_RND_CNT).png $(OUT_DIR)/$@-avl.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "binary-tree-rb"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree-rb -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-rb.$(TST_RND_CNT).dot $(RFLAGS); else ./binary-tree-rb -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-rb.$(TST_RND_CNT).png $(OUT_DIR)/$@-rb.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "binary-tree-random"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree-random -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-random.$(TST_RND_CNT).dot $(RFLAGS); else ./binary-tree-random -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-random.$(TST_RND_CNT).png $(OUT_DIR)/$@-random.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="
	$(Q)echo "binary-tree-treap"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./binary-tree-treap -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@-treap.$(TST_RND_CNT).dot $(RFLAGS); else ./binary-tree-treap -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@-treap.$(TST_RND_CNT).png $(OUT_DIR)/$@-treap.$(TST_RND_CNT).dot; fi
	$(Q)echo "==========================================="

test-btree: btree
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "btree"
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then ./btree -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -g $(OUT_DIR)/$@.$(TST_RND_CNT).dot $(RFLAGS); else ./btree -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS); fi
	$(Q)if test "$(TST_GEN_GRAPH)" = "yes"; then dot -Tpng -o $(OUT_DIR)/$@.$(TST_RND_CNT).png $(OUT_DIR)/$@.$(TST_RND_CNT).dot; fi

test-binary-search: binary-search
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "binary-search"
	$(Q)./binary-search -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd $(RFLAGS)

test-wq: wq
	$(Q)echo "wq"
	$(Q)./wq -t 5 -w 100 $(RFLAGS)

test-bitfield: bitfield
	$(Q)echo "bitfield"
	$(Q)./bitfield $(RFLAGS)

test-kalman: kalman
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "kalman"
	$(Q)./kalman -F1 -H1 -B0 -Q5 -R15 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).0.out $(RFLAGS)
	$(Q)./kalman -F1 -H1 -B0 -Q5 -R50 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).1.out $(RFLAGS)
	$(Q)./kalman -F1 -H1 -B0 -Q10 -R15 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).2.out $(RFLAGS)
	$(Q)./kalman -F1 -H1 -B0 -Q15 -R15 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).3.out $(RFLAGS)
	$(Q)./kalman -F1 -H1 -B0 -Q20 -R15 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).4.out $(RFLAGS)
	$(Q)./kalman -F1 -H1 -B0 -Q20 -R50 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).5.out $(RFLAGS)
	$(Q)gnuplot -e "set terminal png size $(TST_RND_CNT)*50,1400; \
					set key left box outside; \
					set title 'Kalman filter'; \
					set output '$(OUT_DIR)/$@.$(TST.RND_CNT).png'; \
					set xtics 10; \
					set mxtics 10; \
					plot '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:2 smooth csplines with linespoints lc rgb 'red' lw 2 title 'Unfiltered', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q5-R15', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).1.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q5-R50', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).2.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q10-R15', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).3.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q15-R15', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).4.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q20-R15', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).5.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q20-R50'"

test-moving-average: moving-average
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "moving-average"
	$(Q)./moving-average -P3 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).0.out $(RFLAGS)
	$(Q)./moving-average -P5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).1.out $(RFLAGS)
	$(Q)./moving-average -P20 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).2.out $(RFLAGS)
	$(Q)gnuplot -e "set terminal png size $(TST_RND_CNT)*50,1400; \
					set key left box outside; \
					set title 'Moving-Average filter'; \
					set output '$(OUT_DIR)/$@.$(TST.RND_CNT).png'; \
					set xtics 10; \
					set mxtics 10; \
					plot '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:2 smooth csplines with linespoints lc rgb 'red' lw 2 title 'Unfiltered', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:3 smooth csplines with linespoints title 'Moving-Average-3', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).1.out' using 1:3 smooth csplines with linespoints title 'Moving-Average-5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).2.out' using 1:3 smooth csplines with linespoints title 'Moving-Average-20'"

test-alpha-beta: alpha-beta
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "alpha-beta"
	$(Q)./alpha-beta -a0.1 -b0.9 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).0.out $(RFLAGS)
	$(Q)./alpha-beta -a0.9 -b0.1 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).1.out $(RFLAGS)
	$(Q)./alpha-beta -a0.5 -b0.5 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).2.out $(RFLAGS)
	$(Q)./alpha-beta -a0.85 -b0.005 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).3.out $(RFLAGS)
	$(Q)./alpha-beta -a0.5 -b0.1 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).4.out $(RFLAGS)
	$(Q)gnuplot -e "set terminal png size $(TST_RND_CNT)*50,1400; \
					set key left box outside; \
					set title 'Alpha-Beta filter'; \
					set output '$(OUT_DIR)/$@.$(TST.RND_CNT).png'; \
					set xtics 10; \
					set mxtics 10; \
					plot '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:2 smooth csplines with linespoints lc rgb 'red' lw 2 title 'Unfiltered', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.1,Beta-0.9,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).1.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.9,Beta-0.1,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).2.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.5,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).3.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.85,Beta-0.005,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).4.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,dT=0.5'"

test-alpha-beta-gamma: alpha-beta-gamma
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "alpha-beta-gamma"
	$(Q)./alpha-beta-gamma -a0.1 -b0.9 -t0.5 -g0.25 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).0.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.9 -b0.1 -t0.5 -g0.25 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).1.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.5 -b0.5 -t0.5 -g0.25 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).2.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.85 -b0.005 -g0.25 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).3.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.5 -b0.1 -g0.25 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).4.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.5 -b0.1 -g0.5 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).5.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.5 -b0.1 -g0.75 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).6.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.5 -b0.1 -g0.005 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).7.out $(RFLAGS)
	$(Q)gnuplot -e "set terminal png size $(TST_RND_CNT)*50,1400; \
					set key left box outside; \
					set title 'Alpha-beta-gamma filter'; \
					set output '$(OUT_DIR)/$@.$(TST.RND_CNT).png'; \
					set xtics 10; \
					set mxtics 10; \
					plot '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:2 smooth csplines with linespoints lc rgb 'red' lw 2 title 'Unfiltered', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.1,Beta-0.9,Gamma-0.25,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).1.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.9,Beta-0.1,Gamma-0.25,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).2.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.5,Gamma-0.25,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).3.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.85,Beta-0.005,Gamma-0.25,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).4.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,Gamma-0.25,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).5.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,Gamma-0.5,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).6.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,Gamma-0.75,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).7.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,Gamma-0.05,dT=0.5'"

test-filters: moving-average kalman alpha-beta alpha-beta-gamma
	$(Q)mkdir -p $(OUT_DIR)

	$(Q)if test "$(TST_REGEN_RND)" = "yes" -o ! -f $(OUT_DIR)/$@.$(TST_RND_CNT).rnd; then dd if=/dev/urandom of=$(OUT_DIR)/$@.$(TST_RND_CNT).rnd bs=$(TST_RND_CNT) count=4 status=noxfer >/dev/null 2>&1; fi

	$(Q)echo "filters"
	$(Q)./moving-average -P5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).0.out $(RFLAGS)
	$(Q)./kalman -F1 -H1 -B0 -Q15 -R15 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).1.out $(RFLAGS)
	$(Q)./alpha-beta -a0.5 -b0.1 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).2.out $(RFLAGS)
	$(Q)./alpha-beta-gamma -a0.5 -b0.1 -g0.25 -t0.5 -i $(OUT_DIR)/$@.$(TST_RND_CNT).rnd -o $(OUT_DIR)/$@.$(TST_RND_CNT).3.out $(RFLAGS)
	$(Q)gnuplot -e "set terminal png size $(TST_RND_CNT)*50,1400; \
					set key left box outside; \
					set title 'Moving-Average filter'; \
					set output '$(OUT_DIR)/$@.$(TST.RND_CNT).png'; \
					set xtics 10; \
					set mxtics 10; \
					plot '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:2 smooth csplines with linespoints lc rgb 'red' lw 2 title 'Unfiltered', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).0.out' using 1:3 smooth csplines with linespoints title 'Moving-Average-5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).1.out' using 1:3 smooth csplines with linespoints title 'Kalman-Q15-R15', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).2.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,dT=0.5', \
						 '$(OUT_DIR)/$@.$(TST.RND_CNT).3.out' using 1:3 smooth csplines with linespoints title 'Alpha-0.5,Beta-0.1,Gamma-0.25,dT=0.5'"


clean:
	$(Q)rm -rf $(ALGOS) *.out *.png *.rnd *.dot
