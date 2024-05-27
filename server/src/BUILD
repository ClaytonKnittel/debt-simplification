cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [
    ":service",
    "//modules/httplib",
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
