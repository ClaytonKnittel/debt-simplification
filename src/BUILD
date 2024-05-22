cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [
    ":debt_graph",
  ],
)

cc_library(
  name = "debt_graph",
  hdrs = ["debt_graph.h"],
  srcs = ["debt_graph.cc"],
  deps = [
    "@abseil-cpp//absl/container:flat_hash_map",
  ],
)

cc_test(
  name = "debt_graph_test",
  srcs = ["debt_graph_test.cc"],
  deps = [
    ":debt_graph",
    "@googletest//:gtest_main",
  ],
)
