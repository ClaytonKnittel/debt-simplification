#!/bin/sh

sftp -oPort=5167 cknittel@cknittel.com:Documents/VSCode/debt-simplification/server <<EOF
put src/expense_simplifier/debt_graph.h src/expense_simplifier/debt_graph.h
put src/expense_simplifier/debt_graph.cc src/expense_simplifier/debt_graph.cc
put src/expense_simplifier/debt_graph_test.cc src/expense_simplifier/debt_graph_test.cc
put src/expense_simplifier/expense_simplifier.h src/expense_simplifier/expense_simplifier.h
put src/expense_simplifier/expense_simplifier.cc src/expense_simplifier/expense_simplifier.cc
put src/expense_simplifier/expense_simplifier_test.cc src/expense_simplifier/expense_simplifier_test.cc
put src/expense_simplifier/debts.proto src/expense_simplifier/debts.proto
put src/expense_simplifier/utils.h src/expense_simplifier/utils.h
put src/expense_simplifier/BUILD src/expense_simplifier/BUILD

!ssh -p 5167 cknittel@cknittel.com "cd Documents/VSCode/debt-simplification/server && \
  clang-format src/expense_simplifier/debt_graph.h -i && \
  clang-format src/expense_simplifier/debt_graph.cc -i && \
  clang-format src/expense_simplifier/debt_graph_test.cc -i && \
  clang-format src/expense_simplifier/expense_simplifier.h -i && \
  clang-format src/expense_simplifier/expense_simplifier.cc -i && \
  clang-format src/expense_simplifier/expense_simplifier_test.cc -i && \
  clang-format src/expense_simplifier/utils.h -i && \
  bazel build --noshow_progress //src/... && \
  bazel test --noshow_progress //src/..."

get src/expense_simplifier/debt_graph.h src/expense_simplifier/debt_graph.h
get src/expense_simplifier/debt_graph.cc src/expense_simplifier/debt_graph.cc
get src/expense_simplifier/debt_graph_test.cc src/expense_simplifier/debt_graph_test.cc
get src/expense_simplifier/expense_simplifier.h src/expense_simplifier/expense_simplifier.h
get src/expense_simplifier/expense_simplifier.cc src/expense_simplifier/expense_simplifier.cc
get src/expense_simplifier/expense_simplifier_test.cc src/expense_simplifier/expense_simplifier_test.cc
get src/expense_simplifier/utils.h src/expense_simplifier/utils.h
EOF
