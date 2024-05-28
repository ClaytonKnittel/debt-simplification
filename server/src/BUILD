cc_binary(
  name = "main",
  srcs = ["main.cc"],
  deps = [
    ":service",
    ":static_file_server",
    "@abseil-cpp//absl/strings",
    "@com_github_grpc_grpc//:grpc++",
  ],
)

cc_library(
  name = "static_file_server",
  hdrs = ["static_file_server.h"],
  srcs = ["static_file_server.cc"],
  deps = [
    "//server/modules/httplib",
    "@abseil-cpp//absl/status:statusor",
    "@abseil-cpp//absl/strings",
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
