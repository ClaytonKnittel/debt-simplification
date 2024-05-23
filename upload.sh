#!/bin/sh

sftp -oPort=5167 cknittel@cknittel.com:Documents/VSCode/debt-simplification <<EOF
put src/debt_graph.h src/debt_graph.h
put src/debt_graph.cc src/debt_graph.cc
put src/debts.proto src/debts.proto
put src/utils.h src/utils.h
put src/BUILD src/BUILD

-clang-format -i src/debt_graph.h
-clang-format -i src/debt_graph.cc
!ssh -p 5167 cknittel@cknittel.com "cd Documents/VSCode/debt-simplification && clang-format src/debt_graph.h -i && clang-format src/debt_graph.cc -i && clang-format src/utils.h -i && bazel build --noshow_progress --copt="-Wno-unknown-warning-option" //src:main"

get src/debt_graph.h src/debt_graph.h
get src/debt_graph.cc src/debt_graph.cc
get src/utils.h src/utils.h
EOF
