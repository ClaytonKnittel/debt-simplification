cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [
    ":service",
    ":static_file_server",
    "@com_github_grpc_grpc//:grpc++",
  ],
)

cc_library(
  name = "static_file_server",
  hdrs = ["static_file_server.h"],
  srcs = ["static_file_server.cc"],
  deps = [
    "//modules/httplib",
    "@abseil-cpp//absl/status:statusor",
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
