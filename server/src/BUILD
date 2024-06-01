cc_binary(
  name = "server_main",
  srcs = ["server_main.cc"],
  deps = [
    ":service",
    ":static_file_server",
    "@abseil-cpp//absl/strings",
    "@com_github_grpc_grpc//:grpc++",
  ],
  data = [
    "//client:client_static",
  ],
)

cc_library(
  name = "static_file_server",
  hdrs = ["static_file_server.h"],
  srcs = ["static_file_server.cc"],
  deps = [
    "//modules/httplib",
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

cc_binary(
  name = "splitwise_simplifier",
  srcs = ["splitwise_simplifier.cc"],
  deps = [
    "//proto:debts_cc_proto",
    "//server/src/expense_simplifier",
    "//server/src/expense_simplifier:debt_graph",
    "//server/src/csv",
    "@abseil-cpp//absl/flags:flag",
    "@abseil-cpp//absl/flags:parse",
    "@abseil-cpp//absl/status:statusor",
    "@abseil-cpp//absl/strings",
  ],
)
