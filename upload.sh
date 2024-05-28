#!/bin/sh

sftp -oPort=5167 cknittel@cknittel.com:Documents/VSCode/debt-simplification <<EOF
put server/src/expense_simplifier/debt_graph.h server/src/expense_simplifier/debt_graph.h
put server/src/expense_simplifier/debt_graph.cc server/src/expense_simplifier/debt_graph.cc
put server/src/expense_simplifier/debt_graph_test.cc server/src/expense_simplifier/debt_graph_test.cc
put server/src/expense_simplifier/expense_simplifier.h server/src/expense_simplifier/expense_simplifier.h
put server/src/expense_simplifier/expense_simplifier.cc server/src/expense_simplifier/expense_simplifier.cc
put server/src/expense_simplifier/expense_simplifier_test.cc server/src/expense_simplifier/expense_simplifier_test.cc
put server/src/expense_simplifier/debts.proto server/src/expense_simplifier/debts.proto
put server/src/expense_simplifier/utils.h server/src/expense_simplifier/utils.h
put server/src/expense_simplifier/BUILD server/src/expense_simplifier/BUILD

!ssh -p 5167 cknittel@cknittel.com "cd Documents/VSCode/debt-simplification && \
  clang-format server/src/expense_simplifier/debt_graph.h -i && \
  clang-format server/src/expense_simplifier/debt_graph.cc -i && \
  clang-format server/src/expense_simplifier/debt_graph_test.cc -i && \
  clang-format server/src/expense_simplifier/expense_simplifier.h -i && \
  clang-format server/src/expense_simplifier/expense_simplifier.cc -i && \
  clang-format server/src/expense_simplifier/expense_simplifier_test.cc -i && \
  clang-format server/src/expense_simplifier/utils.h -i && \
  bazel build --noshow_progress //server/... && \
  bazel test --noshow_progress //server/..."

get server/src/expense_simplifier/debt_graph.h server/src/expense_simplifier/debt_graph.h
get server/src/expense_simplifier/debt_graph.cc server/src/expense_simplifier/debt_graph.cc
get server/src/expense_simplifier/debt_graph_test.cc server/src/expense_simplifier/debt_graph_test.cc
get server/src/expense_simplifier/expense_simplifier.h server/src/expense_simplifier/expense_simplifier.h
get server/src/expense_simplifier/expense_simplifier.cc server/src/expense_simplifier/expense_simplifier.cc
get server/src/expense_simplifier/expense_simplifier_test.cc server/src/expense_simplifier/expense_simplifier_test.cc
get server/src/expense_simplifier/utils.h server/src/expense_simplifier/utils.h
EOF
