load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
  name = "debts_proto",
  srcs = ["debts.proto"],
)

cc_proto_library(
  name = "debts_cc_proto",
  visibility = ["//visibility:public"],
  deps = [
    ":debts_proto",
  ],
)
