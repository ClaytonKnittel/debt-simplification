load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

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

proto_library(
  name = "service_proto",
  srcs = ["service.proto"],
)

cc_proto_library(
  name = "service_cc_proto",
  visibility = ["//visibility:public"],
  deps = [
    ":service_proto",
  ],
)

cc_grpc_library(
  name = "service_cc_grpc",
  visibility = ["//visibility:public"],
  srcs = [":service_proto"],
  grpc_only = True,
  deps = [":service_cc_proto"],
)
