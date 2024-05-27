package(
  default_visibility = ["//visibility:public"],
)

cc_library(
  name = "debt_graph",
  hdrs = ["debt_graph.h"],
  srcs = ["debt_graph.cc"],
  deps = [
    ":utils",
    "//proto:debts_cc_proto",
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
  name = "layered_graph",
  hdrs = ["layered_graph.h"],
  srcs = ["layered_graph.cc"],
  deps = [
    ":debt_graph",
    "@abseil-cpp//absl/container:flat_hash_map",
  ],
)

cc_test(
  name = "layered_graph_test",
  size = "small",
  srcs = ["layered_graph_test.cc"],
  deps = [
    ":debt_graph",
    ":layered_graph",
    ":utils",
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
    ":layered_graph",
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
