#!/bin/sh

sftp -oPort=5167 cknittel@cknittel.com:Documents/VSCode/debt-simplification <<EOF
put src/debt_graph.h src/debt_graph.h
put src/debt_graph.cc src/debt_graph.cc
put src/debt_graph_test.cc src/debt_graph_test.cc
put src/expense_simplifier.h src/expense_simplifier.h
put src/expense_simplifier.cc src/expense_simplifier.cc
put src/expense_simplifier_test.cc src/expense_simplifier_test.cc
put src/debts.proto src/debts.proto
put src/utils.h src/utils.h
put src/BUILD src/BUILD

!ssh -p 5167 cknittel@cknittel.com "cd Documents/VSCode/debt-simplification && \
  clang-format src/debt_graph.h -i && \
  clang-format src/debt_graph.cc -i && \
  clang-format src/debt_graph_test.cc -i && \
  clang-format src/expense_simplifier.h -i && \
  clang-format src/expense_simplifier.cc -i && \
  clang-format src/expense_simplifier_test.cc -i && \
  clang-format src/utils.h -i && \
  bazel build --noshow_progress //src/... && \
  bazel test --noshow_progress //src/..."

get src/debt_graph.h src/debt_graph.h
get src/debt_graph.cc src/debt_graph.cc
get src/debt_graph_test.cc src/debt_graph_test.cc
get src/expense_simplifier.h src/expense_simplifier.h
get src/expense_simplifier.cc src/expense_simplifier.cc
get src/expense_simplifier_test.cc src/expense_simplifier_test.cc
get src/utils.h src/utils.h
EOF
