cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [
    "//src/expense_simplifier",
    "//src/expense_simplifier:debt_graph",
    "//src/expense_simplifier:layered_graph",
    "//src/expense_simplifier:utils",
  ],
)

cc_library(
  name = "service",
  hdrs = ["service.h"],
  srcs = ["service.cc"],
  deps = [
    "//proto:service_cc_grpc",
    "@com_github_grpc_grpc//:grpc++",
  ],
)
