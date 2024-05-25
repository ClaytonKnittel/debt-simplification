load("@rules_proto//proto:defs.bzl", "proto_library")

cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [
    ":expense_simplifier",
  ],
)

cc_library(
  name = "debt_graph",
  hdrs = ["debt_graph.h"],
  srcs = ["debt_graph.cc"],
  deps = [
    ":debts_cc_proto",
    ":utils",
    "@abseil-cpp//absl/container:flat_hash_map",
    "@abseil-cpp//absl/status:statusor",
    "@abseil-cpp//absl/strings:str_format",
    "@abseil-cpp//absl/strings:string_view",
  ],
)

cc_test(
  name = "debt_graph_test",
  size = "small",
  srcs = ["debt_graph_test.cc"],
  deps = [
    ":debt_graph",
    ":utils",
    "@abseil-cpp//absl/strings:str_format",
    "@abseil-cpp//absl/strings:string_view",
    "@googletest//:gtest_main",
    "@protobuf//:protobuf",
  ],
)

cc_library(
  name = "expense_simplifier",
  hdrs = ["expense_simplifier.h"],
  srcs = ["expense_simplifier.cc"],
  deps = [
    ":debt_graph",
    ":debts_cc_proto",
  ],
)

cc_test(
  name = "expense_simplifier_test",
  size = "small",
  srcs = ["expense_simplifier_test.cc"],
  deps = [
    ":debt_graph",
    ":expense_simplifier",
    ":utils",
    "@googletest//:gtest_main",
    "@protobuf//:protobuf",
  ],
)


proto_library(
  name = "debts_proto",
  srcs = ["debts.proto"],
)

cc_proto_library(
  name = "debts_cc_proto",
  deps = [
    ":debts_proto",
  ],
)

cc_library(
  name = "utils",
  hdrs = ["utils.h"],
  deps = [
    "@abseil-cpp//absl/status",
    "@abseil-cpp//absl/status:statusor",
    "@abseil-cpp//absl/strings",
    "@googletest//:gtest",
  ],
)
